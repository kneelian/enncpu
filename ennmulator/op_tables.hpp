#include <array>
#include <unordered_map>
#include <string>
#include "operations.hpp"

OP REG_REG_TABLE[][8] =
{
	{ ERR,    STRB,        STRW,     STRS,
	  END, 	  LDRB,	       LDRW,     LDRS}, // 0000xxx
	{ ADD, 	  SUB,         MULA,     MULB, 
	  DIV, 	  MOD,         MOV,      INV }, // 0001
	{ CEQ, 	  CNE,         CGT,      CLT,
	  COR, 	  CAND,	       CNAND,    CNOR}, // 0010

	{ BOR,	  BAND,	       BXOR,     BNOR,
	  NOP,    NOP,         NOP,      NOP }, // 0011 !!

	{ POPC,	  PARI,	       LEAD,	 TAIL,
	  LSHL,	  LSHR,	       ROTL,	 ROTR}, // 0100
	{ FADD,	  FSUB,	       FMUL,	 FSQT,
	  FDIV,	  FLOG,	       FCNV,	 FCST}, // 0101
	{ FSIN,	  FCOS,	       FTAN,	 FASIN,
	  FEXP,	  FLN,	       FREC,	 FABS}, // 0110
	{ FNEG,	  FCGT,	       FCLT,	 FMOD,
	  REV,	  MEMMAP,      XCG,      NOP }, // 0111

	{ JMZR,   JLZR,        JMNZR,    JLNZR,
	  CEQ_OR, CNE_OR,      CGT_OR,   CLT_OR },

	{ CGTS, CLTS,   ADDSAT, SUBSAT, 
	  GETL, GETM,   GETH,   MULSAT },	   //  1001

	{ NOP,    /**/  STRBBI,	STRWBI,  STRSBI,
	  NOP,    /**/  LDRBBI,	LDRWBI,  LDRSBI }, // 1010 beforeincrement
	{ ST2SAI, /**/  STRBAI,	STRWAI,  STRSAI,
	  LD2SAI, /**/  LDRBAI,	LDRWAI,  LDRSAI },  // 1011 afterincrement

	{ ST2SBD, /**/  STRBBD,	STRWBD,  STRSBD,
	  LD2SBD, /**/  LDRBBD,	LDRWBD,  LDRSBD },  // 1100 beforedecrement
	{ ST2S,   /**/  STRBAD,	STRWAD,  STRSAD,
	  LD2S,   /**/  LDRBAD,	LDRWAD,  LDRSAD },  // 1101 afterdecrement

	{ NOP, NOP /**/ PADD,    PBAND,
	  NOP, NOP /**/ PSUB,    PBOR },   // 1110
	{ NOP, NOP /**/ PMUL,    PBXOR,
	  NOP, NOP /**/ PDIV,    PCPY },   // 1111
};

const OP REG_IMM_TBLA[] =
{
	ADDI,    // 0000
	SUBI,    // 0001
	MOVI,    // 0010
	ADDHI,   // 0011
	LSHLI,   // 0100
	LSHRI,   // 0101
	NOP,         // 0110 <-- PREFIX FOR SOLO_A
	NOP,         // 0111 <-- PREFIX FOR SOLO_B
	SETI,    // 1000
	CLRI,    // 1001
	CBITI,   // 1010
	TGLI,    // 1011
	SUBHI,   // 1100
	CEQI_OR, // 1101
	CNEI_OR, // 1110
	NOP,        // !!  1111
};
	
const OP REG_IMM_TBLB[] =
{
	CEQI,  
	CNEI,  
	CGTI,  
	CLTI,     // 0011
	CORI,     // 0100
	CANDI,    // 0101
	NOP,  
	NOP,   
	JMZO,     // 1000
	JLZO,     // 1001
	JMNZO,    // 1010
	JLNZO,    // 1011
	BORI,     // 1100
	CGTI_OR,  // 1101
	CLTI_OR,  // 1110
	BANDI,    // 1111	
};

