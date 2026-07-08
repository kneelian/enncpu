#include "utility.hpp"
#include <bit>

void ASSEMBLE(
	std::vector<LINE>& lines,
	u32& current_address,
	std::vector<u8>& out_asm)
{
	#define PRED_MASK 0x8000
	u16 insn = 0;
	for(size_t j = 1; j < lines.size(); j++)
	{
		auto& i = lines.at(j);
		insn = 0;
		
		//std::printf("assembling [%s %d %d %d]\n", unmappings[i.OPERATION].c_str(), i.REG_A, i.REG_B, i.REG_C);

		if(i.OPERATION == DIRECTIVE and i.DIREC != NONE)
		{

			u32 temp = 0;
			switch(i.DIREC)
			{
				case SECTION:

					out_asm.push_back(0x64);
					out_asm.push_back(0x04);
					out_asm.push_back(i.POSITION + 8  >> 16);
					out_asm.push_back(i.POSITION + 8  >>  8);
					out_asm.push_back(i.POSITION + 8  >>  0);
					out_asm.push_back(i.LABEL_OFF - 8 >> 16);
					out_asm.push_back(i.LABEL_OFF - 8 >>  8);
					out_asm.push_back(i.LABEL_OFF - 8 >>  0);
					break;
				case ASCIZ:
				case ASCII:
					{
						std::string* str = std::bit_cast<std::string*>(i.RAW_DATA);
						for(size_t j = 0; j < str->size(); j++)
							out_asm.push_back(str->at(j));
						delete str;
						break;
					}
				case FP48:
				case INT48:
					out_asm.push_back(i.RAW_DATA >> 40);
					out_asm.push_back(i.RAW_DATA >> 32);
					out_asm.push_back(i.RAW_DATA >> 24);
					out_asm.push_back(i.RAW_DATA >> 16);
					out_asm.push_back(i.RAW_DATA >> 8);
					out_asm.push_back(i.RAW_DATA);
					break;

				case FP24:
				case INT24:
					out_asm.push_back(i.RAW_DATA >> 16);
					out_asm.push_back(i.RAW_DATA >> 8);
					out_asm.push_back(i.RAW_DATA);
					break;

				case REP:
					temp = i.RAW_DATA >> 32;
					for(u32 j = 0; j < temp; j++)
					{
						out_asm.push_back(i.RAW_DATA & 0xff);
					}
					break;
					
				case INT16:
					out_asm.push_back(i.RAW_DATA >> 8);
					out_asm.push_back(i.RAW_DATA);
					break;
				case NUL:
				case INT8:
					out_asm.push_back(i.RAW_DATA);
					break;
				default:
				case ORG:
					break;
			}
			continue;
		}

		else if(i.OPERATION == NOP)
		{
			insn = 0b0'00'1001'111'000000;
			goto skip;
		}

		if(i.OPERATION != NOP)
		{
			/* prefixals go first */
			if(i.STATE == 0x7f) // far jumps
			{
				switch(i.OPERATION)
				{
					case JMFAR: insn |= 0x0000; break;
					case JLFAR: insn |= 0x4000; break;
					default: throw; // why did anything else get parsed
				}

				bool negative = false;
				i16 off = i.IMMEDIATE;
				if(off < 0) { negative = true; off = -off; }
				if(negative) insn |= 0b0'0'1'00000'00000000;
				insn |= std::bit_cast<u16>(off) & 0b0'0011111'11111111;
				goto skip;
			}
			
			else if(i.STATE == 0x01)
			{
				if(i.OPERATION == FMADD_48)
				{	
					u8 r1 = i.REG_A >> 1;
					u8 r2 = i.REG_B >> 1;
					u8 r3 = i.REG_C >> 1;
					insn |= (0b00'10000'000'00'00'00) | (r3 << 4) | (r1 << 2) | (r2 << 0); // '
					goto skip;
				} else

				switch(i.OPERATION)
				{
					case FADD_48:
					case FSUB_48:
					case FMUL_48:
					case FDIV_48:
					case FMOV_48:
					case FCGT_48:
					case FCLT_48:
					case FABS_48:
					case FNEG_48:
					case FREC_48:
					case FSQT_48:
					{
						u8 r1 = i.REG_A >> 1;
						u8 r2 = i.REG_B >> 1;
						insn |= 0b00'10000'000'00'00'00 | (r1 << 2) | (r2 << 0); // '
						break;
					}

					case ST2S:  
					case LD2S:  
					case ST2SBD:
					case LD2SBD:
					case ST2SBI:
					case LD2SBI:
					case ST2SAD:
					case LD2SAD:
					case ST2SAI:
					case LD2SAI:
					{
						insn |= 0b00'10000'000'000'000 | (i.REG_A << 3) | (i.REG_B << 0);
						break;
					}
					default: goto skip;
				}

				switch(i.OPERATION)
				{
					case FADD_48:
						insn |= 0b00'10000'001'00'00'00; break;
					case FSUB_48:
						insn |= 0b00'10000'001'01'00'00; break;
					case FMUL_48:
						insn |= 0b00'10000'001'10'00'00; break;
					case FDIV_48:
						insn |= 0b00'10000'001'11'00'00; break;

					case FABS_48:
						insn |= 0b00'10000'010'00'00'00; break;
					case FNEG_48:
						insn |= 0b00'10000'010'01'00'00; break;
					case FREC_48:
						insn |= 0b00'10000'010'10'00'00; break;
					case FSQT_48:
						insn |= 0b00'10000'010'11'00'00; break;

					case FLOG_48:
						insn |= 0b00'10000'011'00'00'00; break;
					case FLN_48:
						insn |= 0b00'10000'011'01'00'00; break;
					case FSIN_48:
						insn |= 0b00'10000'011'10'00'00; break;
					case FCOS_48:
						insn |= 0b00'10000'011'11'00'00; break;

					case FMOV_48:
						insn |= 0b00'10000'100'00'00'00; break;
					case FCGT_48:
						insn |= 0b00'10000'100'01'00'00; break;
					case FCLT_48:
						insn |= 0b00'10000'100'10'00'00; break;
					case FEXP_48:
						insn |= 0b00'10000'100'11'00'00; break;

					case ST2S:  
						insn |= 0b00'10010'010'000'000; break;
					case LD2S:  
						insn |= 0b00'10010'011'000'000; break;
					case ST2SBD:
						insn |= 0b00'10011'111'000'000; break;
					case LD2SBD:
						insn |= 0b00'10011'110'000'000; break;
					case ST2SBI:
						insn |= 0b00'10011'101'000'000; break;
					case LD2SBI:
						insn |= 0b00'10011'100'000'000; break;
					case ST2SAD:
						insn |= 0b00'10011'011'000'000; break;
					case LD2SAD:
						insn |= 0b00'10011'010'000'000; break;
					case ST2SAI:
						insn |= 0b00'10011'001'000'000; break;
					case LD2SAI:
						insn |= 0b00'10011'000'000'000; break;

					default: goto skip;
				}
				goto skip;
			}

			if(i.REG_A > -1 and i.REG_B > -1) // two regs
			{

				insn |= (i.REG_A << 3) | i.REG_B;

			} else
			if(i.REG_A > -1 and i.IMMEDIATE != -1024)
			{


				insn |= (i.REG_A << 6);
				switch(i.OPERATION)
				{
					case JMZO:
					case JLZO:
					case JMNZO:
					case JLNZO:
						if(i.IMMEDIATE < 0)
						{
							u8 temp = -(i.IMMEDIATE / 2);
							temp &= 0b000'11111;
							temp |= 0b001'00000;
							insn |= temp;
						} else
						{
							insn |= std::bit_cast<u8>(i8(i.IMMEDIATE / 2)) & 0b0001'1111;
						}
						break;
					default:
						insn |= std::bit_cast<u8>(i8(i.IMMEDIATE / 1)) & 0b0011'1111;
						break;
				}
			} else
			if(i.REG_A < 0 and i.IMMEDIATE != -1024)
			{
				switch(i.OPERATION)
				{

					case JMO:
					case JLO:
						insn |= std::bit_cast<u8>(i8(i.IMMEDIATE / 2)); break;
					default:
						insn |= std::bit_cast<u8>(i8(i.IMMEDIATE / 1)); break;
				}
			} else
			if (i.REG_A != -127 and i.IMMEDIATE == -1024) // solo
			{
				insn |= i.REG_A << 6;
			}
			switch(i.OPERATION)
			{
				case STRB:  insn |= 0b0'00'0000'001'000000; break;
				case STRW:  insn |= 0b0'00'0000'010'000000; break;
				case STRS:  insn |= 0b0'00'0000'011'000000; break;
				case END:   insn |= 0b0'00'0000'100'000000; break;
				case LDRB:  insn |= 0b0'00'0000'101'000000; break;
				case LDRW:  insn |= 0b0'00'0000'110'000000; break;
				case LDRS:  insn |= 0b0'00'0000'111'000000; break;
				case ADD:   insn |= 0b0'00'0001'000'000000; break;
				case SUB:   insn |= 0b0'00'0001'001'000000; break;
				case MULA:  insn |= 0b0'00'0001'010'000000; break;
				case MULB:  insn |= 0b0'00'0001'011'000000; break;
				case DIV:   insn |= 0b0'00'0001'100'000000; break;
				case MOD:   insn |= 0b0'00'0001'101'000000; break;
				case MOV:   insn |= 0b0'00'0001'110'000000; break;
				case INV:   insn |= 0b0'00'0001'111'000000; break;

				case CEQ:	 insn |= 0b0'00'0010'000'000000; break;
				case CNE:	 insn |= 0b0'00'0010'001'000000; break;
				case CGT:	 insn |= 0b0'00'0010'010'000000; break;
				case CLT:	 insn |= 0b0'00'0010'011'000000; break;
				case COR:	 insn |= 0b0'00'0010'100'000000; break;
				case CAND:	 insn |= 0b0'00'0010'101'000000; break;
				case CNAND:	 insn |= 0b0'00'0010'110'000000; break;
				case CNOR:	 insn |= 0b0'00'0010'111'000000; break;
				case BOR:	 insn |= 0b0'00'0011'000'000000; break;
				case BAND:	 insn |= 0b0'00'0011'001'000000; break;
				case BXOR:	 insn |= 0b0'00'0011'010'000000; break;
				case BNOR:	 insn |= 0b0'00'0011'011'000000; break;

				case POPC:	 insn |= 0b0'00'0100'000'000000; break;	
				case PARI:	 insn |= 0b0'00'0100'001'000000; break;	
				case LEAD:	 insn |= 0b0'00'0100'010'000000; break;	
				case TAIL:	 insn |= 0b0'00'0100'011'000000; break;	
				case LSHL:	 insn |= 0b0'00'0100'100'000000; break;	
				case LSHR:	 insn |= 0b0'00'0100'101'000000; break;	
				case ROTL:	 insn |= 0b0'00'0100'110'000000; break;	
				case ROTR:	 insn |= 0b0'00'0100'111'000000; break;	
	
				case FADD:	 insn |= 0b0'00'0101'000'000000; break;	
				case FSUB:	 insn |= 0b0'00'0101'001'000000; break;	
				case FMUL:	 insn |= 0b0'00'0101'010'000000; break;	
				case FSQT:	 insn |= 0b0'00'0101'011'000000; break;	
				case FDIV:	 insn |= 0b0'00'0101'100'000000; break;	
				case FLOG:	 insn |= 0b0'00'0101'101'000000; break;	
				case FCNV:	 insn |= 0b0'00'0101'110'000000; break;	
				case FCST:	 insn |= 0b0'00'0101'111'000000; break;	
	
				case FSIN:	 insn |= 0b0'00'0110'000'000000; break;	
				case FCOS:	 insn |= 0b0'00'0110'001'000000; break;	
				case FTAN:	 insn |= 0b0'00'0110'010'000000; break;	
				case FASIN:	 insn |= 0b0'00'0110'011'000000; break;	
				case FEXP:	 insn |= 0b0'00'0110'100'000000; break;	
				case FLN:	 insn |= 0b0'00'0110'101'000000; break;	
				case FREC:	 insn |= 0b0'00'0110'110'000000; break;	
				case FABS:	 insn |= 0b0'00'0110'111'000000; break;	
	
				case FNEG:	 insn |= 0b0'00'0111'000'000000; break;	
				case FCGT:	 insn |= 0b0'00'0111'001'000000; break;	
				case FCLT:	 insn |= 0b0'00'0111'010'000000; break;	
				case FMOD:	 insn |= 0b0'00'0111'011'000000; break;	
	
				case REV:	 insn |= 0b0'00'0111'100'000000; break;	
				case MEMMAP: insn |= 0b0'00'0111'101'000000; break; 	
				case XCG:	 insn |= 0b0'00'0111'110'000000; break;	
	
				case JMZR:	 insn |= 0b0'00'1000'000'000000; break;	
				case JLZR:	 insn |= 0b0'00'1000'001'000000; break;	
				case JMNZR:	 insn |= 0b0'00'1000'010'000000; break;	
				case JLNZR:	 insn |= 0b0'00'1000'011'000000; break;

				case CEQ_OR: insn |= 0b0'00'1000'100'000000; break;	
				case CNE_OR: insn |= 0b0'00'1000'101'000000; break;	
				case CGT_OR: insn |= 0b0'00'1000'110'000000; break;	
				case CLT_OR: insn |= 0b0'00'1000'111'000000; break;	
	
				case CGTS:	 insn |= 0b0'00'1001'000'000000; break;	
				case CLTS:	 insn |= 0b0'00'1001'001'000000; break;	

				case GETL: 
					insn |= 0b0'00'1001'100'000000; break;
				case GETM: 
					insn |= 0b0'00'1001'101'000000; break;
				case GETH: 
					insn |= 0b0'00'1001'110'000000; break;

				case SYSCI:
					insn |= 0b0'11'1100'1'00000000; break;

				case ST2S:
					insn |= 0b0'00'1101'000'000000; break;
				case ST2SBD: 
					insn |= 0b0'00'1100'000'000000; break;
				case ST2SAI: 
					insn |= 0b0'00'1011'000'000000; break;
				case LD2S:
					insn |= 0b0'00'1101'100'000000; break;
				case LD2SBD:
					insn |= 0b0'00'1100'100'000000; break;
				case LD2SAI:
					insn |= 0b0'00'1011'100'000000; break;

				case STRBBI: insn |= 0b0'00'1010'001'000000; break;
				case STRWBI: insn |= 0b0'00'1010'010'000000; break;
				case STRSBI: insn |= 0b0'00'1010'011'000000; break;
				case LDRBBI: insn |= 0b0'00'1010'101'000000; break;
				case LDRWBI: insn |= 0b0'00'1010'110'000000; break;
				case LDRSBI: insn |= 0b0'00'1010'111'000000; break;

				case STRBAI: insn |= 0b0'00'1011'001'000000; break;
				case STRWAI: insn |= 0b0'00'1011'010'000000; break;
				case STRSAI: insn |= 0b0'00'1011'011'000000; break;
				case LDRBAI: insn |= 0b0'00'1011'101'000000; break;
				case LDRWAI: insn |= 0b0'00'1011'110'000000; break;
				case LDRSAI: insn |= 0b0'00'1011'111'000000; break;

				case STRBBD: insn |= 0b0'00'1100'001'000000; break;
				case STRWBD: insn |= 0b0'00'1100'010'000000; break;
				case STRSBD: insn |= 0b0'00'1100'011'000000; break;
				case LDRBBD: insn |= 0b0'00'1100'101'000000; break;
				case LDRWBD: insn |= 0b0'00'1100'110'000000; break;
				case LDRSBD: insn |= 0b0'00'1100'111'000000; break;

				case STRBAD: insn |= 0b0'00'1101'001'000000; break;
				case STRWAD: insn |= 0b0'00'1101'010'000000; break;
				case STRSAD: insn |= 0b0'00'1101'011'000000; break;
				case LDRBAD: insn |= 0b0'00'1101'101'000000; break;
				case LDRWAD: insn |= 0b0'00'1101'110'000000; break;
				case LDRSAD: insn |= 0b0'00'1101'111'000000; break;

				case JMA:	 insn |= 0b0'11'0000'0'00000000; break;
				case JLA:	 insn |= 0b0'11'0001'0'00000000; break;
				case JMO:	 insn |= 0b0'11'0010'0'00000000; break;
				case JLO:	 insn |= 0b0'11'0011'0'00000000; break;

				case MVLA:	 insn |= 0b0'11'0100'0'00000000; break;
				case MVLB:	 insn |= 0b0'11'0101'0'00000000; break;
				case MVLC:	 insn |= 0b0'11'0110'0'00000000; break;
				case MVLD:	 insn |= 0b0'11'0111'0'00000000; break;
				case MVMA:	 insn |= 0b0'11'1000'0'00000000; break;
				case MVMB:	 insn |= 0b0'11'1001'0'00000000; break;
				case MVMC:	 insn |= 0b0'11'1010'0'00000000; break;
				case MVMD:	 insn |= 0b0'11'1011'0'00000000; break;
				case MVHA:	 insn |= 0b0'11'1100'0'00000000; break;
				case MVHB:	 insn |= 0b0'11'1101'0'00000000; break;
				case MVHC:	 insn |= 0b0'11'1110'0'00000000; break;
				case MVHD:	 insn |= 0b0'11'1111'0'00000000; break;

				case MVLE:	 insn |= 0b0'11'0000'1'00000000; break;
				case MVLF:	 insn |= 0b0'11'0001'1'00000000; break;
				case MVLG:	 insn |= 0b0'11'0010'1'00000000; break;
				case MVLH:	 insn |= 0b0'11'0011'1'00000000; break;

				case MVME:	 insn |= 0b0'11'0100'1'00000000; break;
				case MVMF:	 insn |= 0b0'11'0101'1'00000000; break;
				case MVMG:	 insn |= 0b0'11'0110'1'00000000; break;
				case MVMH:	 insn |= 0b0'11'0111'1'00000000; break;

				case MVHE:	 insn |= 0b0'11'1000'1'00000000; break;
				case MVHF:	 insn |= 0b0'11'1001'1'00000000; break;
				case MVHG:	 insn |= 0b0'11'1010'1'00000000; break;
				case MVHH:	 insn |= 0b0'11'1011'1'00000000; break;

				case DBGB:   insn |= 0b0'10'0111'000'000000; break;
				case DBGW:   insn |= 0b0'10'0111'000'000001; break;
				case DBGS:   insn |= 0b0'10'0111'000'000010; break;
				case DBGA:   insn |= 0b0'10'0111'000'000011; break;
				case DBGF:   insn |= 0b0'10'0111'000'000100; break;
				case DBGC:   insn |= 0b0'10'0111'000'000101; break;

				case DBGINC: insn |= 0b0'10'0111'000'001101; break;
				
				case ADDI:	insn |= 0b0'01'0000'000'000000; break;
				case SUBI:	insn |= 0b0'01'0001'000'000000; break;
				case ADDHI:	insn |= 0b0'01'0011'000'000000; break;
				case SUBHI:	insn |= 0b0'01'1100'000'000000; break;
				case MOVI:  insn |= 0b0'01'0010'000'000000; break;
				case LSHLI:	insn |= 0b0'01'0100'000'000000; break;
				case LSHRI:	insn |= 0b0'01'0101'000'000000; break;
				case SETI:	insn |= 0b0'01'1000'000'000000; break;
				case CLRI:	insn |= 0b0'01'1001'000'000000; break;
				case CBITI:	insn |= 0b0'01'1010'000'000000; break;
				case TGLI:	insn |= 0b0'01'1011'000'000000; break;
				case CEQI_OR:	insn |= 0b0'01'1101'000'000000; break;
				case CNEI_OR:	insn |= 0b0'01'1110'000'000000; break;
				// case STRSI:	insn |= 0b0'01'1111'000'000000; break;

				case CEQI:	insn |= 0b0'10'0000'000'000000; break;
				case CNEI:	insn |= 0b0'10'0001'000'000000; break;
				case CGTI:	insn |= 0b0'10'0010'000'000000; break;
				case CLTI:	insn |= 0b0'10'0011'000'000000; break;
				case CORI:	insn |= 0b0'10'0100'000'000000; break;
				case CANDI:	insn |= 0b0'10'0101'000'000000; break;
				case BORI:	insn |= 0b0'10'1100'000'000000; break;
				case BANDI:	insn |= 0b0'10'1111'000'000000; break;
				case JMZO:	insn |= 0b0'10'1000'000'000000; break;
				case JLZO:	insn |= 0b0'10'1001'000'000000; break;
				case JMNZO:	insn |= 0b0'10'1010'000'000000; break;
				case JLNZO:	insn |= 0b0'10'1011'000'000000; break;
				case CLTI_OR:	insn |= 0b0'10'1100'000'000000; break;
				case CGTI_OR:	insn |= 0b0'10'1101'000'000000; break;
				// case LDRSI:	insn |= 0b0'10'1110'000'000000; break;

				case PSHI:  insn |= 0b0'11'1110'1'00000000; break;
				case JUNKB: insn |= 0b0'01'0111'000'000101; break;
				case JUNKW: insn |= 0b0'01'0111'000'000110; break;
				case JUNKS: insn |= 0b0'01'0111'000'000111; break;

				case KERNI: insn |= 0b0'11'1101'1'00000000; break;

				case JMR:  insn |= 0b0'01'0110'000'011000; break;
				case JLR:  insn |= 0b0'01'0110'000'011001; break;
				case JMPD: insn |= 0b0'01'0110'000'011010; break;
				case JMSD: insn |= 0b0'01'0110'000'011011; break;
				case JLPD: insn |= 0b0'01'0110'000'011100; break;
				case JLSD: insn |= 0b0'01'0110'000'011101; break;

				case FIL:  insn |= 0b0'01'0110'000'001100; break;
				case SWPR: insn |= 0b0'01'0110'000'001101; break;
				case SHDW: insn |= 0b0'01'0110'000'001110; break;
				case LITE: insn |= 0b0'01'0110'000'001111; break;
				case RSP:  insn |= 0b0'01'0110'000'010000; break;
				case WSP:  insn |= 0b0'01'0110'000'010001; break;
				case RIP:  insn |= 0b0'01'0110'000'010010; break;
				case WIP:  insn |= 0b0'01'0110'000'010011; break;
				case RPS:  insn |= 0b0'01'0110'000'010100; break;
				case WPS:  insn |= 0b0'01'0110'000'010101; break;
				case RXS:  insn |= 0b0'01'0110'000'010110; break;
				case WXS:  insn |= 0b0'01'0110'000'010111; break;
				case RXV:  insn |= 0b0'01'0110'000'011010; break;
				case WXV:  insn |= 0b0'01'0110'000'011011; break;

				case SEED: insn |= 0b0'01'0110'000'011110; break;
				case RND:  insn |= 0b0'01'0110'000'001010; break;

				case FCPI:	insn |= 0b0'01'0110'000'100000; break;
				case FCE:	insn |= 0b0'01'0110'000'100001; break;
				case FC0:	insn |= 0b0'01'0110'000'100010; break;
				case FC1:	insn |= 0b0'01'0110'000'100011; break;
				case FC2:	insn |= 0b0'01'0110'000'100100; break;
				case FCSQ2:	insn |= 0b0'01'0110'000'100101; break;
				case FCPHI:	insn |= 0b0'01'0110'000'100110; break;
				case FCTAU:	insn |= 0b0'01'0110'000'100111; break;
				case CINF:  insn |= 0b0'01'0110'000'101000; break;
				case CNAN:  insn |= 0b0'01'0110'000'101001; break;
				case CNRM:  insn |= 0b0'01'0110'000'101010; break;

				case MMU_SETRO:  insn |= 0b0'01'0110'000'111000; break;
				case MMU_CHKRO:  insn |= 0b0'01'0110'000'111001; break;
				case MMU_SETUSR: insn |= 0b0'01'0110'000'111010; break;
				case MMU_CHKUSR: insn |= 0b0'01'0110'000'111011; break;
				case PEEKB:  insn |= 0b0'01'0110'000'111100; break;
				case PEEKW:  insn |= 0b0'01'0110'000'111101; break;
				case PEEKS:  insn |= 0b0'01'0110'000'111110; break;

				case PSH2W:  insn |= 0b0'01'0110'000'110000; break;
				case PSH2S:  insn |= 0b0'01'0110'000'110001; break;
				case POP2W:  insn |= 0b0'01'0110'000'110100; break;
				case POP2S:  insn |= 0b0'01'0110'000'110101; break;

				case CALW: insn |= 0b0'01'0111'000'001100; break;
				case CNEV: insn |= 0b0'01'0111'000'001101; break;
				case CINV: insn |= 0b0'01'0111'000'001110; break;
				case WFI:  insn |= 0b0'01'0111'000'001111; break;

				case MASK:   insn |= 0b0'01'0111'000'010000; break;
				case UNMASK: insn |= 0b0'01'0111'000'010001; break;
				case CXS:    insn |= 0b0'01'0111'000'010010; break;

				case RET:  insn |= 0b0'01'0111'000'001000; break;
				case ERET: insn |= 0b0'01'0111'000'001001; break;
				case SWVB: insn |= 0b0'01'0111'000'000000; break;
				case SWVW: insn |= 0b0'01'0111'000'000001; break;
				case SWVS: insn |= 0b0'01'0111'000'000010; break;
				case SWAP: insn |= 0b0'01'0111'000'000100; break;

				case PSHB: insn |= 0b0'01'0110'000'000000; break;
				case PSHW: insn |= 0b0'01'0110'000'000001; break;
				case PSHS: insn |= 0b0'01'0110'000'000010; break;
				case POPB: insn |= 0b0'01'0110'000'000100; break;
				case POPW: insn |= 0b0'01'0110'000'000101; break;
				case POPS: insn |= 0b0'01'0110'000'000110; break;

				case PADD:  insn |= 0b0'00'1110'010'000000; break;
				case PSUB:  insn |= 0b0'00'1110'110'000000; break;
				case PMUL:  insn |= 0b0'00'1111'010'000000; break;
				case PDIV:  insn |= 0b0'00'1111'110'000000; break;
				case PBAND: insn |= 0b0'00'1110'011'000000; break;
				case PBOR:  insn |= 0b0'00'1110'111'000000; break;
				case PBXOR: insn |= 0b0'00'1111'011'000000; break;
				case PCPY:  insn |= 0b0'00'1111'111'000000; break;

				insn |= 0b0'00'1100'010'000000;

				case PREF: insn |= 0b0'11'1111'1'00000000; break;

				default: break;
			}
		}

		skip:
		if((i.OPERATION != DIRECTIVE) and (i.OPERATION != LABEL))
		{
			if(i.PREDICATED) { insn |= PRED_MASK; }
			//std::printf("WE ARE NOW PUSHING 0x%04x\n", insn);
			out_asm.push_back(insn >> 8);
			out_asm.push_back(insn);
		}
		/*std::printf("0x%06x: [0x%04x] from %s%s\t%c%s%c%s%s\n",
			  i.POSITION,
			  insn,
			  unmappings[i.OPERATION].c_str(),	// (u16)i.OPERATION,
			  i.PREDICATED?".P":"",
			  (i.REG_A != -127)?i.REG_A + 'A':'\0',
			  (i.REG_B != -127)?", ":"",
			  (i.REG_B != -127)?i.REG_B + 'B':'\0',
			  (i.IMMEDIATE != -1024 and (i.REG_A != -127 or i.REG_B != -127))?", ":"",
			  (i.IMMEDIATE != -1024)?("#" + std::to_string(i.IMMEDIATE)).c_str():""
		);*/
	}

}
