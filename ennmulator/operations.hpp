#pragma once
#include <cstdint>

#define JUMP_BASE 0x4000
#define ABSOLUTE  1
#define REG_BASE  2
#define LINK 	  4
#define NONZERO	  8
#define ZERO 	  16
#define PLUS_REG  32
#define MINUS_REG 64
#define IMMOFFSET 128
#define DISPLACE  256

#define PREFIX7F 0x2000

enum OP : uint16_t
{
	NOP = 0x0000,

	ADD, SUB, MULA, MULB, DIV, MOD,

	ADDI, SUBI, MOVI, INVI, LSHLI,
	LSHRI, ROTLI, ROTRI, SETI, CLRI,
	CBITI, TGLI, CEQI, CNEI, CGTI,
	CORI, CANDI, BORI, BANDI,

	// these don't clear previous condition state
	CEQ_OR,  CNE_OR,  CGT_OR,  CLT_OR,
	CEQI_OR, CNEI_OR, CGTI_OR, CLTI_OR,

	ADDHI, SUBHI, // add and sub to high byte

	ADDSAT, SUBSAT, MULSAT, // saturating

	CEQ, CNE,  CGT,   CLT,  //conditional
	COR, CAND, CNAND, CNOR, //conditional

	CGTS, CLTS, // signed
	CINV,       // reverse predication

	BOR,    BAND,    BXOR,    BNOR,    //bitwise
	BORINV, BANDINV, BXORINV, BNORINV, //inverse
	BNORI,  BXORI, CLTI,

	MOV, INV, // = MOV A, !B
	XCG, // swap Ra and Rb

	PADD,
	PSUB,
	PMUL,
	PDIV,
	PCPY,           // replicate low byte to all three lanes
	PBAND,
	PBOR,
	PBXOR,

	JUNKB,
	JUNKW,
	JUNKS, // trash top of stack

	PEEKB,
	PEEKW,
	PEEKS, // check top of stack w/o pop

	//  byte, word, sesqui
	STRB, STRW, STRS, // store
	LDRB, LDRW, LDRS, // load
	PSHB, PSHW, PSHS, // push
	POPB, POPW, POPS, // pop
	SWVB, SWVW, SWVS, // swerve

	ST2B, ST2W, ST2S, // store two
	LD2B, LD2W, LD2S, // load
	ST3B, ST3W, ST3S, // obsolete
	LD3B, LD3W, LD3S, // obsolete

	ST2BBD, ST2WBD, ST2SBD,
	LD2BBD, LD2WBD, LD2SBD,
	ST3BBD, ST3WBD, ST3SBD, // obsolete
	LD3BBD, LD3WBD, LD3SBD, // obsolete

	ST2BBI, ST2WBI, ST2SBI,
	LD2BBI, LD2WBI, LD2SBI,
	ST3BBI, ST3WBI, ST3SBI, // obsolete
	LD3BBI, LD3WBI, LD3SBI, // obsolete

	ST2BAD, ST2WAD, ST2SAD,
	LD2BAD, LD2WAD, LD2SAD,
	ST3BAD, ST3WAD, ST3SAD, // obsolete
	LD3BAD, LD3WAD, LD3SAD, // obsolete

	ST2BAI, ST2WAI, ST2SAI,
	LD2BAI, LD2WAI, LD2SAI,
	ST3BAI, ST3WAI, ST3SAI, // obsolete
	LD3BAI, LD3WAI, LD3SAI, // obsolete
  
	STRBI, STRWI, STRSI, // store  
	LDRBI, LDRWI, LDRSI, // load

	STRBBI,	STRWBI,  STRSBI,
	LDRBBI,	LDRWBI,  LDRSBI,
	STRBAI,	STRWAI,  STRSAI,
	LDRBAI,	LDRWAI,  LDRSAI,
	STRBBD,	STRWBD,  STRSBD,
	LDRBBD,	LDRWBD,  LDRSBD,
	STRBAD,	STRWAD,  STRSAD,
	LDRBAD,	LDRWAD,  LDRSAD,

