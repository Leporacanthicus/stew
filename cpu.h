#ifndef CPU_H
#define CPU_H

#include "instruction.h"
#include "memory.h"

class CPU
{
public:
    CPU(Memory& mem, uint32_t start);
    bool RunOneInstr();
    void WriteMem(uint32_t addr, uint32_t value)
    {
	memory.Write(addr, value);
    }
    uint32_t ReadMem(uint32_t addr) { return memory.Read(addr); }
    Instruction Fetch()
    {
	Instruction instr;
	instr.value.word = ReadMem(registers[PC].Value());
	registers[PC] += 4;
	return instr;
    }

    uint32_t GetSourceValue(Instruction instr);
    void StoreDestValue(Instruction instr, uint32_t value);

private:
    Memory& memory;
    Register registers[MaxReg];
    FlagRegister flags;
};

#endif


