#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include "instruction.h"
#include "lineparser.h"

enum InstrType
{
    NoArgsType,
    OneArgType,
    TwoArgType,
    BranchType,
    EmtType,
};

#define INSTR(x, t) { #x, x, t }

struct InstrEntry
{
    const char* name;
    InstrKind op;
    InstrType type;
};

InstrEntry instructions[] =
{
    INSTR(NOP,  NoArgsType),
    INSTR(MOV,  TwoArgType),
    INSTR(CMP,  TwoArgType),
    INSTR(ADD,  TwoArgType),
    INSTR(ADC,  TwoArgType),
    INSTR(SUB,  TwoArgType),
    INSTR(SBC,  TwoArgType),
    INSTR(MUL,  TwoArgType),
    INSTR(DIV,  TwoArgType),
    INSTR(AND,  TwoArgType),
    INSTR(OR,   TwoArgType),
    INSTR(XOR,  TwoArgType),
    INSTR(NEG,  TwoArgType),
    INSTR(COM,  TwoArgType),
    INSTR(ASR,  TwoArgType),
    INSTR(ASL,  TwoArgType),
    INSTR(LSR,  TwoArgType),
    INSTR(LSL,  TwoArgType),
    INSTR(ROR,  TwoArgType),
    INSTR(ROL,  TwoArgType),

    INSTR(JSR,  OneArgType),
    INSTR(RET,  NoArgsType),
    INSTR(JMP,  OneArgType),
    
    INSTR(BEQ,  BranchType),
    INSTR(BNE,  BranchType),
    INSTR(BLT,  BranchType),
    INSTR(BGT,  BranchType),
    INSTR(BGE,  BranchType),
    INSTR(BLE,  BranchType),
    INSTR(BHI,  BranchType),
    INSTR(BLOS, BranchType),
    INSTR(BCC,  BranchType),
    INSTR(BHIS, BranchType),
    INSTR(BCS,  BranchType),
    INSTR(BLO,  BranchType),
    INSTR(BNG,  BranchType),
    INSTR(BPL,  BranchType),
    INSTR(BVC,  BranchType),
    INSTR(BVS,  BranchType),

    INSTR(EMT,  EmtType)
};

struct LabelInfo
{
    std::string name;
    size_t addr;
    bool needBP;
};

struct ArgInfo
{
    ArgInfo()
	{
	    mode = static_cast<AddrMode>(0);
	    reg = static_cast<RegName>(0);
	    useData = false;
	    data = 0;
	    label = 0;
	};
    ~ArgInfo()
	{
	    delete label;
	};
    AddrMode mode;
    RegName  reg;
    bool     useData;
    uint32_t data;
    std::string* label;
};

struct BackPatch
{
    std::string label;
    size_t location;
    size_t branchAddr;
};

std::vector<Instruction> code;
std::map<std::string,LabelInfo> labels;
std::vector<BackPatch> backPatchList;

size_t curAddr = 0;
size_t lineNo = 0;

class AsmLineParser: public LineParser
{
public:
    using LineParser::LineParser;
    bool IsSeparator(char c) override;
    void ErrOutput(const std::string& msg) override;
};

void Error(const std::string& msg)
{
    std::cerr << lineNo << ": " << msg << std::endl;
}

bool AsmLineParser::IsSeparator(const char c)
{
    if (Done() || isspace(c))
    {
	return true;
    }
    switch(c)
    {
    case ',':
    case ')':
    case '(':
    case '.':
	return true;
    default:
	return false;
    }
}

void AsmLineParser::ErrOutput(const std::string& msg)
{
    ::Error(msg);
}

bool FindInstruction(const std::string& instr, InstrEntry& e)
{
    std::string str = instr;
    std::transform(str.begin(), str.end(),str.begin(), ::toupper);    
    for(auto x : instructions)
    {
	if (x.name == str)
	{
	    e = x;
	    return true;
	}
    }
    return false;
}