	MVLA,  MVLB, MVLC, MVLD,
	MVMA,  MVMB, MVMC, MVMD,
	MVHA,  MVHB, MVHC, MVHD,
	MVLE,  MVLF, MVLG, MVLH,
	MVME,  MVMF, MVMG, MVMH,
	MVHE,  MVHF, MVHG, MVHH,

	PSHI, // push immediate byte

	END, // reverse endian
	REV, // reverse bits

	POPC,
	PARI, // parity
	LEAD, // leading zeros
	TAIL, // tailing zeros

	LSHL, LSHR,
	ROTL, ROTR,

	ZRO, FIL, // set reg to 0s and 1s

	SEED, // seed RNG with value in register
	RND,  // source 24 bits of randomness

	// read, write pairs for internal regs
	RSP, WSP,
	RIP, WIP,
	RPS, WPS,
	RXS, WXS, CXS, // and check if in an exception
	RXV, WXV, // exception vector

	// test and write MMU page properties
	TPAGE, WPAGE,
	MEMMAP, // map page dest to value src
	
	SWAP, // ABCDEFGH <--> A'B'C'D'E'F'G'H'
	SWPR, // swap only R with R'

	SHDW, // R' := R
	LITE, // R := R'

	SET, CLR, // set and clear bit #imm in reg
	CBIT,     // test bit #imm in reg
	TGL,      // invert bit #imm in reg

	GETL, GETM, GETH, // extract corresponding byte

	FADD, FSUB, FMUL, 
	FSQT, // square root
	FDIV, 
	FLOG, // log2
	FCNV, // convert to f24
	FCST, // trim to i24
	FSIN, FCOS, FTAN, FASIN,
	FEXP, FLN, // e^x and log_n
	FREC, // 1/x
	FABS, // |x|
	FNEG, // -x
	FCGT, FCLT, // gt / lt for floats
	FMOD,

	CINF, CNAN, CNRM, // non-denormal

	CALW, // condition always
	CNEV, // condition never

	FCPI,  FCE,   FC0,   FC1, 
	FC2,   FCSQ2, FCPHI, FCTAU,

	PSH2W,  PSH2S,  PSH3S,
	POP2W,  POP2S,  POP3S,

	PREF,

	FADD_48,
	FSUB_48,
	FMUL_48,
	FDIV_48,
	FMOD_48,

	FCST_24_48,
	FCST_48_24,

	FMADD_48, 

	FMADD_AB_48, 
	FMADD_CD_48, 
	FMADD_EF_48, 
	FMADD_GH_48,

	FMOV_48,

	LOOP, LRET,

	LRET_EQ,
	LRET_NE,
	LRET_GT,
	LRET_LT,
	LRET_FEQ24,
	LRET_FEQ48,
	LRET_FGT24,
	LRET_FGT48,

	INC,
	DEC,

	FCGT_24,
	FCGT_48,

	FABS_48,
	FNEG_48,
	FREC_48,
	FSQT_48,
	FLOG_48,
	FLN_48,
	FSIN_48,
	FCOS_48,
	FEXP_48,

	FCLT_24,
	FCLT_48,

	CNAN_24,
	CNAN_48,

	FCNV_I24_F48,
	FCNV_F48_I24,
	FCNV_F24_F48,
	FCNV_F48_F24,

	FAVG_48,

	VADD_2, VSUB_2, VMUL_2, VSHL_2, VSHR_2, VBAND_2,

	RET = 0x4000, // pop IP from stack

	// jumps, my eternal bane
	/*
		J  -- basic prefix
		M  -- no link
		L  -- pushes IP to stack
		Z  -- tests src for zero before jump
		NZ -- inverse of Z

		A  -- absolute jump
		O  -- imm8 offset rel. to IP
		R  -- jump to register
		RO -- jump to register +/- imm8

		PD -- IP + register
		SD -- IP - register

		e.g. JLZPD
			if reg1 == 0
				push IP to stack
				jump to (imm + reg2)
	*/

