#include <iostream>
#include "cpu.h"
#include "memory.h"

CPU::CPU(Memory& mem, uint32_t start) : memory(mem)
{
    registers[PC].Value(start);
}

/* Return true for "continue", false for "stop" */
bool CPU::RunOneInstr()
{
    Instruction instr = Fetch();
    if (instr.value.op == HLT)
    {
	std::cout << "Hit halt at " << std::hex << registers[PC].Value()
		  << std::endl;
	return false;
    }
    return true;
}

