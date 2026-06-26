#include <bit>

#include "types.hpp"
#include "op_tables.hpp"

#define PRED_MASK 0b1000000000000000
#define TYPE_MASK 0b0110000000000000
#define OPCA_MASK 0b0001111000000000
#define OPCB_MASK 0b0000000111000000
#define DEST_MASK 0b0000000000111000
#define SRCR_MASK 0b0000000000000111
#define SOLO_MASK 0b0000000000111111 /* for solo regs */
#define JMPR_MASK 0b0000000111000000 /* same as OPCB  */
#define IMM6_MASK 0b0000000000111111
#define IMM8_MASK 0b0000000011111111
#define SPEC_MASK 0b0000000100000000

enum ITYPE : u8
{
	REG_REG_A = 0b00,
	REG_IMM_A = 0b01,
	REG_IMM_B = 0b10,
	IMM8_INSN = 0b11
};

INSN DECODE_PREFIXED(u16 raw, u8 prefix);

INSN CPU::DECODE(u16 raw)
{

	INSN result;

	//printf("decoding 0x%04x w/ prefix 0x%02x\n", raw, PREFIX);
	if(PREFIX) { 
		result = DECODE_PREFIXED(raw, PREFIX);
		if((result.OPERATION == JMFAR) or (result.OPERATION == JLFAR) and PREFIX == 0x7f)
			PREFIX = 0x00;
		//printf("prefixed decoded: %s\n",
		//	unmappings[result.OPERATION].c_str());
		return result;
	}

	if(raw & 0x8000)
		result.PREDICATED = true;
	raw &= 0x7fff; // mask out the predication bit

	u8  op_part_a;
	u8  op_part_b;
	u8  extra_reg;
	u8  source_reg;
	u8  destin_reg;
	i16 immediate = 0;

	u8 insn_type = (raw & TYPE_MASK) >> 13;

	switch(insn_type)
	{
		case IMM8_INSN:
		{
			bool spec = (raw & SPEC_MASK);
			op_part_a  = (raw & OPCA_MASK) >> 9;
			immediate  = 
				std::bit_cast<i8>(u8(raw & IMM8_MASK));

			if(!spec)
				result.OPERATION = IMM8_A[op_part_a];
			else
				result.OPERATION = IMM8_B[op_part_a];

			result.IMMEDIATE   = std::bit_cast<i8>(u8(immediate));

		}
		break;

		case REG_REG_A:
		{
			op_part_a  = (raw >> 9) & 0b1111; // (raw & OPCA_MASK) >> 9;
			op_part_b  = (raw >> 6) & 0b0111; // (raw & OPCB_MASK) >> 6;
			destin_reg = (raw >> 3) & 0b0111; // (raw & DEST_MASK) >> 3;
			source_reg = (raw >> 0) & 0b0111; // (raw & SRCR_MASK) >> 0;
			
			result.FIRST_REG   =  destin_reg;
			result.SECOND_REG  =  source_reg;
			result.OPERATION   =  REG_REG_TABLE
							    	[op_part_a]
							    	[op_part_b];

		}
		break;

		case REG_IMM_A:
		case REG_IMM_B:
		{
			op_part_a  = (raw >> 9) & 0b1111;
			destin_reg = (raw >> 6) & 0b0111;
			op_part_b  = (raw & SOLO_MASK) >> 0;
			immediate  = (raw & IMM6_MASK) >> 0;

			result.IMMEDIATE = immediate;

			result.FIRST_REG = destin_reg;

			switch(op_part_a)
			{
				case 0b0110:
					if(insn_type == REG_IMM_A)
						result.OPERATION = REG_SOLO_TBLA[op_part_b];
					else
						result.OPERATION = REG_IMM_TBLB[op_part_a];
						//result.OPERATION = NOP; //REG_SOLO_TBLB[op_part_b];
					break;
				case 0b0111:
					if(insn_type == REG_IMM_A)
						result.OPERATION = NO_REGISTERS[op_part_b];
					else
						result.OPERATION = DEBUGS[op_part_b];
					break;
				default:
					if(insn_type == REG_IMM_A)
						result.OPERATION = REG_IMM_TBLA[op_part_a];
					else
						result.OPERATION = REG_IMM_TBLB[op_part_a];
					break;
			}
		}
		default: break;
	}
	return result;
}

