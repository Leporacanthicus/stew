#include <iostream>
#include "cpu.h"
#include "memory.h"

CPU::CPU(Memory& mem, uint32_t start) : memory(mem)
{
    registers[PC].Value(start);
}

uint32_t CPU::GetSourceValue(Instruction instr)
{
    switch(instr.value.srcMode)
    {
    case Direct:
	return registers[instr.value.source].Value();

    case Indir:
	return ReadMem(registers[instr.value.source].Value());

    case IndirAutoInc:
    {
	uint32_t v = ReadMem(registers[instr.value.source].Value());
	registers[instr.value.source] += 4;
	return v;
    }
    case AutoDecIndir:
	registers[instr.value.source] -= 4;
	return ReadMem(registers[instr.value.source].Value());
    }
    return 0xdeadbeef;
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
    default:
	std::cerr << "Not yet impelemented function" << std::endl;
	break;
    }
    return true;
}