const OP REG_SOLO_TBLA[] =
{
	PSHB, PSHW, PSHS, NOP,  // 0000xx
	POPB, POPW, POPS, NOP,  // 0001xx
	TPAGE,WPAGE,RND,  ZRO,  // 0010xx
	FIL,  SWPR, SHDW, LITE, // 0011xx
	RSP,  WSP,  RIP,  WIP,  // 0100
	RPS,  WPS,  RXS,  WXS,  // 0101
	JMR,  JLR,  RXV,  WXV, // 0110
	ERR,  ERR,  SEED, NOP, // 0111

	FCPI, 	FCE,  	FC0,  	FC1,  // 1000
	FC2,  	FCSQ2,	FCPHI,	FCTAU,// 1001xx
	CINF, 	CNAN, 	CNRM, 	ERR,  // 1010xx
	PSH2W,  PSH2S,  ERR,    ERR,  // 1100xx
	POP2W,  POP2S,  ERR,    ERR,  // 1101xx
	MMU_SETRO,   // 111000
	MMU_CHKRO,   // 111001
	MMU_SETUSR,  // 111010
	MMU_CHKUSR,  // 111011 
	PEEKB, 	PEEKW, 	PEEKS, 	ERR,  // 1111xx 
};

const OP REG_SOLO_TBLB[] =
{
		ERR, ERR, ERR, ERR,
		ERR, ERR, ERR, ERR,
		ERR, ERR, ERR, ERR,
		ERR, ERR, ERR, ERR,

		ERR, ERR, ERR, ERR,
		ERR, ERR, ERR, ERR,
		ERR, ERR, ERR, ERR,
		ERR, ERR, ERR, ERR,

		ERR, ERR, ERR, ERR,
		ERR, ERR, ERR, ERR,
		ERR, ERR, ERR, ERR,
		ERR, ERR, ERR, ERR,

		ERR, ERR, ERR, ERR,
		ERR, ERR, ERR, ERR,
		ERR, ERR, ERR, ERR,
		ERR, ERR, ERR, ERR,
};

const OP NO_REGISTERS[] =
	{
		SWVB, SWVW,   SWVS,  NOP,    // 0000xx
		SWAP, JUNKB,  JUNKW, JUNKS,  // 0001
		RET,  ERET,   RESET, DBG,    // 0010
		CALW, CNEV,   CINV,  WFINT,  // 0011
		MASK, UNMASK, CXS,   NOP,    // 0100 
	};

const std::array<OP, 64> DEBUGS =
	{
		DBGB,   DBGW,   DBGS,    DBGA, // 0000xx
		DBGF,   DBGC,   ERR,     ERR,
		DBGINB, DBGINW, DBGINS,  ERR,
		DBGINF, DBGINC, ERR,     ERR	
	};

const std::array<OP, 16> IMM8_A =
{
	JMA,   JLA,  JMO,  JLO,
	MVLA,  MVLB, MVLC, MVLD,
	MVMA,  MVMB, MVMC, MVMD,
	MVHA,  MVHB, MVHC, MVHD
};

const std::array<OP, 16> IMM8_B =
{
	MVLE,  MVLF, MVLG, MVLH,
	MVME,  MVMF, MVMG, MVMH,
	MVHE,  MVHF, MVHG, MVHH,
	SYSCI, KERNI,PSHI, PREF, // prefix
};

const std::array<OP, 32> FP48_TYPE =
{
	NOP, NOP, NOP, NOP,

	FADD_48,  FSUB_48,  FMUL_48,  FDIV_48,
	FABS_48,  FNEG_48,  FREC_48,  FSQT_48,
	FLOG_48,  FLN_48,   FSIN_48,  FCOS_48,
	FMOV_48,  FCGT_48,  FCLT_48,  FEXP_48,

	NOP,      NOP,      NOP,      NOP,
	NOP,      NOP,      NOP,      NOP,
	NOP,      NOP,      NOP,      NOP
};