//#define PRED_MASK 0b1000000000000000

#define P_PREF_MASK 0b0100000000000000
#define P_TYPE_MASK 0b0011111000000000
#define P_OPCA_MASK 0b0000000111000000
#define P_OPCB_MASK 0b0000000000110000
#define P_FP48_REGA 0b0000000000001100
#define P_FP48_REGB 0b0000000000000011

#define P_FP24_REGA 0b0000000000111000
#define P_FP24_REGB 0b0000000000000111

INSN DECODE_PREFIXED(u16 raw, u8 prefix)
{
	INSN result;

	if(prefix == 0x7f)
	{
		if(raw & 0x8000) result.PREDICATED = true;
		if(raw & 0x4000) result.OPERATION = JLFAR;
		else
			result.OPERATION = JMFAR;
		i16 off = raw & 0x1fff;
		if(raw & 0x2000) { off = -off; }
		result.IMMEDIATE = off;

		// std::printf("!! [J%cFAR] <--- 0x%04x !! \n", (result.OPERATION==JMFAR)?'M':'L', raw);

		return result;
	}

	if(raw & 0x8000)
		result.PREDICATED = true;
	if(raw & 0x4000)
		result.HOLD_PREFIX = true;
	// mask out the top two flag bits

	raw &= 0x3fff;

	// std::printf("The hold-prefix is: %d\n", result.HOLD_PREFIX);

	u16 TYPEMASK = (raw & P_TYPE_MASK) >> 9;
	u16 OPCA = (raw & P_OPCA_MASK) >> 6;
	u16 OPCB = (raw & P_OPCB_MASK) >> 4;

	u16 FP48REGA = (raw & P_FP48_REGA) >> 2; // these will shift up
	u16 FP48REGB = (raw & P_FP48_REGB) >> 0;

	u16 FP24REGA = (raw & P_FP24_REGA) >> 3;
	u16 FP24REGB = (raw & P_FP24_REGB) >> 0;

	switch(TYPEMASK)
	{
		case 0b10000:
			result.OPERATION  = FP48_TYPE[(OPCA << 2) | OPCB];
			result.FIRST_REG  = FP48REGA << 1;
			result.SECOND_REG = FP48REGB << 1;
			if(result.OPERATION == FMADD_48)
				 result.THIRD_REG = OPCB << 1; 
			break;

		case 0b10001:
			result.OPERATION  = PRE_1_FP24_OPS_A[OPCA];
			result.FIRST_REG  = FP24REGA;
			result.SECOND_REG = FP24REGB;
			break;

		case 0b10010:
			result.OPERATION  = PRE_1_FP24_OPS_B[OPCA];
			result.FIRST_REG  = FP24REGA;
			result.SECOND_REG = FP24REGB;
			break;
		case 0b10011:
			result.OPERATION  = PRE_1_MEM_OPS_C[OPCA];
			result.FIRST_REG  = FP24REGA;
			result.SECOND_REG = FP24REGB;
			break;
		case 0b10100:
			result.OPERATION  = PRE_1_MEM_OPS_D[OPCA];
			result.FIRST_REG  = FP24REGA;
			result.SECOND_REG = FP24REGB;
			break;

		default:
			return {};
	}
	// std::printf("insn raw: 0x%04x : %x | %x = %x \n", raw, OPCA, OPCB, (OPCA << 2) | OPCB);
	// std::printf("decoded: [%d]\tfmadd_48: [%d] / fmov_48: [%d]\n", result.OPERATION, FMADD_48, FMOV_48);

	return result;
}
