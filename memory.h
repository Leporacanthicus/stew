#ifndef MEMORY_H
#define MEMORY_H

class Memory
{
public:
    Memory(uint32_t base, uint32_t size);
    ~Memory();
    void Write(uint32_t addr, uint32_t value, uint32_t size);
    uint32_t Read(uint32_t addr, uint32_t size);
private:
    uint32_t base;
    uint32_t *mem;
};

#endif
