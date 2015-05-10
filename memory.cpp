#include <cstdint>
#include "memory.h"

Memory::Memory(uint32_t base, uint32_t size) : base(base)
{
    mem = new uint32_t[size / sizeof(uint32_t)];
}

void Memory::Write(uint32_t addr, uint32_t value)
{
    mem[addr / sizeof(uint32_t)] = value;
}

uint32_t Memory::Read(uint32_t addr)
{
    return mem[addr / sizeof(uint32_t)];
}
