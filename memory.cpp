#include <cstdint>
#include "memory.h"

Memory::Memory(uint32_t base, uint32_t size) : base(base)
{
    mem = new uint32_t[size / sizeof(uint32_t)];
}