void AddLabel(const std::string& name, size_t addr)
{
    if (labels.find(name) != labels.end())
    {
	Error("Label already defined: " + name);
	return;
    }

    LabelInfo li;
    li.name = name;
    li.addr = addr;
    li.needBP = false;

    labels[name] = li;
    
    for(auto bp = backPatchList.begin(); bp != backPatchList.end();)
    {
	if (bp->label == name)
	{
	    if (bp->branchAddr)
	    {
		code[bp->location].value.branch = addr - bp->branchAddr;
	    }
	    else
	    {
		code[bp->location].value.word = addr;
	    }
	    bp = backPatchList.erase(bp);
	}
	else
	{
	    bp++;
	}
    }
}

bool ParseLabel(LineParser& lp, LabelInfo& label)
{
    lp.Save();
    std::string name = lp.GetWord();
    if (name != "")
    {
	auto it = labels.find(name);
	if (it != labels.end())
	{
	    label = it->second;
	}
	else
	{
	    label.name = name;
	    label.addr = 0;
	    label.needBP = true;
	}
	return true;
    }
    lp.Restore();
    return false;
}

bool ParseRegister(LineParser& lp, RegName& reg)
{
    lp.Save();
    lp.SkipSpaces();
    if (lp.Accept('r'))
    {
	char ch = lp.Get();
	int n;
	if (isdigit(ch))
	{
	    n = ch - '0';
	    if (ch == '1')
	    {
		n = 10;
		ch = lp.Peek();
		if (ch >= '0' && ch <= '5')
		{
		    n += ch - '0';
		    lp.Get();
		}
	    }
	    if (lp.IsSeparator(lp.Peek()))
	    {
		reg = static_cast<RegName>(n);
		return true;
	    }
	}
    }
    else if (lp.Accept('p') && lp.Accept('c'))
    {
	if (lp.IsSeparator(lp.Peek()))
	{
	    reg = R15;
	    return true;
	}
    }
    else if (lp.Accept('s') && lp.Accept('p'))
    {
	if (lp.IsSeparator(lp.Peek()))
	{
	    reg = R14;
	    return true;
	}
    }
    lp.Restore();
    return false;
}

bool ParseArg(LineParser& lp, ArgInfo& info)
{
    bool maybeAutoDecr = false;
    RegName rn;
    if (lp.Accept('-'))
    {
	maybeAutoDecr = true;
    }
    if (lp.Accept('('))
    {
	if (!ParseRegister(lp, rn))
	{
	    lp.Error("Expected register name");
	    return false;
	}
	info.reg = rn;
	info.mode = Indir;
	lp.Expect(')');
	if (maybeAutoDecr)
	{
	    info.mode = AutoDecIndir;
	}
	else if (lp.Accept('+'))
	{
	    info.mode = IndirAutoInc;
	}
	return true;
    }

    if (ParseRegister(lp, rn))
    {
	info.reg = rn;
	info.mode = Direct;
	return true;
    }
    
    if (lp.Accept('#'))
    {
	std::string w = lp.GetWord();
	std::cout << "w=" << w << std::endl;
	int value = std::stoi(w);
	
	info.data = value;
	info.useData = true;
	info.mode = IndirAutoInc;
	info.reg = PC;
	return true;
    }

    LabelInfo label;
    if (ParseLabel(lp, label))
    {
	info.useData = true;
	info.data = label.addr;
	info.mode = IndirAutoInc;
	info.reg = PC;
	if (label.needBP)
	{
	    info.label = new std::string(label.name);
	}
	return true;
    }

    lp.Error("Expected register name");
    return false;
}

bool ParseOpSize(LineParser& lp, OperandSize &opSize)
{
    opSize = Op32;
    if (lp.Accept('.'))
    {
	switch(lp.Get())
	{
	case 'b':
	    opSize = Op8;
	    break;
	case 'w':
	    opSize = Op16;
	    break;
	case 'l':
	    opSize = Op32;
	    break;
	default:
	    lp.Error("Bad operand size");
	    return false;
	}
    }
    return true;
}

