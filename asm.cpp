#include <string>
#include <fstream>
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

enum SectionType
{
    Code,
    Data,
    BSS,
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
    INSTR(HLT,  NoArgsType),
    
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

class AsmLineParser: public LineParser
{
public:
    using LineParser::LineParser;
    bool IsSeparator(char c) override;
    void ErrOutput(const std::string& msg) override;
};

struct BackPatch
{
    std::string label;
    size_t location;
    size_t branchAddr;
};

std::vector<uint8_t> code;
std::vector<uint8_t> data;
size_t bss_size;
std::map<std::string,LabelInfo> labels;
std::vector<BackPatch> backPatchList;

size_t curAddr = 0;
size_t lineNo = 0;
SectionType section = Code;

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
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    // TODO: We should use a map or something to make it faster..
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
	    std::vector<uint8_t> code_bytes(sizeof(Instruction));
	    addr -= bp->branchAddr;
	    memcpy(code_bytes.data(), &addr, sizeof(Instruction));
	    int size = 4;
	    if (bp->branchAddr)
	    {
		size = 3;
	    }
	    for(int i = 0; i < size; i++)
	    {
		code[bp->location + i] = code_bytes[i];
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
		else
		{
		    n = 1;
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
    lp.SkipSpaces();
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

void CodeStore(Instruction instr)
{
    std::vector<uint8_t> code_bytes(sizeof(instr));
    memcpy(code_bytes.data(), &instr, sizeof(instr));
    code.insert(code.end(), code_bytes.begin(), code_bytes.end());
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
	CodeStore(data);
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
    CodeStore(instr);
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
    CodeStore(instr);
    curAddr += 4;
}

bool ParseTwoArgs(LineParser& lp, InstrKind op, OperandSize opsize)
{
    ArgInfo arg1;
    if (ParseArg(lp, arg1))
    {		    
	lp.Expect(',');
	ArgInfo arg2;
	if (ParseArg(lp, arg2))
	{
	    StoreInstr(op, opsize, arg1, arg2);
	    return true;
	}
    }
    return false;
}

bool ParseOneArg(LineParser& lp, InstrKind op, OperandSize opsize)
{
    ArgInfo arg1;
    if (ParseArg(lp, arg1))
    {		    
	StoreInstr(op, opsize, arg1);
	return true;
    }
    return false;
}

bool ParseBranch(LineParser& lp, InstrKind op)
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
	return true;
    }

    lp.Error("Expected branch label?");
    return false;
}

bool ParseEmt(LineParser& lp, InstrKind op)
{
    uint32_t n = 0;
    if (lp.GetNum(n))
    {	
	StoreInstr(op, static_cast<int>(n));
	return true;
    }
    return false;
}

void StoreBytesToSection(const std::vector<uint8_t> bytes)
{
    if (section == Data)
    {
	data.insert(data.end(), bytes.begin(), bytes.end());
    }
    else
    {
	code.insert(code.end(), bytes.begin(), bytes.end());
    }
}


bool ParseDb(LineParser& lp)
{
    std::vector<uint8_t> bytes;
    char ch;
    while(!lp.Done())
    {
	if (lp.Accept('"'))
	{
	    while(!lp.Accept('"'))
	    {
		if (!lp.Done())
		{
		    lp.Error("String termination not found");
		    return false;
		}
		ch = lp.Get();
		bytes.push_back(static_cast<uint8_t>(ch));
	    }
	}
	else
	{
	    if (lp.Peek() == ',')
	    {
		lp.Get();
	    }
	    uint32_t n = 0;
	    if (!lp.GetNum(n))
	    {
		lp.Error("Expected number here");
		return false;
	    }
	    bytes.push_back(static_cast<uint8_t>(n));
	}
    }

    StoreBytesToSection(bytes);
    return true;
}

bool ParseConstant(LineParser& lp, uint32_t size)
{
    uint32_t value;
    if (lp.GetNum(value))
    {
	std::vector<uint8_t> bytes(size);
	memcpy(bytes.data(), &value, size);
	StoreBytesToSection(bytes);
	return true;
    }
    return false;
}

bool ParsePseudoOp(LineParser& lp)
{
    if (!lp.Accept('.'))
    {
	return false;
    }
    std::string op = lp.GetWord();
    if (op == "db")
    {
	return ParseDb(lp);
    }
    else if (op == "long")
    {
	return ParseConstant(lp, 4);
    }
    else if (op == "word")
    {
	return ParseConstant(lp, 2);
    }
    else if (op == "byte")
    {
	return ParseConstant(lp, 1);
    }
    return false;
}

bool ParseInstruction(LineParser& lp, std::string w)
{
    InstrEntry e;
    if (FindInstruction(w, e))
    {
	OperandSize opsize;
	if (!ParseOpSize(lp, opsize))
	{
	    return false;
	}
	switch(e.type)
	{
	case TwoArgType:
	    return ParseTwoArgs(lp, e.op, opsize);

	case OneArgType:
	    return ParseOneArg(lp, e.op, opsize);

	case NoArgsType:
	    StoreInstr(e.op);
	    return true;

	case BranchType:
	    return ParseBranch(lp, e.op);

	case EmtType:
	    return ParseEmt(lp, e.op);
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
	    AddLabel(w.substr(0, w.length()-1), curAddr);
	    continue;
	}

	if (section == Code && ParseInstruction(lp, w))
	{
	    break;
	}

	if (!ParsePseudoOp(lp))
	{
	    lp.Error("Syntax error");
	}
    }
}

void Output(std::ostream& out, const std::vector<uint8_t>& binary)
{
    int count = 0;
    for(auto v : binary)
    {
	out << std::hex << static_cast<uint32_t>(v) << " ";
	count++;
	if (count == 16)
	{
	    out << std::endl;
	    count = 0;
	}
    }
    if (count)
    {
	out << std::endl;
    }
}

void Assemble(std::istream& in, std::ostream& out)
{
    std::string line;
    while(std::getline(in, line))
    {
	lineNo++;
	Parse(line);
    }
    Output(out, code);
    Output(out, data);
}

int main(int argc, char **argv)
{
    std::istream *in = &std::cin;
    std::ostream *out = &std::cout;
    std::ifstream f;
    if (argc > 1)
    {
	f.open(argv[1]);
	if (!f)
	{
	    std::cerr << "Could not open file: " << argv[1] << std::endl;
	    return 1;
	}
	in = &f;
    }
    std::ofstream of;
    if (argc > 2)
    {
	of.open(argv[2]);
	if (!of)
	{
	    std::cerr << "Could not open file: " << argv[2] << std::endl;
	    return 1;
	}
	out = &of;
    }
    Assemble(*in, *out);
    return 0;
}