const std::array<OP, 8> PRE_1_FP24_OPS_A =
{
	FADD, FSUB, FMUL, FDIV,
	FSQT, FLOG, FNEG, FMOD,
};

const std::array<OP, 8> PRE_1_FP24_OPS_B =
{
	FCGT, FCLT, FCNV, FCST,
	NOP,  NOP,  NOP,  NOP
};

const std::array<OP, 8> PRE_1_MEM_OPS_A =
{
	LDRS, STRS, LD2S, ST2S,
	NOP, NOP, PSHS, POPS,
};

const std::array<OP, 8> PRE_1_MEM_OPS_B =
{
	LDRSAI, STRSAI, LDRSAD, STRSAD,
	LDRSBI, STRSBI, LDRSBD, STRSBD,
};


const std::array<OP, 8> PRE_1_MEM_OPS_C =
{
	LD2SAI, ST2SAI, LD2SAD, ST2SAD,
	LD2SBI, ST2SBI, LD2SBD, ST2SBD,
};


const std::array<OP, 8> PRE_1_MEM_OPS_D =
{
	NOP, NOP, NOP, NOP,
	NOP, NOP, NOP, NOP,
};
 
std::unordered_map<OP, std::string> unmappings =
{ 
	{ JMFAR, "JMFAR" },
	{ JLFAR, "JLFAR" },
	{ CXS, "CXS" },
	{ MASK, "MASK" },
	{ UNMASK, "UNMASK" },
	{ STRWAI, "STRWAI" },
	{ STRWAD, "STRWAD" },
	{ STRWBI, "STRWBI" },
	{ STRWBD, "STRWBD" },
	{ WFINT, "WFI" },
	{ ERET, "ERET" },
	{ NOP,  "NOP" }, 
	{ ADD,  "ADD" }, 
	{ SUB,  "SUB" }, 
	{ PREF, "PRE" },
	{ MULA,  "MULA" }, 
	{ MULB,  "MULB" }, 
	{ DIV,  "DIV" }, 
	{ MOD,  "MOD" }, 
	{ ADDI,  "ADDI" }, 
	{ SUBI,  "SUBI" }, 
	{ MOVI,  "MOVI" }, 
	{ INVI,  "INVI" }, 
	{ LSHLI,  "LSHLI" }, 
	{ LSHRI,  "LSHRI" }, 
	{ ROTLI,  "ROTLI" }, 
	{ ROTRI,  "ROTRI" }, 
	{ SETI,  "SETI" }, 
	{ CLRI,  "CLRI" }, 
	{ CBITI,  "CBITI" }, 
	{ TGLI,  "TGLI" }, 
	{ CEQI,  "CEQI" }, 
	{ CNEI,  "CNEI" }, 
	{ CGTI,  "CGTI" }, 
	{ CORI,  "CORI" }, 
	{ CANDI,  "CANDI" }, 
	{ BORI,  "BORI" }, 
	{ BANDI,  "BANDI" }, 
	{ ADDSAT,  "ADDSAT" }, 
	{ SUBSAT,  "SUBSAT" }, 
	{ MULSAT,  "MULSAT" }, 
	{ SYSCI, "SYSCI" },
	{ KERNI, "KERNI" },
	{ CEQ,  "CEQ" }, 
	{ CNE,  "CNE" }, 
	{ CGT,  "CGT" }, 
	{ CLT,  "CLT" }, 
	{ COR,  "COR" }, 
	{ CAND,  "CAND" }, 
	{ CNAND,  "CNAND" }, 
	{ CNOR,  "CNOR" }, 
	{ CGTS,  "CGTS" }, 
	{ CLTS,  "CLTS" }, 
	{ CINV,  "CINV" }, 
	{ BOR,  "BOR" }, 
	{ BAND,  "BAND" }, 
	{ BXOR,  "BXOR" }, 
	{ BNOR,  "BNOR" }, 
	{ BORINV,  "BORINV" }, 
	{ BANDINV,  "BANDINV" }, 
	{ BXORINV,  "BXORINV" }, 
	{ BNORINV,  "BNORINV" }, 
	{ BNORI,  "BNORI" }, 
	{ BXORI,  "BXORI" }, 
	{ CLTI,  "CLTI" }, 
	{ MOV,  "MOV" }, 
	{ INV,  "INV" }, 
	{ XCG,  "XCG" }, 
	{ STRB,  "STRB" }, 
	{ STRW,  "STRW" }, 
	{ STRS,  "STRS" }, 
	{ LDRB,  "LDRB" }, 
	{ LDRW,  "LDRW" }, 
	{ LDRS,  "LDRS" }, 
	{ PSHB,  "PSHB" }, 
	{ PSHW,  "PSHW" }, 
	{ PSHS,  "PSHS" }, 
	{ POPB,  "POPB" }, 
	{ POPW,  "POPW" }, 
	{ POPS,  "POPS" }, 
	{ SWVB,  "SWVB" }, 
	{ SWVW,  "SWVW" }, 
	{ SWVS,  "SWVS" }, 
	{ STRBI,  "STRBI" }, 
	{ STRWI,  "STRWI" }, 
	{ STRSI,  "STRSI" }, 
	{ LDRBI,  "LDRBI" }, 
	{ LDRWI,  "LDRWI" }, 
	{ LDRSI,  "LDRSI" }, 
	{ MVLA,  "MVLA" }, 
	{ MVLB,  "MVLB" }, 
	{ MVLC,  "MVLC" }, 
	{ MVLD,  "MVLD" }, 
	{ MVMA,  "MVMA" }, 
	{ MVMB,  "MVMB" }, 
	{ MVMC,  "MVMC" }, 
	{ MVMD,  "MVMD" }, 
	{ MVHA,  "MVHA" }, 
	{ MVHB,  "MVHB" }, 
	{ MVHC,  "MVHC" }, 
	{ MVHD,  "MVHD" }, 
	{ MVLE,  "MVLE" }, 
	{ MVLF,  "MVLF" }, 
	{ MVLG,  "MVLG" }, 
	{ MVLH,  "MVLH" }, 
	{ MVME,  "MVME" }, 
	{ MVMF,  "MVMF" }, 
	{ MVMG,  "MVMG" }, 
	{ MVMH,  "MVMH" }, 
	{ MVHE,  "MVHE" }, 
	{ MVHF,  "MVHF" }, 
	{ MVHG,  "MVHG" }, 
	{ MVHH,  "MVHH" }, 
	{ PSHI,  "PSHI" }, 
	{ END,  "END" }, 
	{ REV,  "REV" }, 
	{ POPC,  "POPC" }, 
	{ PARI,  "PARI" }, 
	{ LEAD,  "LEAD" }, 
	{ TAIL,  "TAIL" }, 
	{ LSHL,  "LSHL" }, 
	{ LSHR,  "LSHR" }, 
	{ ROTL,  "ROTL" }, 
	{ ROTR,  "ROTR" }, 
	{ ZRO,  "ZRO" }, 
	{ FIL,  "FIL" }, 
	{ SEED,  "SEED" }, 
	{ RND,  "RND" }, 
	{ RSP,  "RSP" }, 
	{ WSP,  "WSP" }, 
	{ RIP,  "RIP" }, 
	{ WIP,  "WIP" }, 
	{ RPS,  "RPS" }, 
	{ WPS,  "WPS" }, 
	{ RXS,  "RXS" }, 
	{ WXS,  "WXS" }, 
	{ TPAGE,  "TPAGE" }, 
	{ WPAGE,  "WPAGE" }, 
	{ MEMMAP,  "MEMMAP" }, 
	{ SWAP,  "SWAP" }, 
	{ SWPR,  "SWPR" }, 
	{ SHDW,  "SHDW" }, 
	{ LITE,  "LITE" }, 
	{ SET,  "SET" }, 
	{ CLR,  "CLR" }, 
	{ CBIT,  "CBIT" }, 
	{ TGL,  "TGL" }, 
	{ FADD,  "FADD" }, 
	{ FSUB,  "FSUB" }, 
	{ FMUL,  "FMUL" }, 
	{ FSQT,  "FSQT" }, 
	{ FDIV,  "FDIV" }, 
	{ FLOG,  "FLOG" }, 
	{ FCNV,  "FCNV" }, 
	{ FCST,  "FCST" }, 
	{ FSIN,  "FSIN" }, 
	{ FCOS,  "FCOS" }, 
	{ FTAN,  "FTAN" }, 
	{ FASIN,  "FASIN" }, 
	{ FEXP,  "FEXP" }, 
	{ FLN,  "FLN" }, 
	{ FREC,  "FREC" }, 
	{ FABS,  "FABS" }, 
	{ FNEG,  "FNEG" }, 
	{ FCGT,  "FCGT" }, 
	{ FCLT,  "FCLT" }, 
	{ FMOD,  "FMOD" }, 
	{ CINF,  "CINF" }, 
	{ CNAN,  "CNAN" }, 
	{ CNRM,  "CNRM" }, 
	{ CALW,  "CALW" }, 
	{ CNEV,  "CNEV" }, 
	{ FCPI,  "FCPI" }, 
	{ FCE,  "FCE" }, 
	{ FC0,  "FC0" }, 
	{ FC1,  "FC1" }, 
	{ FC2,  "FC2" }, 
	{ FCSQ2,  "FCSQ2" }, 
	{ FCPHI,  "FCPHI" }, 
	{ FCTAU,  "FCTAU" }, 
	{ RET ,   "RET"}, 
 	{ JMA ,   "JMA"}, 
 	{ JLA ,   "JLA"}, 
 	{ JMO ,   "JMO"}, 
 	{ JLO ,   "JLO"}, 
 	{ JMR ,   "JMR"}, 
 	{ JLR ,   "JLR"}, 
 	{ JMD ,   "JMD"}, 
 	{ JLD ,   "JLD"}, 
 	{ JMPD,  "JMPD" }, 
	{ JLPD,  "JLPD" }, 
	{ JMSD,  "JMSD" }, 
	{ JLSD,  "JLSD" }, 
	{ JMRO ,   "JMRO"}, 
 	{ JLRO ,   "JLRO"}, 
 	{ JMZO ,   "JMZO"}, 
 	{ JLZO ,   "JLZO"}, 
 	{ JMNZO,  "JMNZO" }, 
	{ JLNZO,  "JLNZO" }, 
	{ JMRD ,  "JMRD"}, 
 	{ JLRD ,  "JLRD"}, 
 	{ JMZD,  "JMZD" }, 
	{ JLZD,  "JLZD" }, 
	{ JMNZD ,  "JMNZD"}, 
 	{ JLNZD ,  "JLNZD"}, 
 	{ JMRPD ,  "JMRPD"}, 
 	{ JLRPD ,  "JLRPD"}, 
 	{ JMRSD ,  "JMRSD"}, 
 	{ JLRSD ,  "JLRSD"}, 
 	{ JMZPD,  "JMZPD" }, 
	{ JLZPD,  "JLZPD" }, 
	{ JMZSD,  "JMZSD" }, 
	{ JLZSD,  "JLZSD" }, 
	{ JMNZPD,  "JMNZPD" }, 
	{ JLNZPD,  "JLNZPD" }, 
	{ JMNZSD,  "JMNZSD" }, 
	{ JLNZSD,  "JLNZSD" }, 
	{ DBG ,  "DBG"}, 
 	{ DBGB,  "DBGB" }, 
	{ DBGW,  "DBGW" }, 
	{ DBGS,  "DBGS" }, 
	{ DBGF,  "DBGF" }, 
	{ DBGA,  "DBGA" }, 
	{ SYSCI,  "SYSCI" }, 
	{ TRAPI,  "TRAPI" }, 
	{ KILL,  "KILL" }, 
	{ RESET,  "RESET" }, 
	{ RESERVED,  "RESERVED" }, 
	{ ERR,  "ERR"}, 
 };
