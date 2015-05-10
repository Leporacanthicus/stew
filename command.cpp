#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <map>
#include "command.h"
#include "cpu.h"
#include "stew.h"

class CmdClass
{
public:
    virtual bool DoIt(LineParser& lp) = 0;
    virtual std::string Description() = 0;
};

std::map<std::string, CmdClass*> cmdMap;

class LoadCmd : public CmdClass
{
public:
    bool DoIt(LineParser& lp) override;
    std::string Description() override
	{
	    return  "LOAD file - Load hex data from the file";
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
    
    std::ifstream f(file);
    uint32_t v;
    uint32_t addr = 0;
    while(f >> std::hex >> v)
    {
	cpu->WriteMem(addr, v);
	addr += 4;
    }

    std::cout << "Loaded " << addr << " bytes" << std::endl;
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
    std::cout << "Command available:\n" << std::endl;
    // TODO: Collect all descritions and align the first '-' to make it neat.
    for(auto c : cmdMap)
    {
	std::cout << c.second->Description() << std::endl;
    }
    return false;
}

class QuitCmd : public CmdClass
{
public:
    QuitCmd(const std::string& nm) : name(nm) { }
    bool DoIt(LineParser& lp) override;
    std::string Description() override
	{
	    return  name + " - Show this message";
	}
private:
    std::string name;
};

bool QuitCmd::DoIt(LineParser& lp)
{
    return true;
}

static void ShowPC()
{
    std::cout << "PC=" << std::hex << cpu->RegValue(PC) << std::endl;
}

class StepCmd : public CmdClass
{
public:
    bool DoIt(LineParser& lp) override;
    std::string Description() override
	{
	    return  "STEP - Step one instruction";
	}
};

bool StepCmd::DoIt(LineParser& lp)
{
    cpu->RunOneInstr();
    ShowPC();
    return false;
}

class RegsCmd : public CmdClass
{
public:
    bool DoIt(LineParser& lp) override;
    std::string Description() override
	{
	    return  "REGS - Show register values";
	}
};

static std::string RegStr(int i)
{
    if (i == 15) return "pc";
    if (i == 14) return "sp";
    std::string name = "r";
    if (i >= 10)
    {
	name += "1";
	i -= 10;
    }
    name += '0' + i;
    return name;
}

bool RegsCmd::DoIt(LineParser& lp)
{
    int count = 0;
    for(int i = R0; i <= PC; i++)
    {
	std::cout << std::setw(3) << std::setfill(' ') << RegStr(i) << ": "
		  << std::hex << std::setw(8) << std::setfill('0')
		  << cpu->RegValue((RegName)i) << " ";
	count++;
	if (count == 4)
	{
	    count = 0;
	    std::cout << std::endl;
	}
    }
    return false;
}

class RunCmd : public CmdClass
{
public:
    bool DoIt(LineParser& lp) override;
    std::string Description() override
	{
	    return  "RUN - Execute program";
	}
};

bool RunCmd::DoIt(LineParser& lp)
{
    while(cpu->RunOneInstr());
    return false;
}

void InitCommands()
{
    cmdMap["load"] = new LoadCmd;
    cmdMap["help"] = new HelpCmd;
    cmdMap["quit"] = new QuitCmd("quit");
    cmdMap["exit"] = new QuitCmd("exit");
    cmdMap["step"] = new StepCmd;
    cmdMap["s"]    = cmdMap["step"];
    cmdMap["regs"] = new RegsCmd;
    cmdMap["run"]  = new RunCmd;
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
