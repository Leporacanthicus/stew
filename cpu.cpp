#include <iostream>
#include "cpu.h"
#include "memory.h"
#include "emt.h"

CPU::CPU(Memory& mem, uint32_t start) : memory(mem)
{
    registers[PC].Value(start);
}


static bool CmpOverflow(uint64_t v, uint32_t v1, uint32_t v2, uint64_t sign)
{
    /*
      cmp:
      V: set if there was arithmetic overflow; that is, operands were
      of opposite signs and the sign of the destination was the
      same as the sign of the result; cleared otherwise
    */

    return ((v & sign) == (v2 & sign)) & ((v1 ^ v2) & sign);
}

static bool SubOverflow(uint64_t v, uint32_t v1, uint32_t v2, uint64_t sign)
{
    /* sub:
      V: set if there was arithmetic overflow as a result of the oper·
      ation, that is if operands were of opposite signs and the sign
      of the source was the same as the sign of the result; cleared
      otherwise. */
    return ((v & sign) == (v1 & sign)) & ((v1 ^ v2) & sign);
}    
    
static bool AddOverflow(uint64_t v, uint32_t v1, uint32_t v2, uint64_t sign)
{
    /*  add:
      V: set if there was arithmetic overflow as a result of the oper·
      ation; that is both operands were of the same sign and the
      result was of the opposite sign; cleared otherwise
a    */
    return ((v & sign) != (v1 & sign)) & ((v1 & sign) == (v2 & sign));
}

void CPU::UpdateFlags(uint64_t v, uint32_t v1, uint32_t v2, OperandSize opsize,
		      OverflowFunc oflow)
{
    uint64_t mask;
    switch(opsize)
    {
    case Op8:
	mask = 0xff;
	break;
    case Op16:
	mask = 0xffff;
	break;
    case Op32:
	mask = 0xffffffff;
	break;
    }
    uint64_t sign = (mask + 1) >> 1;
    flags.z = !(v & mask);
    flags.n = v & sign;
    flags.c = v & (mask + 1);
    if (oflow)
    {
	flags.v = oflow(v, v1, v2, sign);
    }
    else
    {
	flags.v = false;
    }
}

static uint32_t SizeFromOpSize(OperandSize opsize)
{
    switch(opsize)
    {
    case Op8:
	return 1;
    case Op16:
	return 2;
    case Op32:
	return 4;
    }
}

uint32_t CPU::GetValue(AddrMode mode, RegName reg, OperandSize opsize)
{
    size_t size = SizeFromOpSize(opsize);
    size_t regsize = size;
    if (reg == PC || reg == SP)
    {
	regsize = 4;
    }
    switch(mode)
    {
    case Direct:
	return registers[reg].Value();

    case Indir:
	return ReadMem(registers[reg].Value(), size);

    case IndirAutoInc:
    {
	uint32_t v = ReadMem(registers[reg].Value(), size);
	registers[reg] += regsize;
	return v;
    }
    case AutoDecIndir:
	registers[reg] -= regsize;
	return ReadMem(registers[reg].Value(), size);
    }
    return 0xdeadbeef;
}

uint32_t CPU::GetSourceValue(Instruction instr)
{
    return GetValue(instr.value.srcMode, instr.value.source, instr.value.size);
}

uint32_t CPU::GetDestValue(Instruction instr)
{
    return GetValue(instr.value.destMode, instr.value.dest, instr.value.size);
}

static uint32_t SignExtend(uint32_t value, OperandSize opsize)
{
    switch(opsize)
    {
    case Op32:
	break;
    case Op16:
	if (value & 0x8000)
	{
	    value |= 0xFFFF0000;
	}
	break;
    case Op8:
	if (value & 0x80)
	{
	    value |= 0xFFFFFF00;
	}
	break;
    }
    return value;
}

void CPU::StoreDestValue(Instruction instr, uint32_t value)
{
    size_t size = SizeFromOpSize(instr.value.size);
    size_t regsize = size;
    RegName reg = instr.value.dest;
    if (reg == PC || reg == SP)
    {
	regsize = 4;
    }
    switch(instr.value.destMode)
    {
    case Direct:
	value = SignExtend(value, instr.value.size);
	registers[reg].Value(value);
	break;

    case Indir:
	WriteMem(registers[reg].Value(), value, size);
	break;

    case IndirAutoInc:
	WriteMem(registers[reg].Value(), value, size);
	registers[reg] += regsize;
	break;
	
    case AutoDecIndir:
	registers[reg] -= regsize;
	WriteMem(registers[reg].Value(), value, size);
	break;
    }
}

void CPU::Emt(uint32_t num)
{
    switch(num)
    {
    case PrintChar:
	std::cout << (char)registers[R0].Value() << std::flush;
	break;
    }
}

void CPU::BranchIfTrue(Instruction instr, bool cond)
{
    if (cond)
    {
	registers[PC] += instr.value.branch;
    }
}

void CPU::Move(Instruction instr)
{
    uint32_t v = GetSourceValue(instr);
    StoreDestValue(instr, v);
    UpdateFlags(v, v, v, instr.value.size, 0 );
}

void CPU::Add(Instruction instr)
{
    uint32_t v1 = GetSourceValue(instr);
    uint32_t v2 = GetDestValue(instr);
    uint64_t v = static_cast<uint64_t>(v1) + v2;
    StoreDestValue(instr, v);
    UpdateFlags(v, v1, v2, instr.value.size, AddOverflow);
}

void CPU::Sub(Instruction instr)
{
    uint32_t src = GetSourceValue(instr);
    uint32_t dest = GetDestValue(instr);
    uint32_t v = static_cast<uint64_t>(dest) - src;
    StoreDestValue(instr, v);
    UpdateFlags(v, src, dest, instr.value.size, SubOverflow);
}

