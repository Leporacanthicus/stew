#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <map>
#include "command.h"

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
    while(f >> std::hex >> v)
    {
	std::cout << std::hex << v << std::endl;
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

void InitCommands()
{
    cmdMap["load"] = new LoadCmd;
    cmdMap["help"] = new HelpCmd;
    cmdMap["quit"] = new QuitCmd("quit");
    cmdMap["exit"] = new QuitCmd("exit");
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
