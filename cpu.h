#ifndef CPU_H
#define CPU_H

#include "instruction.h"
#include "memory.h"

class CPU
{
public:
    CPU(Memory& mem, uint32_t start);
    void RunOneInstr();
    void WriteMem(uint32_t addr, uint32_t value)
    {
	memory.Write(addr, value);
    }
    uint32_t ReadMem(uint32_t addr) { return memory.Read(addr); }

private:
    Memory& memory;
    Register registers[MaxReg];
    FlagRegister flags;
};

#endif


