#include <iostream>
#include "cpu.h"
#include "memory.h"

CPU::CPU(Memory& mem, uint32_t start) : memory(mem)
{
    registers[PC].Value(start);
}

uint32_t CPU::GetValue(AddrMode mode, RegName reg)
{
    switch(mode)
    {
    case Direct:
	return registers[reg].Value();

    case Indir:
	return ReadMem(registers[reg].Value());

    case IndirAutoInc:
    {
	uint32_t v = ReadMem(registers[reg].Value());
	registers[reg] += 4;
	return v;
    }
    case AutoDecIndir:
	registers[reg] -= 4;
	return ReadMem(registers[reg].Value());
    }
    return 0xdeadbeef;
}

uint32_t CPU::GetSourceValue(Instruction instr)
{
    return GetValue(instr.value.srcMode, instr.value.source);
}

uint32_t CPU::GetDestValue(Instruction instr)
{
    return GetValue(instr.value.destMode, instr.value.dest);
}

void CPU::StoreDestValue(Instruction instr, uint32_t value)
{
    switch(instr.value.srcMode)
    {
    case Direct:
	registers[instr.value.source].Value(value);
	break;

    case Indir:
	WriteMem(registers[instr.value.source].Value(), value);
	break;

    case IndirAutoInc:
	WriteMem(registers[instr.value.source].Value(), value);
	registers[instr.value.source] += 4;
	break;
	
    case AutoDecIndir:
	registers[instr.value.source] -= 4;
	WriteMem(registers[instr.value.source].Value(), value);
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
	break;
    }
    case ADD:
    {
	uint32_t v1 = GetSourceValue(instr);
	uint32_t v2 = GetDestValue(instr);
	uint32_t v = v1 + v2;
	StoreDestValue(instr, v);
	break;
    }
    case SUB:
    {
	uint32_t v1 = GetSourceValue(instr);
	uint32_t v2 = GetDestValue(instr);
	uint32_t v = v2 - v1;
	StoreDestValue(instr, v);
	break;
    }
    default:
	std::cerr << "Not yet impelemented function at: "
		  << std::hex << registers[PC].Value()
		  << std::endl;
	std::cerr << "Instr = " << instr.value.word << std::endl;
	break;
    }
    return true;
}

