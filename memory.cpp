#include <cstdint>
#include <iostream>
#include <cstring>
#include "memory.h"

Memory::Memory(uint32_t base, uint32_t size) : base(base)
{
    mem = new uint32_t[size / sizeof(uint32_t)];
    memset(mem, 0, size);
}

Memory::~Memory()
{
    delete mem;
}

void Memory::Write(uint32_t addr, uint32_t value, uint32_t opsize)
{
    uint32_t mask = 0;
    uint32_t amask = 0;
    uint32_t shift = 0;
    switch(opsize)
    {
    case 1:
	amask = 0;
	shift = (addr & 3) << 3;
	mask = 0xFF << shift;
	break;
    case 2:
	amask = 1;
	shift = (addr & 2) << 3;
	mask = 0xFFFF << shift;
	break;
    case 4:
	mask = 0xFFFFFFFF;
	amask = 3;
	shift = 0;
	break;
    default:
	std::cerr << "Invalid size" << std::endl;
	break;
    }
    uint32_t old = mem[addr / sizeof(uint32_t)];
    value = (old & ~mask) | ((value << shift) & mask);
    if (addr & amask)
    {
	std::cerr << "Unaligned access" << std::endl;
    }
    mem[addr / sizeof(uint32_t)] = value;
}

uint32_t Memory::Read(uint32_t addr, uint32_t opsize)
{
    uint32_t mask = 0;
    uint32_t shift = 0;
    uint32_t amask = 0;
    switch(opsize)
    {
    case 1:
	amask = 0;
	mask = 0xFF;
	shift =  (addr & 3) << 3;
	break;
    case 2:
	mask = 0xFFFF;
	amask = 1;
	shift =  (addr & 2) << 3;
	break;
    case 4:
	mask = 0xFFFFFFFF;
	amask = 3;
	shift = 0;
	break;
    default:
	std::cerr << "Invalid size" << std::endl;
	break;
    }
    if (addr & amask)
    {
	std::cerr << "Unaligned access" << std::endl;
    }
    uint32_t value = mem[addr / sizeof(uint32_t)];
    return (value >> shift) & mask;
}