void CPU::Cmp(Instruction instr)
{
    uint32_t src = GetSourceValue(instr);
    uint32_t dest = GetDestValue(instr);
    uint64_t v = static_cast<uint64_t>(src) - dest;
    UpdateFlags(v, src, dest, instr.value.size, CmpOverflow);
}

void CPU::Div(Instruction instr)
{
    uint32_t v1 = GetSourceValue(instr);
    uint32_t v2 = GetDestValue(instr);
    uint32_t v_div = v2 / v1;
    uint32_t v_mod = v2 % v1;
    StoreDestValue(instr, v_div);
    if (instr.value.destMode == Direct)
    {
	registers[instr.value.dest+1].Value(v_mod);
    }
    UpdateFlags(v_div, 0, 0, instr.value.size, 0); // TODO: Add ovflowe func.
}

void CPU::Mul(Instruction instr)
{
    uint32_t v1 = GetSourceValue(instr);
    uint32_t v2 = GetDestValue(instr);
    uint64_t v = static_cast<uint64_t>(v2) * v1;
    StoreDestValue(instr, v);
    UpdateFlags(v, 0, 0, instr.value.size, 0);  // TODO: Add overflow func.
}

void CPU::Jmp(Instruction instr)
{
    uint32_t v = GetSourceValue(instr);
    registers[PC].Value(v);
}

void CPU::Jsr(Instruction instr)
{
    uint32_t v = GetSourceValue(instr);
    registers[SP] -= 4;
    WriteMem(registers[SP].Value(), registers[PC].Value(), 4);
    registers[PC].Value(v);
}

void CPU::Ret(Instruction instr)
{
    registers[PC].Value(ReadMem(registers[SP].Value(), 4));
    registers[SP] += 4;
}

/* Return true for "continue", false for "stop" */
ExecResult CPU::RunOneInstr()
{
    Instruction instr = Fetch();
    switch(instr.value.op)
    {
    case HLT:
	std::cout << "Hit halt at " << std::hex << registers[PC].Value()
		  << std::endl;
	return Halt;

    case BPT:
	return Breakpoint;
	
    case MOV:
	Move(instr);
	break;
    case ADD:
	Add(instr);
	break;
    case SUB:
	Sub(instr);
	break;
    case CMP:
	Cmp(instr);
	break;
    case DIV:
	Div(instr);
	break;
    case MUL:
	Mul(instr);
	break;

    case CLC:
	flags.c = false;
	break;
    case CLV:
	flags.v = false;
	break;
    case CLN:
	flags.n = false;
	break;
    case CLZ:
	flags.z = false;
	break;

    case SEC:
	flags.c = true;
	break;
    case SEV:
	flags.v = true;
	break;
    case SEN:
	flags.n = true;
	break;
    case SEZ:
	flags.z = true;
	break;

    case JMP:
	Jmp(instr);
	break;
    case JSR:
	Jsr(instr);
	break;
    case RET:
	Ret(instr);
	break;

	/*
		|0010xx || BNE || Branch if not equal (Z=0)
		|0014xx || BEQ || Branch if equal (Z=1)
		|0020xx || BGE || Branch if greater than or equal (N|V = 0)
		|0024xx || BLT || Branch if less than (N|V = 1)
		|0030xx || BGT || Branch if greater than (N^V = 1)
		|0034xx || BLE || Branch if less than or equal (N^V = 0)
		|1010xx || BHI || Branch if higher than (C|Z = 0)
		|1014xx || BLOS|| Branch if lower or same (C|Z = 1)
		|1020xx || BVC || Branch if overflow clear (V=0)
		|1024xx || BVS || Branch if overflow set (V=1)
		|1030xx || BCC || Branch if carry clear (C=0)
		|       || BHIS|| Branch if higher or same (C=0)
		|1034xx || BCS || Branch if carry set (C=1)
		|       || BLO || Branch if lower than (C=1)
	*/
    case BNE:
	BranchIfTrue(instr, !flags.z);
	break;
    case BEQ:
	BranchIfTrue(instr, flags.z);
	break;
    case BLT:
	BranchIfTrue(instr, flags.n | flags.v);
	break;
    case BGT:
	BranchIfTrue(instr, flags.n ^ flags.v);
	break;
    case BGE:
	BranchIfTrue(instr, !(flags.n | flags.v));
	break;
    case BLE:
	BranchIfTrue(instr, !(flags.n ^ flags.v));
	break;
    case BHI:
	BranchIfTrue(instr, !(flags.c | flags.z));
	break;
    case BLOS:
	BranchIfTrue(instr, flags.c | flags.z);
	break;
    case BVS:
	BranchIfTrue(instr, flags.v);
	break;
    case BVC:
	BranchIfTrue(instr, !flags.v);
	break;
    case BCC:
	BranchIfTrue(instr, !flags.c);
	break;
    case BCS:
	BranchIfTrue(instr, flags.c);
	break;
    case BPL:
	BranchIfTrue(instr, !flags.n);
	break;
    case BMI:
	BranchIfTrue(instr, flags.n);
	break;
    case BR:
	BranchIfTrue(instr, true);
	break;

    case NOP:
	// Nothing to see here, move on!
	break;
	
    case EMT:
	Emt(instr.value.branch);
	break;
	
    default:
	std::cerr << "Not yet impelemented function at: "
		  << std::hex << registers[PC].Value()
		  << std::endl;
	std::cerr << "Instr = " << instr.value.word << std::endl;
	return Unknown;
	break;
    }
    return Continue;
}

