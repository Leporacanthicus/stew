#include <cstdint>
#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <algorithm>
#include "cpu.h"
#include "memory.h"
#include "command.h"
#include "instruction.h"
#include "lineparser.h"
#include "stew.h"

CPU *cpu;

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

int main()
{
    Memory mem(0, 1 * 1024 * 1024);
    cpu = new CPU(mem, 0);

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
	if (Command(lp))
	{
	    break;
	}
    }
}
