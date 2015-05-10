#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <cstdint>

enum RegName
{
    R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15,
    SP = R14,
    PC = R15,
    MaxReg,
};

enum OperandSize
{
    Op8,
    Op16,
    Op32,
};

class Register
{
public:
    Register(uint32_t v = 0) : value(v) {}
    uint32_t& Value() { return value; }
    void Value(uint32_t v) { value = v; }
private:
    uint32_t value;
};

class FlagRegister
{
public:
    unsigned n:1;
    unsigned z:1;
    unsigned v:1;
    unsigned c:1;
};

enum AddrMode
{
    Direct,			/* Rn */
    Indir,			/* [Rn] */
    IndirAutoInc,		/* [Rn]+ */
    AutoDecIndir,		/* -[Rn] */
};

enum InstrKind
{
    /* Regular two operand instructions */
    NOP = 0,			/* Ignored size, source, dest. */
    MOV,
    CMP,
    ADD,
    ADC,
    SUB,
    SBC,
    MUL,
    DIV,
    AND,
    OR,
    XOR,
    NEG,
    COM,			/* Complement = NOT = ~ */
    ASR,			/* Arithmetic shift */
    ASL,			/* Arithmetic shift */
    LSR,			/* Logic shift */
    LSL,
    ROR,			/* Rotate */
    ROL,

    /* Flow control unconditional  - ignores dest operands and operand size */
    JSR = 32,
    RET,
    JMP,
    HLT,			/* Stop execution */
    
    /* Branch instructions */
    BEQ = 48,
    BNE,
    BLT,
    BGT,
    BGE,
    BLE,
    BHI,			/* Higher than (unsigned) */
    BLOS,			/* Lower or same */
    BCC,			/* Carry clear */
    BHIS = BCC,			/* Higher or same (unsigned) */
    BCS,			/* Carry set */
    BLO = BCS,
    BNG,			/* Negative */
    BPL,			/* Positive */
    BVC,			/* Overflow clear */
    BVS,			/* Overflow set */

    /* Special type instructions */
    EMT = 64,  			/* Emulation trap - call OS */

    MAX_INST = 255
};

class Instruction
{
public:
    Instruction() { value.word = 0; }
    union Instr
    {
	struct	
	{
	    union
	    {
		struct		/* Regular two operands */
		{		
		    RegName      source:8;
		    RegName      dest:8;
		    AddrMode     srcMode:2;
		    AddrMode     destMode:2;
		    uint32_t     unused:2;
		    OperandSize  size:2;
		    InstrKind    op:8;
		};
		struct		/* Brand instructions */
		{
		    int32_t      branch:24;
		    InstrKind    dummy_op:8;
		};
	    };
	};
	uint32_t word;
    };
    Instr value;
};

#endif
