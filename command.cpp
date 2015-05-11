#include <string>
#include <fstream>
#include <sstream>
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
    virtual bool Repeat() { return false; }
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
	cpu->WriteMem(addr, v, 1);
	addr += 1;
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

static void ShowRegs()
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
    std::cout << "Flags:" << cpu->Flags() << " " << "@pc: "
	      << cpu->ReadMem(cpu->RegValue(PC), 4) << std::endl;
}

class StepCmd : public CmdClass
{
public:
    bool DoIt(LineParser& lp) override;
    std::string Description() override
	{
	    return  "STEP - Step one instruction";
	}
    bool Repeat() override { return true; }
};

bool StepCmd::DoIt(LineParser& lp)
{
    uint32_t count = 1;
    if (!lp.Done())
    {
	if (!lp.GetNum(count))
	{
	    lp.Error("Expected number as argument");
	    return false;
	}
    }
    for(uint32_t i = 0; i < count; i++)
    {
	cpu->RunOneInstr();
	ShowRegs();
    }
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

bool RegsCmd::DoIt(LineParser& lp)
{
    ShowRegs();
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

class DumpCmd : public CmdClass
{
public:
    DumpCmd(const std::string& nm, uint32_t sz)
	{
	    size = sz;
	    name = nm;
	}
    bool DoIt(LineParser& lp) override;
    std::string Description() override
	{
	    std::stringstream ss;
	    ss << name << " addr {length} - Dump memory with wordsize=" << size;
	    return ss.str();
	}
private:
    uint32_t size;
    std::string name;
};

bool DumpCmd::DoIt(LineParser& lp)
{
    uint32_t addr;
    uint32_t len = 1;

    // TODO: Also allow register to be used as address.
    if (!lp.GetNum(addr, 16))
    {
	lp.Error("Invalid address");
	return false;
    }
    if (!lp.Done() && !lp.GetNum(len))
    {
	lp.Error("Invalid lenght");
	return false;
    }
    int cnt = 0;
    for(uint32_t i = 0; i < len; i += size)
    {
	uint32_t v = cpu->ReadMem(addr, size);
	if (cnt == 0)
	{
	    std::cout << std::hex << std::setw(8) << std::setfill('0') << addr << ": ";
	}
	std::cout << std::hex << std::setw(2*size) << std::setfill('0') << v << " ";
	cnt += size;
	addr += size;
	if (cnt == 16)
	{
	    std::cout << std::endl;
	    cnt = 0;
	}
    }
    if (cnt != 0)
    {
	std::cout << std::endl;
    }
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
    cmdMap["db"]   = new DumpCmd("db", 1);
    cmdMap["dw"]   = new DumpCmd("dw", 2);
    cmdMap["dl"]   = new DumpCmd("dl", 4);
}

bool Command(LineParser& lp)
{
    static std::string lastcmd;
    if (lp.Line() == "" && lastcmd != "")
    {
	lp.SetLine(lastcmd);
    }
    std::string cmd = lp.GetWord();
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);    

    auto it = cmdMap.find(cmd);
    if (it != cmdMap.end())
    {
	bool result = it->second->DoIt(lp);
	if (it->second->Repeat())
	{
	    lastcmd = lp.Line();
	}
	else
	{
	    lastcmd = "";
	}
	return result;
    }

    lp.Error("Command not found");
    return false;
}
