#ifndef CPU_H
#define CPU_H

#include "instruction.h"
#include "memory.h"

class CPU
{
public:
    CPU(Memory& mem, uint32_t start);
    bool RunOneInstr();
    /* The read/write memory are usef for loading and dumping memrory */
    void WriteMem(uint32_t addr, uint32_t value)
    {
	memory.Write(addr, value);
    }
    uint32_t ReadMem(uint32_t addr)
    {
	return memory.Read(addr);
    }

    uint32_t RegValue(RegName r) { return registers[r].Value(); }
    
private:
    Instruction Fetch()
    {
	Instruction instr;
	instr.value.word = ReadMem(registers[PC].Value());
	registers[PC] += 4;
	return instr;
    }

    uint32_t GetValue(AddrMode mode, RegName reg);
    uint32_t GetSourceValue(Instruction instr);
    uint32_t GetDestValue(Instruction instr);
    void StoreDestValue(Instruction instr, uint32_t value);

private:
    Memory& memory;
    Register registers[MaxReg];
    FlagRegister flags;
};

#endif


