#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <cstdint>

enum RegName
{
    R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15,
    SP = R14,
    PC = R15, 
};

enum OperandSize
{
    Op8,
    Op16,
    Op32,
};

class Register
{
    Register(uint32_t v) : value(v) {}
    uint32_t& Value() { return value; }
private:
    uint32_t value;
};

enum AddrMode
{
    Dir,			/* Rn */
    Indir,			/* [Rn] */
    IndirAuotInc,		/* [Rn]+ */
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
    JSR = 16,
    RET,
    JMP,
    
    /* Branch instructions */
    BEQ = 32,
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
    union Instr
    {
	struct	
	{
	    InstrKind    op:8;
	    union	       /* Regulard two operands */
	    {		
		OperandSize  size:2;
		uint32_t     unused:10;
		AddrMode     destMode:2;
	        AddrMode     srcMode:2;
		RegName      dest:8;
		RegName      source:8;
	    };
	    union		/* Brand instructions */
	    {
		int32_t     branch:24;
	    };
	};

	uint32_t word;
    };
};

#endif