void SaveArg(const ArgInfo& arg)
{
    if (arg.useData)
    {
	Instruction data;
	data.value.word = arg.data;
	if (arg.label)
	{
	    BackPatch bp = { *arg.label, code.size(), 0 };
	    backPatchList.push_back(bp);
	}
	code.push_back(data);
	curAddr += 4;
    }
}

void StoreInstr(InstrKind op, OperandSize opsize,
		const ArgInfo& arg1, const ArgInfo& arg2)
{
    Instruction instr;
    instr.value.op = op;
    instr.value.size = opsize;
    instr.value.destMode = arg2.mode;
    instr.value.srcMode = arg1.mode;
    instr.value.dest = arg2.reg;
    instr.value.source = arg1.reg;
    code.push_back(instr);
    curAddr += 4;
    SaveArg(arg1);
    SaveArg(arg2);
}

void StoreInstr(InstrKind op, OperandSize opsize, const ArgInfo& arg1)
{
    ArgInfo arg2;
    StoreInstr(op, opsize, arg1, arg2);
}

void StoreInstr(InstrKind op)
{
    ArgInfo arg1;
    ArgInfo arg2;
    OperandSize opsize;
    memset(&opsize, 0, sizeof(opsize));
    StoreInstr(op, opsize, arg1, arg2);
}

void StoreInstr(InstrKind op, int distance)
{
    Instruction instr;
    instr.value.branch = distance;
    instr.value.op = op;
    code.push_back(instr);
    curAddr += 4;
}

void ParseTwoArgs(LineParser& lp, InstrKind op, OperandSize opsize)
{
    ArgInfo arg1;
    if (ParseArg(lp, arg1))
    {		    
	lp.Expect(',');
	ArgInfo arg2;
	if (ParseArg(lp, arg2))
	{
	    StoreInstr(op, opsize, arg1, arg2);
	}
    }
}

void ParseOneArg(LineParser& lp, InstrKind op, OperandSize opsize)
{
    ArgInfo arg1;
    if (ParseArg(lp, arg1))
    {		    
	StoreInstr(op, opsize, arg1);
    }
}

void ParseBranch(LineParser& lp, InstrKind op)
{
    LabelInfo label;
    if (ParseLabel(lp, label))
    {
	int distance = 0;
	if (label.needBP)
	{
	    BackPatch bp = { label.name, code.size(), curAddr + 4 };
	    backPatchList.push_back(bp);
	}
	else
	{
	    distance = label.addr - (curAddr + 4);
	}
	StoreInstr(op, distance);
    }
    else
    {
	lp.Error("Expected branch label?");
    }
}

void Parse(const std::string& line)
{
    AsmLineParser lp(line);
    lp.SkipSpaces();
    if (lp.Peek() == ';') return;
    
    while(!lp.Done())
    {
	std::string w = lp.GetWord();
	if (w != "" && w[w.length()-1] == ':')
	{
	    AddLabel(w.substr(0, w.length()-1), curAddr);
	    continue;
	}
	InstrEntry e;
	if (FindInstruction(w, e))
	{
	    OperandSize opsize;
	    if (!ParseOpSize(lp, opsize))
	    {
		continue;
	    }
	    switch(e.type)
	    {
	    case TwoArgType:
	    {
		Instruction instr;
		ParseTwoArgs(lp, e.op, opsize);
		break;
	    }
	    case OneArgType:
	    {
		ParseOneArg(lp, e.op, opsize);
		break;
	    }
	    case NoArgsType:
	    {
		StoreInstr(e.op);
		break;
	    }
	    case BranchType:
	    {
		ParseBranch(lp, e.op);
		break;
	    }
	    default:
		lp.Error("Not yet implemented");
		return;
	    }
	}
	// TODO: Add pseudo-instructions.
	else
	{
	    lp.Error("Invalid instruction");
	}
    }
}
    
int main()
{
    std::string line;
    while(std::getline(std::cin, line))
    {
	lineNo++;
	Parse(line);
    }
    for(auto v : code)
    {
	std::cout << std::hex << v.value.word << std::endl;
    }
}
