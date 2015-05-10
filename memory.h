#ifndef MEMORY_H
#define MEMORY_H

class Memory
{
public:
    Memory(uint32_t base, uint32_t size);
private:
    uint32_t base;
    uint32_t *mem;
};

#endif
