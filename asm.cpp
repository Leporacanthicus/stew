#include <string>
#include <iostream>
#include <vector>
#include "instruction.h"

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
    InstrKind kind;
    InstrType type;
};

InstrEntry instructions[] = 
{
    INSTR(NOP, NoArgsType),
    INSTR(MOV, NormalType),
    INSTR(CMP, NormalType),
    INSTR(ADD, NormalType),
    INSTR(ADC, NormalType),
    INSTR(SUB, NormalType),
    INSTR(SBC, NormalType),
    INSTR(MUL, NormalType),
    INSTR(AND, NormalType),
    INSTR(OR, NormalType),
    INSTR(XOR, NormalType),
    INSTR(NEG, NormalType),
    INSTR(COM, NormalType),
    INSTR(ASR, NormalType),
    INSTR(ASL, NormalType),
    INSTR(LSR, NormalType),
    INSTR(LSL, NormalType),
    INSTR(ROR, NormalType),
    INSTR(ROL, NormalType),

    INSTR(JSR, NormalType),
    INSTR(RET, NoArgsType),
    INSTR(JMP, NormalType),
    
    INSTR(BEQ, BranchType),
    INSTR(BNE, BranchType),
    INSTR(BLT, BranchType),
    INSTR(BGT, BranchType),
    INSTR(BGE, BranchType),
    INSTR(BLE, BranchType),
    INSTR(BHI, BranchType),
    INSTR(BLOS, BranchType),
    INSTR(BCC, BranchType),
    INSTR(BHIS, BranchType),
    INSTR(BCS, BranchType),
    INSTR(BLO, BranchType),
    INSTR(BNG, BranchType),
    INSTR(BPL, BranchType),
    INSTR(BVC, BranchType),
    INSTR(BVS, BranchType),

    INSTR(EMT, EmtType)
};

std::vector<Instruction> code;

bool IsSeparator(const char c)
{
    switch(c)
    {
    case ',':
    case ')':
    case '(':
	return true;
    default:
	return false;
    }
    
}

void SkipSpaces(const std::string& line, std::string::size_type& pos)
{
    std::string::size_type len = line.length();
    while(isspace(line[pos]) && pos < len)
    {
	pos++;
    }
}    

std::string GetWord(const std::string& line, std::string::size_type& pos)
{
    std::string::size_type len = line.length();
    std::string w;
    while(pos < len && !isspace(line[pos]) && !IsSeparator(line[pos]))
    {
	w += line[pos];
	pos++;
    }
    SkipSpaces(line, pos);
    return w;
}

void Parse(const std::string& line)
{
    std::string::size_type pos = 0;
    std::string::size_type len = line.length();
    SkipSpaces(line, pos);
    if (pos == len) return;
    if (line[pos] == '#') return;

    while(pos < len)
    {
	std::string w = GetWord(line, pos);
	if (IsSeparator(line[pos])) pos++;
	std::cout << w << std::endl;
    }
}
    
int main()
{
    std::string line;
    while(std::getline(std::cin, line))
    {
	std::cout << "Parse..." << std::endl;
	Parse(line);
    }
}
