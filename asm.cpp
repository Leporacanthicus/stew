#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include "instruction.h"
#include "lineparser.h"

enum InstrType
{
    NoArgsType,
    NormalType,
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
    INSTR(MOV,  NormalType),
    INSTR(CMP,  NormalType),
    INSTR(ADD,  NormalType),
    INSTR(ADC,  NormalType),
    INSTR(SUB,  NormalType),
    INSTR(SBC,  NormalType),
    INSTR(MUL,  NormalType),
    INSTR(DIV,  NormalType),
    INSTR(AND,  NormalType),
    INSTR(OR,   NormalType),
    INSTR(XOR,  NormalType),
    INSTR(NEG,  NormalType),
    INSTR(COM,  NormalType),
    INSTR(ASR,  NormalType),
    INSTR(ASL,  NormalType),
    INSTR(LSR,  NormalType),
    INSTR(LSL,  NormalType),
    INSTR(ROR,  NormalType),
    INSTR(ROL,  NormalType),

    INSTR(JSR,  NormalType),
    INSTR(RET,  NoArgsType),
    INSTR(JMP,  NormalType),
    
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
};

struct ArgInfo
{
    AddrMode mode;
    RegName  reg;
    uint32_t Value;
};

std::vector<Instruction> code;
std::map<std::string,LabelInfo> labels;
size_t curAddr = 0;
size_t lineNo = 0;

class AsmLineParser: public LineParser
{
public:
    using LineParser::LineParser;
    bool IsSeparator(char c) override;
    void ErrOutput(const std::string& msg) override;
};

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

void Error(const std::string& msg)
{
    std::cerr << lineNo << ": " << msg << std::endl;
}

void AsmLineParser::ErrOutput(const std::string& msg)
{
    ::Error(msg);
}

void AddLabel(const std::string& name)
{
    if (labels.find(name) != labels.end())
    {
	Error("Label already defined: " + name);
	return;
    }
    LabelInfo li;

    li.name = name;
    li.addr = curAddr;

    labels[name] = li;
}

bool GetRegister(LineParser& lp, RegName& reg)
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

bool GetNormalArg(LineParser& lp, ArgInfo& info)
{
    bool maybeAutoDecr = false;
    RegName rn;
    if (lp.Accept('-'))
    {
	maybeAutoDecr = true;
    }
    if (lp.Accept('('))
    {
	if (!GetRegister(lp, rn))
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
	    info.mode = IndirAuotInc;
	}
	return true;
    }

    if (!GetRegister(lp, rn))
    {
	lp.Error("Expected register name");
	return false;
    }
    info.reg = rn;
    info.mode = Direct;
    return true;
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
	    AddLabel(w.substr(0, w.length()-1));
	    continue;
	}
	InstrEntry e;
	if (FindInstruction(w, e))
	{
	    OperandSize opsize = Op32;
	    if(lp.Accept('.'))
	    {
		char ch = lp.Get();
		switch(ch)
		{
		case 'w':
		    opsize = Op16;
		    break;
		case 'b':
		    opsize = Op8;
		    break;
		case 'l':
		    opsize = Op32;
		    break;
		default:
		    lp.Error(std::string("Unknown size value ") + ch);
		    return;
		}
	    }
	    switch(e.type)
	    {
	    case NormalType:
		ArgInfo arg1;
		if (GetNormalArg(lp, arg1))
		{		    
		    lp.Expect(',');
		    ArgInfo arg2;
		    if (GetNormalArg(lp, arg2))
		    {
			Instruction instr;
			instr.value.op = e.op;
			instr.value.size = opsize;
			instr.value.destMode = arg2.mode;
			instr.value.destMode = arg1.mode;
			instr.value.dest = arg2.reg;
			instr.value.source = arg1.reg;
			std::cout << std::hex << instr.value.word << std::endl;
			code.push_back(instr);
			curAddr+=4;
		    }
		}
		break;
	    default:
		lp.Error("Not yet implemented");
		return;
	    }
	}
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
