#include <string>
#include <fstream>
#include <iostream>
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

class StepCmd : public CmdClass
{
public:
    bool DoIt(LineParser& lp) override;
    std::string Description() override
	{
	    return  "STEP - Show this message";
	}
};

bool StepCmd::DoIt(LineParser& lp)
{
    cpu->RunOneInstr();
    return false;
}

void InitCommands()
{
    cmdMap["load"] = new LoadCmd;
    cmdMap["help"] = new HelpCmd;
    cmdMap["quit"] = new QuitCmd("quit");
    cmdMap["exit"] = new QuitCmd("exit");
    cmdMap["step"] = new StepCmd;
//    cmdMap["run"]  = new RunCmd;
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
