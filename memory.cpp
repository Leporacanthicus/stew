#include <cstdint>
#include <iostream>
#include "memory.h"

Memory::Memory(uint32_t base, uint32_t size) : base(base)
{
    mem = new uint32_t[size / sizeof(uint32_t)];
}

void Memory::Write(uint32_t addr, uint32_t value, uint32_t opsize)
{
    uint32_t mask = 0xFFFFFFFF;
    switch(opsize)
    {
    case 1:
	mask = 0xFF << ((3 - (addr & 3)) << 3);
	break;
    case 2:
	mask = 0xFFFF << ((2 - (addr & 2)) << 3);
	break;
    default:
	break;
    }
    uint32_t old = mem[addr / sizeof(uint32_t)] = value;
    value = (old & ~mask) | (value & mask);
    mem[addr / sizeof(uint32_t)] = value;
}

uint32_t Memory::Read(uint32_t addr, uint32_t opsize)
{
    uint32_t mask = 0xFFFFFFFF;
    uint32_t shift = 0;
    switch(opsize)
    {
    case 1:
	mask = 0xFF;
	shift =  (addr & 3) << 3;
	break;
    case 2:
	mask = 0xFFFF;
	shift =  (addr & 2) << 3;
	break;
    }
    uint32_t value = mem[addr / sizeof(uint32_t)];
    return (value >> shift) & mask;
}
