#include <cstdint>
#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <algorithm>
#include "instruction.h"
#include "lineparser.h"

class CmdClass
{
public:
    virtual bool DoIt(LineParser& lp) = 0;
    virtual std::string Description() = 0;
};

std::map<std::string, CmdClass*> cmdMap;

class StewLineParser : public LineParser
{
public:
    using LineParser::LineParser;
    bool IsSeparator(char c) override;
    void ErrOutput(const std::string& msg) override;
};

void StewLineParser::ErrOutput(const std::string& msg)
{
    std::cerr << msg << std::endl;
}

bool StewLineParser::IsSeparator(char c)
{
    if (Done() || isspace(c))
    {
	return true;
    }
    return false;
}

class Memory
{
public:
    Memory(uint32_t base, uint32_t size);
private:
    uint32_t base;
    uint32_t *mem;
};

Memory::Memory(uint32_t base, uint32_t size) : base(base)
{
    mem = new uint32_t[size / sizeof(uint32_t)];
}

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

CPU::CPU(Memory& mem, uint32_t start) : memory(mem)
{
    registers[PC].Value(start);
}


class LoadCmd : public CmdClass
{
public:
    bool DoIt(LineParser& lp) override;
    std::string Description() override
	{
	    return  "LOAD  file - Load hex data from the file";
	}
};

bool LoadCmd::DoIt(LineParser& lp)
{
    std::string file = lp.GetWord();
    if (file == "")
    {
	lp.Error("Expected filename to be given");
	return false;
    }
    // Do something here...
    return false;
}

class HelpCmd : public CmdClass
{
public:
    bool DoIt(LineParser& lp) override;
    std::string Description() override
	{
	    return  "Help - Show this message";
	}
};

bool HelpCmd::DoIt(LineParser& lp)
{
    (void) lp;
    std::cout << "Command available:\n" << std::endl;
    for(auto c : cmdMap)
    {
	std::cout << c.second->Description() << std::endl;
    }
    return false;
}

void InitCommands()
{
    cmdMap["load"] = new LoadCmd;
    cmdMap["help"] = new HelpCmd;
}

bool Command(LineParser& lp)
{
    std::string cmd = lp.GetWord();
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);    

    auto it = cmdMap.find(cmd);
    if (it != cmdMap.end())
    {
	return it->second->DoIt(lp);
    }
    lp.Error("Command not found");
    return false;
}

int main()
{
    Memory mem(0, 1 * 1024 * 1024);
    CPU cpu(mem, 0);

    InitCommands();
    for(;;)
    {
	std::cout << ". " << std::flush;
	std::string line;
	if (!std::getline(std::cin, line))
	{
	    std::cout << "<EOF>" << std::endl;
	    break;
	}
        StewLineParser lp(line);
	if (!Command(lp))
	{
	    break;
	}
    }
}
