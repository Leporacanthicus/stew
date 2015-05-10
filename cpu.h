#include "instruction.h"
#include "memory.h"

class CPU
{
public:
    CPU(Memory& mem, uint32_t start);
    void RunOneInstr();
private:
    Memory& memory;
    Register registers[MaxReg];
    FlagRegister flags;
};


