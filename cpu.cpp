#include <iostream>
#include "cpu.h"
#include "memory.h"
#include "emt.h"

CPU::CPU(Memory& mem, uint32_t start) : memory(mem)
{
    registers[PC].Value(start);
}

void CPU::UpdateFlags(uint32_t v)
{
    flags.z = (v == 0);
    flags.n = (v & 0x80000000);
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
    switch(mode)
    {
    case Direct:
	return registers[reg].Value();

    case Indir:
	return ReadMem(registers[reg].Value(), size);

    case IndirAutoInc:
    {
	uint32_t v = ReadMem(registers[reg].Value(), size);
	registers[reg] += SizeFromOpSize(opsize);
	return v;
    }
    case AutoDecIndir:
	registers[reg] -= SizeFromOpSize(opsize);
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

void CPU::StoreDestValue(Instruction instr, uint32_t value)
{
    size_t size = SizeFromOpSize(instr.value.size);
    switch(instr.value.destMode)
    {
    case Direct:
	registers[instr.value.dest].Value(value);
	break;

    case Indir:
	WriteMem(registers[instr.value.dest].Value(), value, size);
	break;

    case IndirAutoInc:
	WriteMem(registers[instr.value.dest].Value(), value, size);
	registers[instr.value.dest] += size;
	break;
	
    case AutoDecIndir:
	registers[instr.value.dest] -= size;
	WriteMem(registers[instr.value.dest].Value(), value, size);
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

/* Return true for "continue", false for "stop" */
bool CPU::RunOneInstr()
{
    Instruction instr = Fetch();
    switch(instr.value.op)
    {
    case HLT:
    {
	std::cout << "Hit halt at " << std::hex << registers[PC].Value()
		  << std::endl;
	return false;
    }
    case MOV:
    {
	uint32_t v = GetSourceValue(instr);
	StoreDestValue(instr, v);
	UpdateFlags(v);
	break;
    }
    case ADD:
    {
	uint32_t v1 = GetSourceValue(instr);
	uint32_t v2 = GetDestValue(instr);
	uint32_t v = v1 + v2;
	StoreDestValue(instr, v);
	UpdateFlags(v);
	break;
    }
    case SUB:
    {
	uint32_t v1 = GetSourceValue(instr);
	uint32_t v2 = GetDestValue(instr);
	uint32_t v = v2 - v1;
	StoreDestValue(instr, v);
	UpdateFlags(v);
	break;
    }
    case CMP:
    {
	uint32_t v1 = GetSourceValue(instr);
	uint32_t v2 = GetDestValue(instr);
	uint32_t v = v2 - v1;
	UpdateFlags(v);
	break;
    }
    case DIV:
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
	UpdateFlags(v_div);
	break;
    }
    case MUL:
    {
	uint32_t v1 = GetSourceValue(instr);
	uint32_t v2 = GetDestValue(instr);
	uint32_t v = v2 * v1;
	StoreDestValue(instr, v);
	UpdateFlags(v);
	break;
    }	
    
    case JMP:
    {
	uint32_t v = GetSourceValue(instr);
	registers[PC].Value(v);
	break;
    }
    case JSR:
    {
	uint32_t v = GetSourceValue(instr);
	registers[SP] -= 4;
	WriteMem(registers[SP].Value(), registers[PC].Value(), 4);
	registers[PC].Value(v);
	break;
    }
    case RET:
	registers[PC].Value(ReadMem(registers[SP].Value(), 4));
	registers[SP] += 4;
	break;
	
    case BNE:
	if (!flags.z)
	{
	    registers[PC] += instr.value.branch;
	}
	break;
    case BEQ:
	if (flags.z)
	{
	    registers[PC] += instr.value.branch;
	}
	break;
	
    case EMT:
	Emt(instr.value.branch);
	break;
    default:
	std::cerr << "Not yet impelemented function at: "
		  << std::hex << registers[PC].Value()
		  << std::endl;
	std::cerr << "Instr = " << instr.value.word << std::endl;
	return false;
	break;
    }
    return true;
}

