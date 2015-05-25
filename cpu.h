#ifndef CPU_H
#define CPU_H

#include <cassert>
#include "instruction.h"
#include "memory.h"

enum ExecResult
{
    Continue,
    Halt,
    Breakpoint,
    Unknown,
};

typedef bool (*OverflowFunc)(uint64_t v, uint32_t v1, uint32_t v2,
			     uint64_t sign);

class CPU
{
public:
    CPU(Memory& mem, uint32_t start);
    ExecResult RunOneInstr();
    /* The read/write memory are usef for loading and dumping memrory */
    void WriteMem(uint32_t addr, uint32_t value, uint32_t size)
    {
	memory.Write(addr, value, size);
    }
    uint32_t ReadMem(uint32_t addr, uint32_t size)
    {
	return memory.Read(addr, size);
    }

    uint32_t RegValue(RegName r) { return registers[r].Value(); }
    void RegValue(RegName r, uint32_t v) { registers[r].Value(v); }
    uint32_t Flags() { return flags.word; }

private:
    Instruction Fetch()
    {
	Instruction instr;
	assert(!(registers[PC].Value() & 3) &&
	       "Expect even instruction address");
	instr.value.word = ReadMem(registers[PC].Value(), 4);
	registers[PC] += 4;
	return instr;
    }

    uint32_t GetValue(AddrMode mode, RegName reg, OperandSize opsize);
    uint32_t GetSourceValue(Instruction instr);
    uint32_t GetDestValue(Instruction instr);
    void StoreDestValue(Instruction instr, uint32_t value);
    void UpdateFlags(uint64_t value, uint32_t v1, uint32_t v2,
		     OperandSize opsize, OverflowFunc oflow);
    void Emt(uint32_t num);
    void BranchIfTrue(Instruction instr, bool cond);
    void Move(Instruction instr);
    void Add(Instruction instr);
    void Sub(Instruction instr);
    void Div(Instruction instr);
    void Mul(Instruction instr);
    void Jmp(Instruction instr);
    void Jsr(Instruction instr);
    void Ret(Instruction instr);
    void Cmp(Instruction instr);

private:
    Memory& memory;
    Register registers[MaxReg];
    FlagRegister flags;
};

#endif