	// zero registers
	JMA = JUMP_BASE | ABSOLUTE,
	JLA = JUMP_BASE | ABSOLUTE | LINK,
	JMO = JUMP_BASE | IMMOFFSET,
	JLO = JUMP_BASE | IMMOFFSET | LINK,

	// one register, no imm
	JMR = JUMP_BASE | REG_BASE,
	JLR = JUMP_BASE | REG_BASE | LINK,

	JMD = JUMP_BASE | DISPLACE,
	JLD = JUMP_BASE | DISPLACE | LINK,

	JMPD= JUMP_BASE | PLUS_REG, 
	JLPD= JUMP_BASE | PLUS_REG | LINK,
	JMSD= JUMP_BASE | MINUS_REG, 
	JLSD= JUMP_BASE | MINUS_REG | LINK,

	// one register, one imm
	JMRO = JUMP_BASE | REG_BASE | IMMOFFSET,
	JLRO = JUMP_BASE | REG_BASE | IMMOFFSET | LINK,
	JMZO = JUMP_BASE | ZERO | IMMOFFSET,
	JLZO = JUMP_BASE | ZERO | IMMOFFSET | LINK,
	JMNZO= JUMP_BASE | NONZERO | IMMOFFSET,
	JLNZO= JUMP_BASE | NONZERO | IMMOFFSET | LINK,

	// two registers

	JMZR  = JUMP_BASE | ZERO,
	JMNZR = JUMP_BASE | NONZERO,
	JLZR  = JUMP_BASE | ZERO	| LINK,
	JLNZR = JUMP_BASE | NONZERO	| LINK,

	JMRD = JUMP_BASE | REG_BASE | DISPLACE,
	JLRD = JUMP_BASE | REG_BASE | DISPLACE | LINK,

	JMZD  = JUMP_BASE | ZERO | DISPLACE,
	JLZD  = JUMP_BASE | ZERO | DISPLACE | LINK,

	JMNZD = JUMP_BASE | NONZERO | DISPLACE,
	JLNZD = JUMP_BASE | NONZERO | DISPLACE | LINK,

	JMRPD = JUMP_BASE | REG_BASE | PLUS_REG,
	JLRPD = JUMP_BASE | REG_BASE | PLUS_REG | LINK,
	JMRSD = JUMP_BASE | REG_BASE | MINUS_REG,
	JLRSD = JUMP_BASE | REG_BASE | MINUS_REG | LINK,

	JMZPD  = JUMP_BASE | ZERO | PLUS_REG,
	JLZPD  = JUMP_BASE | ZERO | PLUS_REG | LINK,
	JMZSD  = JUMP_BASE | ZERO | MINUS_REG,
	JLZSD  = JUMP_BASE | ZERO | MINUS_REG | LINK,

	JMNZPD = JUMP_BASE | NONZERO | PLUS_REG,
	JLNZPD = JUMP_BASE | NONZERO | PLUS_REG | LINK,
	JMNZSD = JUMP_BASE | NONZERO | MINUS_REG,
	JLNZSD = JUMP_BASE | NONZERO | MINUS_REG | LINK,

	// far jumps

	JMFAR = JUMP_BASE | IMMOFFSET | PREFIX7F,
	JLFAR = JUMP_BASE | IMMOFFSET | LINK | PREFIX7F,

	DBG = 0x8000,
	DBGB, DBGW, DBGS,
	DBGF,
	DBGC,
	DBGINC,
	DBGINB,
	DBGINW,
	DBGINS,
	DBGINF,
	DBGA = 0x80ff,
	MMU_SETRO,
	MMU_CHKRO,
	MMU_SETUSR,
	MMU_CHKUSR,
	SYSCI, // syscall
	TRAPI, // exceptional
	KERNI, // jump to kernel
	ERET,  // return from kernel
	WFINT,
	MASK,
	UNMASK,
	KILL,
	RESET,
	RESERVED,
	LABEL, DIRECTIVE, COMMENT,
	ERR = 0xffff
};
