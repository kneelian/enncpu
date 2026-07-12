#include <array>
#include <algorithm>
#include <cmath>
#include <bit>

#include "types.hpp"
#include "random_module.hpp"
extern std::unordered_map<OP, std::string> unmappings;

int EXEC_JUMP(INSN insn, CPU* me);
int EXEC_SYSC(u16 SYSC, CPU* me);

int CPU::EXECUTE(const INSN insn)
{
	//std::printf("This should execute a %s (0x%06x) at [0x%06x]\n", unmappings[insn.OPERATION].c_str(), insn.IMMEDIATE, IP);
	if(insn.OPERATION == ERR) 
	{ 
		std::printf("OOPS! Hit an error @ 0x%06x\n", IP - 2);
		return -1; // will later trap
	} else

	if(insn.OPERATION >= DBGB and insn.OPERATION <= DBGA)
	{
		static const char REG_NAMES[] = "ABCDEFGH";
		static char temp_input[16];
		switch(insn.OPERATION)
		{
			case DBGB:
				printf("DEBUG: %c = 0x%02x\n",
					REG_NAMES[insn.FIRST_REG],
					(u8)ACTIVE_SET->at(insn.FIRST_REG)
				);
				break;
			case DBGW:
				printf("DEBUG: %c = 0x%04x\n",
					REG_NAMES[insn.FIRST_REG],
					(u16)ACTIVE_SET->at(insn.FIRST_REG)
				);
				break;
			case DBGS:
				printf("DEBUG: %c = 0x%06x\n",
					REG_NAMES[insn.FIRST_REG],
					(u32)ACTIVE_SET->at(insn.FIRST_REG)
				);
				break;
			case DBGA:
				printf("DEBUG: %c = %c%c%c\n",
					REG_NAMES[insn.FIRST_REG],
					char(ACTIVE_SET->at(insn.FIRST_REG) >> 16),
					char(ACTIVE_SET->at(insn.FIRST_REG) >>  8),
					char(ACTIVE_SET->at(insn.FIRST_REG) >>  0)
				);
				break;
			case DBGF:
				printf("DEBUG: %c = %f \n",
					REG_NAMES[insn.FIRST_REG],
					std::bit_cast<float>(ACTIVE_SET->at(insn.FIRST_REG) << 8)
				);
				break;
			case DBGC:
				printf("%c", char(ACTIVE_SET->at(insn.FIRST_REG) >>  0));
				break;

			case DBGINC:
				ACTIVE_SET->at(insn.FIRST_REG) = std::getchar();
				break; //std::fgets(temp_input, 16, stdin);

			default:
			break;
		}
		return 0;
	}

	if(insn.HOLD_PREFIX != true)
		CLR_PREFIX(); 

	if(insn.PREDICATED and !IS_COND_SET())
		return 0; // predication failure, continue
	
	if(insn.OPERATION & JUMP_BASE)
		return EXEC_JUMP(insn, this);

	i64 temp = 0;
	i64 swvt = 0;

	float  f32temp = 0.0;
	double f64temp = 0.0;

	if(insn.SECOND_REG >= 0)
		ACTIVE_SET->at(insn.SECOND_REG & 7) &= 0x00ffffff;

	switch(insn.OPERATION)
	{
		case ADDHI:
			ACTIVE_SET->at(insn.FIRST_REG) += (insn.IMMEDIATE << 16);
			break;
		case ADDI:
			ACTIVE_SET->at(insn.FIRST_REG) += insn.IMMEDIATE;
			break;
		case ADD:
			ACTIVE_SET->at(insn.FIRST_REG) +=
				ACTIVE_SET->at(insn.SECOND_REG);
			break;
		case ADDSAT:
			temp = 
				ACTIVE_SET->at(insn.FIRST_REG) +
				ACTIVE_SET->at(insn.SECOND_REG);
			if(temp > 0x00ffffff)
				temp = 0x00ffffff;
			ACTIVE_SET->at(insn.FIRST_REG) = temp;
			break;

		case SUBHI:
			ACTIVE_SET->at(insn.FIRST_REG) -= (insn.IMMEDIATE << 16);
			ACTIVE_SET->at(insn.FIRST_REG) &= 0x00ffffff;
			break;
		case SUBI:
			ACTIVE_SET->at(insn.FIRST_REG) -= insn.IMMEDIATE;
			break;
		case SUB:
			ACTIVE_SET->at(insn.FIRST_REG) -=
				ACTIVE_SET->at(insn.SECOND_REG);
			break;
		case SUBSAT:
			temp = 
				ACTIVE_SET->at(insn.FIRST_REG) -
				ACTIVE_SET->at(insn.SECOND_REG);
			if(temp < 0)
				temp = 0;
			ACTIVE_SET->at(insn.FIRST_REG) = temp;
			break;

		case MULA:
		case MULB:
			temp = u64(ACTIVE_SET->at(insn.FIRST_REG)) * u64(ACTIVE_SET->at(insn.SECOND_REG));
			if(insn.OPERATION == MULA)
				ACTIVE_SET->at(insn.FIRST_REG) = ((temp >> 0) & 0x00ffffff);
			else
				ACTIVE_SET->at(insn.FIRST_REG) = ((u64(temp) >>24) & 0x00ffffff);
			break;
		case MULSAT:
			temp = 
				ACTIVE_SET->at(insn.FIRST_REG) *
				ACTIVE_SET->at(insn.SECOND_REG);
			if(temp > 0x00ffffff)
				temp = 0x00ffffff;
			ACTIVE_SET->at(insn.FIRST_REG) = temp;
			break;
		case DIV:
			ACTIVE_SET->at(insn.FIRST_REG) /= 
				ACTIVE_SET->at(insn.SECOND_REG);
			break;
		case MOD:
			ACTIVE_SET->at(insn.FIRST_REG) %= 
				ACTIVE_SET->at(insn.SECOND_REG);
			break;

		case PADD:
			{
				u8 t1, t2, t3;
				u8 t4, t5, t6;
				t1 = (ACTIVE_SET->at(insn.FIRST_REG) >> 16);
				t2 = (ACTIVE_SET->at(insn.FIRST_REG) >>  8);
				t3 = (ACTIVE_SET->at(insn.FIRST_REG) >>  0);
				t4 = (ACTIVE_SET->at(insn.SECOND_REG) >>16);
				t5 = (ACTIVE_SET->at(insn.SECOND_REG) >> 8);
				t6 = (ACTIVE_SET->at(insn.SECOND_REG) >> 0);
				t1 += t4;
				t2 += t5;
				t3 += t6;
				ACTIVE_SET->at(insn.FIRST_REG) = (u32(t1) << 16) | (u32(t2) << 8) | (u32(t3) << 0);
			}
			break;
		case PSUB:
			{
				u8 t1, t2, t3;
				u8 t4, t5, t6;
				t1 = (ACTIVE_SET->at(insn.FIRST_REG) >> 16);
				t2 = (ACTIVE_SET->at(insn.FIRST_REG) >>  8);
				t3 = (ACTIVE_SET->at(insn.FIRST_REG) >>  0);
				t4 = (ACTIVE_SET->at(insn.SECOND_REG) >>16);
				t5 = (ACTIVE_SET->at(insn.SECOND_REG) >> 8);
				t6 = (ACTIVE_SET->at(insn.SECOND_REG) >> 0);
				t1 -= t4;
				t2 -= t5;
				t3 -= t6;
				ACTIVE_SET->at(insn.FIRST_REG) = (u32(t1) << 16) | (u32(t2) << 8) | (u32(t3) << 0);
			}
			break;
		case PMUL:
			{
				u8 t1, t2, t3;
				u8 t4, t5, t6;
				t1 = (ACTIVE_SET->at(insn.FIRST_REG) >> 16);
				t2 = (ACTIVE_SET->at(insn.FIRST_REG) >>  8);
				t3 = (ACTIVE_SET->at(insn.FIRST_REG) >>  0);
				t4 = (ACTIVE_SET->at(insn.SECOND_REG) >>16);
				t5 = (ACTIVE_SET->at(insn.SECOND_REG) >> 8);
				t6 = (ACTIVE_SET->at(insn.SECOND_REG) >> 0);
				t1 *= t4;
				t2 *= t5;
				t3 *= t6;
				ACTIVE_SET->at(insn.FIRST_REG) = (u32(t1) << 16) | (u32(t2) << 8) | (u32(t3) << 0);
			}
			break;
		case PDIV:
			{
				u8 t1, t2, t3;
				u8 t4, t5, t6;
				t1 = (ACTIVE_SET->at(insn.FIRST_REG) >> 16);
				t2 = (ACTIVE_SET->at(insn.FIRST_REG) >>  8);
				t3 = (ACTIVE_SET->at(insn.FIRST_REG) >>  0);
				t4 = (ACTIVE_SET->at(insn.SECOND_REG) >>16);
				t5 = (ACTIVE_SET->at(insn.SECOND_REG) >> 8);
				t6 = (ACTIVE_SET->at(insn.SECOND_REG) >> 0);
				t1 /= t4;
				t2 /= t5;
				t3 /= t6;
				ACTIVE_SET->at(insn.FIRST_REG) = (u32(t1) << 16) | (u32(t2) << 8) | (u32(t3) << 0);
			}
			break;
		case PBAND:
			{
				u8 t1, t2, t3;
				u8 t4, t5, t6;
				t1 = (ACTIVE_SET->at(insn.FIRST_REG) >> 16);
				t2 = (ACTIVE_SET->at(insn.FIRST_REG) >>  8);
				t3 = (ACTIVE_SET->at(insn.FIRST_REG) >>  0);
				t4 = (ACTIVE_SET->at(insn.SECOND_REG) >>16);
				t5 = (ACTIVE_SET->at(insn.SECOND_REG) >> 8);
				t6 = (ACTIVE_SET->at(insn.SECOND_REG) >> 0);
				t1 &= t4;
				t2 &= t5;
				t3 &= t6;
				ACTIVE_SET->at(insn.FIRST_REG) = (u32(t1) << 16) | (u32(t2) << 8) | (u32(t3) << 0);
			}
			break;
		case PBOR:
			{
				u8 t1, t2, t3;
				u8 t4, t5, t6;
				t1 = (ACTIVE_SET->at(insn.FIRST_REG) >> 16);
				t2 = (ACTIVE_SET->at(insn.FIRST_REG) >>  8);
				t3 = (ACTIVE_SET->at(insn.FIRST_REG) >>  0);
				t4 = (ACTIVE_SET->at(insn.SECOND_REG) >>16);
				t5 = (ACTIVE_SET->at(insn.SECOND_REG) >> 8);
				t6 = (ACTIVE_SET->at(insn.SECOND_REG) >> 0);
				t1 |= t4;
				t2 |= t5;
				t3 |= t6;
				ACTIVE_SET->at(insn.FIRST_REG) = (u32(t1) << 16) | (u32(t2) << 8) | (u32(t3) << 0);
			}
			break;
		case PBXOR:
			{
				u8 t1, t2, t3;
				u8 t4, t5, t6;
				t1 = (ACTIVE_SET->at(insn.FIRST_REG) >> 16);
				t2 = (ACTIVE_SET->at(insn.FIRST_REG) >>  8);
				t3 = (ACTIVE_SET->at(insn.FIRST_REG) >>  0);
				t4 = (ACTIVE_SET->at(insn.SECOND_REG) >>16);
				t5 = (ACTIVE_SET->at(insn.SECOND_REG) >> 8);
				t6 = (ACTIVE_SET->at(insn.SECOND_REG) >> 0);
				t1 ^= t4;
				t2 ^= t5;
				t3 ^= t6;
				ACTIVE_SET->at(insn.FIRST_REG) = (u32(t1) << 16) | (u32(t2) << 8) | (u32(t3) << 0);
			}
			break;
		case PCPY:
			temp = ACTIVE_SET->at(insn.SECOND_REG) & 0xff;
			ACTIVE_SET->at(insn.FIRST_REG) =
				(temp << 16) | (temp << 8) | (temp << 0);
			break;

		case MOVI:
			ACTIVE_SET->at(insn.FIRST_REG) =
			(insn.IMMEDIATE & 0xff);
			break;
		case MOV:
			ACTIVE_SET->at(insn.FIRST_REG) =
				ACTIVE_SET->at(insn.SECOND_REG);
			break;
		case INVI:
			ACTIVE_SET->at(insn.FIRST_REG) =
			(~u8(insn.IMMEDIATE)) & 0xff;
			break;
		case XCG:
			temp = ACTIVE_SET->at(insn.FIRST_REG);
			ACTIVE_SET->at(insn.FIRST_REG) = ACTIVE_SET->at(insn.SECOND_REG);
			ACTIVE_SET->at(insn.SECOND_REG) = temp;
			break;

		case GETL:
			ACTIVE_SET->at(insn.FIRST_REG) =
				(ACTIVE_SET->at(insn.SECOND_REG) & 0x0000ff) >>  0; break;
		case GETM:
			ACTIVE_SET->at(insn.FIRST_REG) =
				(ACTIVE_SET->at(insn.SECOND_REG) & 0x00ff00) >>  8; break;
		case GETH:
			ACTIVE_SET->at(insn.FIRST_REG) =
				(ACTIVE_SET->at(insn.SECOND_REG) & 0xff0000) >> 16; break;

		case INV:
			ACTIVE_SET->at(insn.FIRST_REG) =
				 (~ACTIVE_SET->at(insn.SECOND_REG)) 
				  & 0x00ffffff;
			break;

		case CEQI_OR:
			if( ACTIVE_SET->at(insn.FIRST_REG) == 
				insn.IMMEDIATE)
				SET_COND();
			break;
		case CEQI:
			if( ACTIVE_SET->at(insn.FIRST_REG) == 
				insn.IMMEDIATE)
				SET_COND();
			else
				CLR_COND();
			break;
		case CEQ_OR:
			if( ACTIVE_SET->at(insn.FIRST_REG) == 
				ACTIVE_SET->at(insn.SECOND_REG))
				SET_COND();
			break;
		case CEQ:
			if( ACTIVE_SET->at(insn.FIRST_REG) == 
				ACTIVE_SET->at(insn.SECOND_REG))
				SET_COND();
			else
				CLR_COND();
			break;
		case CNEI_OR:
			if( ACTIVE_SET->at(insn.FIRST_REG) != 
				insn.IMMEDIATE)
				SET_COND();
			break;
		case CNEI:
			if( ACTIVE_SET->at(insn.FIRST_REG) != 
				insn.IMMEDIATE)
				SET_COND();
			else
				CLR_COND();
			break;
		case CNE_OR:
			if( ACTIVE_SET->at(insn.FIRST_REG) != 
				ACTIVE_SET->at(insn.SECOND_REG))
				SET_COND();
			break;
		case CNE:
			if( ACTIVE_SET->at(insn.FIRST_REG) != 
				ACTIVE_SET->at(insn.SECOND_REG))
				SET_COND();
			else
				CLR_COND();
			break;
		case CGTI_OR:
			if( ACTIVE_SET->at(insn.FIRST_REG) >
				insn.IMMEDIATE)
				SET_COND();
			break;
		case CGTI:
			if( ACTIVE_SET->at(insn.FIRST_REG) >
				insn.IMMEDIATE)
				SET_COND();
			else
				CLR_COND();
			break;
		case CGT_OR:
			if( ACTIVE_SET->at(insn.FIRST_REG) > 
				ACTIVE_SET->at(insn.SECOND_REG))
				SET_COND();
			break;
		case CGT:
			if( ACTIVE_SET->at(insn.FIRST_REG) > 
				ACTIVE_SET->at(insn.SECOND_REG))
				SET_COND();
			else
				CLR_COND();
			break;
		case CGTS:
			if (
					(
						(ACTIVE_SET->at(insn.FIRST_REG) & 0x800000)?
						(ACTIVE_SET->at(insn.FIRST_REG) | 0xff000000):
						 ACTIVE_SET->at(insn.FIRST_REG)
					) >
					(
						(ACTIVE_SET->at(insn.SECOND_REG) & 0x800000)?
						(ACTIVE_SET->at(insn.SECOND_REG) | 0xff000000):
						 ACTIVE_SET->at(insn.SECOND_REG)
					)
				)
				SET_COND();
			else
				CLR_COND();
			break;
		case CINV:
			if(IS_COND_SET())
				CLR_COND();
			else
				SET_COND();
			break;
		case CLTS:
			if (
					(
						(ACTIVE_SET->at(insn.FIRST_REG) & 0x800000)?
						(ACTIVE_SET->at(insn.FIRST_REG) | 0xff000000):
						 ACTIVE_SET->at(insn.FIRST_REG)
					) <
					(
						(ACTIVE_SET->at(insn.SECOND_REG) & 0x800000)?
						(ACTIVE_SET->at(insn.SECOND_REG) | 0xff000000):
						 ACTIVE_SET->at(insn.SECOND_REG)
					)
				)
				SET_COND();
			else
				CLR_COND();
			break;
		case CLTI_OR:
			if( ACTIVE_SET->at(insn.FIRST_REG) < 
				insn.IMMEDIATE)
				SET_COND();
			break;
		case CLTI:
			if( ACTIVE_SET->at(insn.FIRST_REG) < 
				insn.IMMEDIATE)
				SET_COND();
			else
				CLR_COND();
			break;
		case CLT_OR:
			if( ACTIVE_SET->at(insn.FIRST_REG) <
				ACTIVE_SET->at(insn.SECOND_REG))
				SET_COND();
			break;
		case CLT:
			if( ACTIVE_SET->at(insn.FIRST_REG) <
				ACTIVE_SET->at(insn.SECOND_REG))
				SET_COND();
			else
				CLR_COND();
			break;
		case CORI:
			if( ACTIVE_SET->at(insn.FIRST_REG) or 
				insn.IMMEDIATE)
				SET_COND();
			else
				CLR_COND();
			break;
		case COR:
			if( ACTIVE_SET->at(insn.FIRST_REG) or 
				ACTIVE_SET->at(insn.SECOND_REG))
				SET_COND();
			else
				CLR_COND();
			break;
		case CANDI:
			if( ACTIVE_SET->at(insn.FIRST_REG) and 
				insn.IMMEDIATE)
				SET_COND();
			else
				CLR_COND();
			break;
		case CAND:
			if( ACTIVE_SET->at(insn.FIRST_REG) and 
				ACTIVE_SET->at(insn.SECOND_REG))
				SET_COND();
			else
				CLR_COND();
			break;
		case CNAND:
			if(!(ACTIVE_SET->at(insn.FIRST_REG) and 
				 ACTIVE_SET->at(insn.SECOND_REG)))
				SET_COND();
			else
				CLR_COND();
			break;
		case CNOR:
			if(!(ACTIVE_SET->at(insn.FIRST_REG) or 
				 ACTIVE_SET->at(insn.SECOND_REG)))
				SET_COND();
			else
				CLR_COND();
			break;
		case BORI:
			ACTIVE_SET->at(insn.FIRST_REG) |= insn.IMMEDIATE;
			break;
		case BOR:
			ACTIVE_SET->at(insn.FIRST_REG) |=
				ACTIVE_SET->at(insn.SECOND_REG);
			break;
		case BANDI:
			ACTIVE_SET->at(insn.FIRST_REG) &= insn.IMMEDIATE;
			break;
		case BAND:
			ACTIVE_SET->at(insn.FIRST_REG) &=
				ACTIVE_SET->at(insn.SECOND_REG);
			break;
		case BXOR:
			ACTIVE_SET->at(insn.FIRST_REG) ^=
				ACTIVE_SET->at(insn.SECOND_REG);
			break;
		case BNOR:
			ACTIVE_SET->at(insn.FIRST_REG) |=
				ACTIVE_SET->at(insn.SECOND_REG);
			ACTIVE_SET->at(insn.FIRST_REG) =
				~(ACTIVE_SET->at(insn.FIRST_REG));
			break; 
		case BORINV:
			ACTIVE_SET->at(insn.FIRST_REG) |=
				~(ACTIVE_SET->at(insn.SECOND_REG));
			break;
		case BANDINV:
			ACTIVE_SET->at(insn.FIRST_REG) &=
				~(ACTIVE_SET->at(insn.SECOND_REG));
			break;
		case BXORINV:
			ACTIVE_SET->at(insn.FIRST_REG) ^=
				~(ACTIVE_SET->at(insn.SECOND_REG));
			break;
		case BNORINV:
			ACTIVE_SET->at(insn.FIRST_REG) |=
				~(ACTIVE_SET->at(insn.SECOND_REG));
			ACTIVE_SET->at(insn.FIRST_REG) =
				~(ACTIVE_SET->at(insn.FIRST_REG));
			break; 
		case POPC:
			ACTIVE_SET->at(insn.FIRST_REG) = 
				std::popcount(ACTIVE_SET->at(insn.SECOND_REG));
			break;
		case PARI:
			ACTIVE_SET->at(insn.FIRST_REG) = 
				std::popcount(ACTIVE_SET->at(insn.SECOND_REG)) & 0x1;
			break;
		case LEAD:
			ACTIVE_SET->at(insn.FIRST_REG) = 
				std::countl_zero(ACTIVE_SET->at(insn.SECOND_REG)) - 8;
			break;
		case TAIL:
			if(ACTIVE_SET->at(insn.SECOND_REG) == 0)
				ACTIVE_SET->at(insn.FIRST_REG) = 24;
			else
				ACTIVE_SET->at(insn.FIRST_REG) = 
					std::countr_zero(ACTIVE_SET->at(insn.SECOND_REG));
			break;

		case LSHLI:
			if(insn.IMMEDIATE < 24)
				ACTIVE_SET->at(insn.FIRST_REG) <<=
					insn.IMMEDIATE;
			else ACTIVE_SET->at(insn.FIRST_REG) = 0;
			ACTIVE_SET->at(insn.FIRST_REG) &= 0x00ffffff;
			break;
		case LSHL:
			if(ACTIVE_SET->at(insn.SECOND_REG) < 24)
				ACTIVE_SET->at(insn.FIRST_REG) <<=
					ACTIVE_SET->at(insn.SECOND_REG);
			else ACTIVE_SET->at(insn.FIRST_REG) = 0;
			ACTIVE_SET->at(insn.FIRST_REG) &= 0x00ffffff;
			break;
		case LSHRI:
			if(insn.IMMEDIATE < 24)
				ACTIVE_SET->at(insn.FIRST_REG) >>=
					insn.IMMEDIATE;
			else ACTIVE_SET->at(insn.FIRST_REG) = 0;
			break;
		case LSHR:
			if(ACTIVE_SET->at(insn.SECOND_REG) < 24)
				ACTIVE_SET->at(insn.FIRST_REG) >>= 
					ACTIVE_SET->at(insn.SECOND_REG);
			else ACTIVE_SET->at(insn.FIRST_REG) = 0;
			ACTIVE_SET->at(insn.FIRST_REG) &= 0x00ffffff;
			break;
		case ROTR:
		case ROTL: // WIP !! UNIMPLEMENTED
			break;

		case SET:
			ACTIVE_SET->at(insn.FIRST_REG) |=
				(1 << (ACTIVE_SET->at(insn.SECOND_REG) % 24));
			break;
		case SETI:
			ACTIVE_SET->at(insn.FIRST_REG) |=
				(1 << (insn.IMMEDIATE % 24));
			break;

		case CLR:
			ACTIVE_SET->at(insn.FIRST_REG) &=
				~(1 << (ACTIVE_SET->at(insn.SECOND_REG) % 24));
			break;
		case CLRI:
			ACTIVE_SET->at(insn.FIRST_REG) &=
				~(1 << (insn.IMMEDIATE % 24));
			break;

		case CALW:
			SET_COND();
			break;
		case CNEV:
			CLR_COND();
			break;

		case FADD:
			f32temp =  
				std::bit_cast<float>(ACTIVE_SET->at(insn.FIRST_REG) << 8);
			f32temp += 
				std::bit_cast<float>(ACTIVE_SET->at(insn.SECOND_REG) << 8);
			ACTIVE_SET->at(insn.FIRST_REG) = (std::bit_cast<u32>(f32temp)) >> 8;
			break;
		case FSUB:
			f32temp =  
				std::bit_cast<float>(ACTIVE_SET->at(insn.FIRST_REG) << 8);
			f32temp -= 
				std::bit_cast<float>(ACTIVE_SET->at(insn.SECOND_REG) << 8);
			ACTIVE_SET->at(insn.FIRST_REG) = (std::bit_cast<u32>(f32temp)) >> 8;
			break;
		case FMUL:
			f32temp =  
				std::bit_cast<float>(ACTIVE_SET->at(insn.FIRST_REG) << 8);
			f32temp *= 
				std::bit_cast<float>(ACTIVE_SET->at(insn.SECOND_REG) << 8);
			ACTIVE_SET->at(insn.FIRST_REG) = (std::bit_cast<u32>(f32temp)) >> 8;
			break;
		case FSQT:
			f32temp =  
				std::bit_cast<float>(ACTIVE_SET->at(insn.SECOND_REG) << 8);
			f32temp = std::sqrt(f32temp);
			ACTIVE_SET->at(insn.FIRST_REG) = (std::bit_cast<u32>(f32temp)) >> 8;
			break;
		case FDIV:
			f32temp =  
				std::bit_cast<float>(ACTIVE_SET->at(insn.FIRST_REG) << 8);
			f32temp /= 
				std::bit_cast<float>(ACTIVE_SET->at(insn.SECOND_REG) << 8);
			ACTIVE_SET->at(insn.FIRST_REG) = (std::bit_cast<u32>(f32temp)) >> 8;
			break;
		case FMOD:
			f32temp = 
				std::bit_cast<float>(ACTIVE_SET->at(insn.FIRST_REG) << 8);
			f32temp = 
				std::fmod(
					f32temp, 
					std::bit_cast<float>(ACTIVE_SET->at(insn.SECOND_REG) << 8)
				);
			ACTIVE_SET->at(insn.FIRST_REG) = (std::bit_cast<u32>(f32temp)) >> 8;
			break;
		case FLOG:
			f32temp =  
				std::bit_cast<float>(ACTIVE_SET->at(insn.SECOND_REG) << 8);
			f32temp = std::log2(f32temp);
			ACTIVE_SET->at(insn.FIRST_REG) = (std::bit_cast<u32>(f32temp)) >> 8;
			break;

		case FCNV:
			f32temp = float(ACTIVE_SET->at(insn.SECOND_REG));
			ACTIVE_SET->at(insn.FIRST_REG) = (std::bit_cast<u32>(f32temp)) >> 8;
			break;

		case FCST:
			f32temp = std::bit_cast<float>(ACTIVE_SET->at(insn.SECOND_REG) << 8);
			ACTIVE_SET->at(insn.FIRST_REG) = u32(f32temp) & 0x00ffffff;
			break;

		case FSIN:
			f32temp = std::bit_cast<float>(ACTIVE_SET->at(insn.SECOND_REG) << 8);
			f32temp = std::sin(f32temp);
			ACTIVE_SET->at(insn.FIRST_REG) = (std::bit_cast<u32>(f32temp)) >> 8;
			break;
		case FCOS:
			f32temp = std::bit_cast<float>(ACTIVE_SET->at(insn.SECOND_REG) << 8);
			f32temp = std::cos(f32temp);
			ACTIVE_SET->at(insn.FIRST_REG) = (std::bit_cast<u32>(f32temp)) >> 8;
			break;
		case FTAN:
			f32temp = std::bit_cast<float>(ACTIVE_SET->at(insn.SECOND_REG) << 8);
			f32temp = std::tan(f32temp);
			ACTIVE_SET->at(insn.FIRST_REG) = (std::bit_cast<u32>(f32temp)) >> 8;
			break;
		case FASIN:
			f32temp = std::bit_cast<float>(ACTIVE_SET->at(insn.SECOND_REG) << 8);
			f32temp = std::asin(f32temp);
			ACTIVE_SET->at(insn.FIRST_REG) = (std::bit_cast<u32>(f32temp)) >> 8;
			break;
		case FEXP:
			f32temp = std::bit_cast<float>(ACTIVE_SET->at(insn.SECOND_REG) << 8);
			f32temp = std::exp(f32temp);
			ACTIVE_SET->at(insn.FIRST_REG) = (std::bit_cast<u32>(f32temp)) >> 8;
			break;
		case FLN:
			f32temp = std::bit_cast<float>(ACTIVE_SET->at(insn.SECOND_REG) << 8);
			f32temp = std::log(f32temp);
			ACTIVE_SET->at(insn.FIRST_REG) = (std::bit_cast<u32>(f32temp)) >> 8;
			break;
		case FREC:
			f32temp = std::bit_cast<float>(ACTIVE_SET->at(insn.SECOND_REG) << 8);
			f32temp = (1.0) / f32temp;
			ACTIVE_SET->at(insn.FIRST_REG) = (std::bit_cast<u32>(f32temp)) >> 8;
			break;
		case FABS:
			f32temp = std::bit_cast<float>(ACTIVE_SET->at(insn.SECOND_REG) << 8);
			f32temp = std::abs(f32temp);
			ACTIVE_SET->at(insn.FIRST_REG) = (std::bit_cast<u32>(f32temp)) >> 8;
			break;

		case FNEG:
			f32temp = std::bit_cast<float>(ACTIVE_SET->at(insn.SECOND_REG) << 8);
			f32temp = -(f32temp);
			ACTIVE_SET->at(insn.FIRST_REG) = (std::bit_cast<u32>(f32temp)) >> 8;
			break;

		case FCGT:
			if(std::bit_cast<float>(ACTIVE_SET->at(insn.FIRST_REG)  << 8) >
			   std::bit_cast<float>(ACTIVE_SET->at(insn.SECOND_REG) << 8))
				SET_COND();
			else
				CLR_COND();
			break;
		case FCLT:
			if(std::bit_cast<float>(ACTIVE_SET->at(insn.FIRST_REG)  << 8) <
			   std::bit_cast<float>(ACTIVE_SET->at(insn.SECOND_REG) << 8))
				SET_COND();
			else
				CLR_COND();
			break;

		case FCPI:
			ACTIVE_SET->at(insn.FIRST_REG) = 
				std::bit_cast<u32>(3.141592653589f) >> 8;
			break;
		case FCE:
			ACTIVE_SET->at(insn.FIRST_REG) = 
				std::bit_cast<u32>(2.718281828459f) >> 8;
			break;
		case FC0:
			ACTIVE_SET->at(insn.FIRST_REG) = 
				std::bit_cast<u32>(0.0f) >> 8;
			break;
		case FC1:
			ACTIVE_SET->at(insn.FIRST_REG) = 
				std::bit_cast<u32>(1.0f) >> 8;
			break;
		case FC2:
			ACTIVE_SET->at(insn.FIRST_REG) = 
				std::bit_cast<u32>(2.0f) >> 8;
			break;
		case FCSQ2:
			ACTIVE_SET->at(insn.FIRST_REG) = 
				std::bit_cast<u32>(1.414213562373f) >> 8;
			break;
		case FCPHI:
			ACTIVE_SET->at(insn.FIRST_REG) = 
				std::bit_cast<u32>(1.618033988749f) >> 8;
			break;
		case FCTAU:
			ACTIVE_SET->at(insn.FIRST_REG) = 
				std::bit_cast<u32>(6.283185307179f) >> 8;
			break;

		case CINF:
			f32temp = std::bit_cast<float>(ACTIVE_SET->at(insn.FIRST_REG) << 8);
			if(std::isinf(f32temp))
				SET_COND();
			else
				CLR_COND();
			break;
		case CNAN:
			f32temp = std::bit_cast<float>(ACTIVE_SET->at(insn.FIRST_REG) << 8);
			if(std::isnan(f32temp))
				SET_COND();
			else
				CLR_COND();
			break;
		case CNRM:
			f32temp = std::bit_cast<float>(ACTIVE_SET->at(insn.FIRST_REG) << 8);
			if(std::isnormal(f32temp))
				SET_COND();
			else
				CLR_COND();
			break;

		case END:
			ACTIVE_SET->at(insn.FIRST_REG) =
				((ACTIVE_SET->at(insn.SECOND_REG) & 0x0000ff) << 16) |
				((ACTIVE_SET->at(insn.SECOND_REG) & 0x00ff00) << 0 ) |
				((ACTIVE_SET->at(insn.SECOND_REG) & 0xff0000) >> 16);
			break;
		case REV:
			ACTIVE_SET->at(insn.FIRST_REG) = 0;
			temp = ACTIVE_SET->at(insn.SECOND_REG);
			for(int i = 0; i < 24; i++)
			{
				if(temp & 1)
					ACTIVE_SET->at(insn.FIRST_REG) |= (1 << (23 - i));
				temp >>= 1;
			}
			break;
		case MEMMAP:
			if(PS & 0x0001)
			{
				LINKED_MMU->SET_MAPPING( 
					ACTIVE_SET->at(insn.FIRST_REG)  & 0xfff,
					ACTIVE_SET->at(insn.SECOND_REG) & 0xfff);
			}
			break;
		case MMU_SETRO:
			if(PS & 0x0001)
			{
				if(ACTIVE_SET->at(insn.FIRST_REG) & 0x1000)
					  LINKED_MMU->SET_READONLY(ACTIVE_SET->at(insn.FIRST_REG) & 0xfff);
				else
					LINKED_MMU->CLEAR_READONLY(ACTIVE_SET->at(insn.FIRST_REG) & 0xfff);
			}
			break;
		case MMU_CHKRO:
			if(PS & 0x0001)
			{
				ACTIVE_SET->at(insn.FIRST_REG) =
					LINKED_MMU->CHECK_READONLY(ACTIVE_SET->at(insn.FIRST_REG) & 0xfff)?1:0;
			}
			break;
		case MMU_SETUSR:
			if(PS & 0x0001)
			{
				if(ACTIVE_SET->at(insn.FIRST_REG) & 0x1000)
					  LINKED_MMU->SET_USERPERM(ACTIVE_SET->at(insn.FIRST_REG) & 0xfff);
				else
					LINKED_MMU->CLEAR_USERPERM(ACTIVE_SET->at(insn.FIRST_REG) & 0xfff);
			}
			break;
		case MMU_CHKUSR:
			if(PS & 0x0001)
			{
				ACTIVE_SET->at(insn.FIRST_REG) =
					LINKED_MMU->CHECK_USERPERM(ACTIVE_SET->at(insn.FIRST_REG) & 0xfff)?1:0;
			}
			break;


		case STRB:
			PUT_8 (ACTIVE_SET->at(insn.SECOND_REG),
			    u8(ACTIVE_SET->at(insn.FIRST_REG)));
			break;
		case STRW:
			PUT_16(ACTIVE_SET->at(insn.SECOND_REG),
			   u16(ACTIVE_SET->at(insn.FIRST_REG)));
			break;
		case STRS:
			PUT_24(ACTIVE_SET->at(insn.SECOND_REG),
			   u32(ACTIVE_SET->at(insn.FIRST_REG)));
			break;

		case ST2B:
			PUT_8 (ACTIVE_SET->at (insn.SECOND_REG),
			    u8(ACTIVE_SET->at (insn.FIRST_REG)));
			PUT_8 (ACTIVE_SET->at (insn.SECOND_REG) + 1,
			    u8(ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 )));
			break;
		case ST2W:
			PUT_16(ACTIVE_SET->at (insn.SECOND_REG),
			   u16(ACTIVE_SET->at (insn.FIRST_REG)));
			PUT_16(ACTIVE_SET->at (insn.SECOND_REG) + 2,
			   u16(ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 )));
			break;
		case ST2S:
			PUT_24(ACTIVE_SET->at (insn.SECOND_REG),
			   u32(ACTIVE_SET->at (insn.FIRST_REG)));
			PUT_24(ACTIVE_SET->at (insn.SECOND_REG) + 3,
			   u32(ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 )));
			break;
		case ST3S:
			PUT_24(ACTIVE_SET->at (insn.SECOND_REG),
			   u32(ACTIVE_SET->at (insn.FIRST_REG)));
			PUT_24(ACTIVE_SET->at (insn.SECOND_REG) + 3,
			   u32(ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 )));
			PUT_24(ACTIVE_SET->at (insn.SECOND_REG) + 6,
			   u32(ACTIVE_SET->at((insn.FIRST_REG  + 2) & 7 )));
			break;

		case STRBBI:
			ACTIVE_SET->at(insn.SECOND_REG) += 1;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			PUT_8 (ACTIVE_SET->at(insn.SECOND_REG),
			    u8(ACTIVE_SET->at(insn.FIRST_REG)));
			break;
		case STRWBI:
			ACTIVE_SET->at(insn.SECOND_REG) += 2;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			PUT_16(ACTIVE_SET->at(insn.SECOND_REG),
			   u16(ACTIVE_SET->at(insn.FIRST_REG)));
			break;
		case STRSBI:
			ACTIVE_SET->at(insn.SECOND_REG) += 3;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			PUT_24(ACTIVE_SET->at(insn.SECOND_REG),
			    u32(ACTIVE_SET->at(insn.FIRST_REG)));
			break;

		case STRBAI:
			PUT_8 (ACTIVE_SET->at(insn.SECOND_REG),
			    u8(ACTIVE_SET->at(insn.FIRST_REG) & 0x0000ff));
			ACTIVE_SET->at(insn.SECOND_REG) += 1;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			break;
		case STRWAI:
			PUT_16(ACTIVE_SET->at(insn.SECOND_REG),
			   u16(ACTIVE_SET->at(insn.FIRST_REG) & 0x00ffff));
			ACTIVE_SET->at(insn.SECOND_REG) += 2;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			break;
		case STRSAI:
			PUT_24(ACTIVE_SET->at(insn.SECOND_REG),
			    u32(ACTIVE_SET->at(insn.FIRST_REG)));
			ACTIVE_SET->at(insn.SECOND_REG) += 3;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			break;

		case STRBBD:
			ACTIVE_SET->at(insn.SECOND_REG) -= 1;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			PUT_8 (ACTIVE_SET->at(insn.SECOND_REG),
			    u8(ACTIVE_SET->at(insn.FIRST_REG) & 0x0000ff));
			break;
		case STRWBD:
			ACTIVE_SET->at(insn.SECOND_REG) -= 2;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			PUT_16(ACTIVE_SET->at(insn.SECOND_REG),
			   u16(ACTIVE_SET->at(insn.FIRST_REG) & 0x00ffff));
			break;
		case STRSBD:
			ACTIVE_SET->at(insn.SECOND_REG) -= 3;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			PUT_24(ACTIVE_SET->at(insn.SECOND_REG),
			    u32(ACTIVE_SET->at(insn.FIRST_REG)));
			break;

		case STRBAD:
			PUT_8 (ACTIVE_SET->at(insn.SECOND_REG),
			    u8(ACTIVE_SET->at(insn.FIRST_REG) & 0x0000ff));
			ACTIVE_SET->at(insn.SECOND_REG) -= 1;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			break;
		case STRWAD:
			PUT_16(ACTIVE_SET->at(insn.SECOND_REG),
			   u16(ACTIVE_SET->at(insn.FIRST_REG) & 0x00ffff));
			ACTIVE_SET->at(insn.SECOND_REG) -= 2;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			break;
		case STRSAD:
			PUT_24(ACTIVE_SET->at(insn.SECOND_REG),
			    u32(ACTIVE_SET->at(insn.FIRST_REG)));
			ACTIVE_SET->at(insn.SECOND_REG) -= 3;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			break;

		case ST2WBI:
			ACTIVE_SET->at(insn.SECOND_REG) += 4;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			PUT_16(ACTIVE_SET->at (insn.SECOND_REG),
			   u16(ACTIVE_SET->at (insn.FIRST_REG)));
			PUT_16(ACTIVE_SET->at (insn.SECOND_REG) + 2,
			   u16(ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 )));
			break;
		case ST2WBD:
			ACTIVE_SET->at(insn.SECOND_REG) -= 4;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			PUT_16(ACTIVE_SET->at (insn.SECOND_REG),
			   u16(ACTIVE_SET->at (insn.FIRST_REG)));
			PUT_16(ACTIVE_SET->at (insn.SECOND_REG) + 2,
			   u16(ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 )));
			break;
		case ST2WAI:
			PUT_16(ACTIVE_SET->at (insn.SECOND_REG),
			   u16(ACTIVE_SET->at (insn.FIRST_REG)));
			PUT_16(ACTIVE_SET->at (insn.SECOND_REG) + 2,
			   u16(ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 )));
			ACTIVE_SET->at(insn.SECOND_REG) += 4;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			break;
		case ST2WAD:
			PUT_16(ACTIVE_SET->at (insn.SECOND_REG),
			   u16(ACTIVE_SET->at (insn.FIRST_REG)));
			PUT_16(ACTIVE_SET->at (insn.SECOND_REG) + 2,
			   u16(ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 )));
			ACTIVE_SET->at(insn.SECOND_REG) -= 4;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			break;

		case ST2SBI:
			ACTIVE_SET->at(insn.SECOND_REG) += 6;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			PUT_24(ACTIVE_SET->at (insn.SECOND_REG),
			   u32(ACTIVE_SET->at (insn.FIRST_REG)));
			PUT_24(ACTIVE_SET->at (insn.SECOND_REG) + 3,
			   u32(ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 )));
			break;
		case ST2SBD:
			ACTIVE_SET->at(insn.SECOND_REG) -= 6;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			PUT_24(ACTIVE_SET->at (insn.SECOND_REG),
			   u32(ACTIVE_SET->at (insn.FIRST_REG)));
			PUT_24(ACTIVE_SET->at (insn.SECOND_REG) + 3,
			   u32(ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 )));
			break;
		case ST2SAI:
			PUT_24(ACTIVE_SET->at (insn.SECOND_REG),
			   u32(ACTIVE_SET->at (insn.FIRST_REG)));
			PUT_24(ACTIVE_SET->at (insn.SECOND_REG) + 3,
			   u32(ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 )));
			ACTIVE_SET->at(insn.SECOND_REG) += 6;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			break;
		case ST2SAD:
			PUT_24(ACTIVE_SET->at (insn.SECOND_REG),
			   u32(ACTIVE_SET->at (insn.FIRST_REG)));
			PUT_24(ACTIVE_SET->at (insn.SECOND_REG) + 3,
			   u32(ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 )));
			ACTIVE_SET->at(insn.SECOND_REG) -= 6;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			break;

		case ST3SBI:
			ACTIVE_SET->at(insn.SECOND_REG) += 9;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			PUT_24(ACTIVE_SET->at (insn.SECOND_REG),
			   u32(ACTIVE_SET->at (insn.FIRST_REG)));
			PUT_24(ACTIVE_SET->at (insn.SECOND_REG) + 3,
			   u32(ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 )));
			PUT_24(ACTIVE_SET->at (insn.SECOND_REG) + 6,
			   u32(ACTIVE_SET->at((insn.FIRST_REG  + 2) & 7 )));
			break;
		case ST3SBD:
			ACTIVE_SET->at(insn.SECOND_REG) -= 9;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			PUT_24(ACTIVE_SET->at (insn.SECOND_REG),
			   u32(ACTIVE_SET->at (insn.FIRST_REG)));
			PUT_24(ACTIVE_SET->at (insn.SECOND_REG) + 3,
			   u32(ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 )));
			PUT_24(ACTIVE_SET->at (insn.SECOND_REG) + 6,
			   u32(ACTIVE_SET->at((insn.FIRST_REG  + 2) & 7 )));
			break;
		case ST3SAI:
			PUT_24(ACTIVE_SET->at (insn.SECOND_REG),
			   u32(ACTIVE_SET->at (insn.FIRST_REG)));

			PUT_24(ACTIVE_SET->at (insn.SECOND_REG) + 3,
			   u32(ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 )));

			PUT_24(ACTIVE_SET->at (insn.SECOND_REG) + 6,
			   u32(ACTIVE_SET->at((insn.FIRST_REG  + 2) & 7 )));

			ACTIVE_SET->at(insn.SECOND_REG) += 9;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;

			break;
		case ST3SAD:
			PUT_24(ACTIVE_SET->at (insn.SECOND_REG),
			   u32(ACTIVE_SET->at (insn.FIRST_REG)));
			PUT_24(ACTIVE_SET->at (insn.SECOND_REG) + 3,
			   u32(ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 )));
			PUT_24(ACTIVE_SET->at (insn.SECOND_REG) + 6,
			   u32(ACTIVE_SET->at((insn.FIRST_REG  + 2) & 7 )));
			ACTIVE_SET->at(insn.SECOND_REG) -= 9;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			break;

		case LD2B:
			temp = ACTIVE_SET->at(insn.SECOND_REG);
			ACTIVE_SET->at(insn.FIRST_REG) = GET_8 (temp);
			ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 ) = 
				GET_8 (temp + 1);
			break;
		case LD2W:
			temp = ACTIVE_SET->at(insn.SECOND_REG);
			ACTIVE_SET->at(insn.FIRST_REG) = GET_16(temp);
			ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 ) = 
				GET_16(temp + 2);
			break;
		case LD2S:
			temp = ACTIVE_SET->at(insn.SECOND_REG);
			ACTIVE_SET->at(insn.FIRST_REG) = GET_24(temp);
			ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 ) = 
				GET_24(temp + 3);
			break;
		case LD3S:
			temp = ACTIVE_SET->at(insn.SECOND_REG);
			ACTIVE_SET->at(insn.FIRST_REG) = GET_24(temp);
			ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 ) = 
				GET_24(temp + 3);
			ACTIVE_SET->at((insn.FIRST_REG  + 2) & 7 ) = 
				GET_24(temp + 6);
			break;

		case LD2WBI:
			temp = ACTIVE_SET->at(insn.SECOND_REG);
			ACTIVE_SET->at(insn.SECOND_REG) += 4;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			ACTIVE_SET->at(insn.FIRST_REG) = GET_16(ACTIVE_SET->at(insn.SECOND_REG));
			ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 ) = 
				GET_16(ACTIVE_SET->at(insn.SECOND_REG) + 2);
			break;

		case LD2WBD:
			temp = ACTIVE_SET->at(insn.SECOND_REG);
			ACTIVE_SET->at(insn.SECOND_REG) -= 4;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			ACTIVE_SET->at(insn.FIRST_REG) = GET_16(ACTIVE_SET->at(insn.SECOND_REG));
			ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 ) = 
				GET_16(ACTIVE_SET->at(insn.SECOND_REG) + 2);
			break;
		case LD2WAI:
			temp = ACTIVE_SET->at(insn.SECOND_REG);
			ACTIVE_SET->at(insn.FIRST_REG) = GET_16(temp);
			ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 ) = 
				GET_16(temp + 2);
			ACTIVE_SET->at(insn.SECOND_REG) += 4;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			break;

		case LD2WAD:
			temp = ACTIVE_SET->at(insn.SECOND_REG);
			ACTIVE_SET->at(insn.FIRST_REG) = GET_16(ACTIVE_SET->at(insn.SECOND_REG));
			ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 ) = 
				GET_16(temp + 2);
			ACTIVE_SET->at(insn.SECOND_REG) -= 4;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			break;

		case LD2SBI:
			ACTIVE_SET->at(insn.SECOND_REG) += 6;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			temp = ACTIVE_SET->at(insn.SECOND_REG);
			ACTIVE_SET->at(insn.FIRST_REG) = GET_24(temp);
			ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 ) = 
				GET_24(temp + 3);
			break;
		case LD2SBD:
			ACTIVE_SET->at(insn.SECOND_REG) -= 6;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			temp = ACTIVE_SET->at(insn.SECOND_REG);
			ACTIVE_SET->at(insn.FIRST_REG) = GET_24(temp);
			ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 ) = 
				GET_24(temp + 3);
			break;
		case LD2SAI:
			temp = ACTIVE_SET->at(insn.SECOND_REG);
			ACTIVE_SET->at(insn.FIRST_REG) = GET_24(temp);
			ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 ) = 
				GET_24(temp + 3);
			ACTIVE_SET->at(insn.SECOND_REG) += 6;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			break;
		case LD2SAD:
			temp = ACTIVE_SET->at(insn.SECOND_REG);
			ACTIVE_SET->at(insn.FIRST_REG) = GET_24(temp);
			ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 ) = 
				GET_24(temp + 3);
			ACTIVE_SET->at(insn.SECOND_REG) -= 6;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			break;

		case LD3SBI:
			ACTIVE_SET->at(insn.SECOND_REG) += 9;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			temp = ACTIVE_SET->at(insn.SECOND_REG);
			ACTIVE_SET->at(insn.FIRST_REG) = GET_24(temp);
			ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 ) = 
				GET_24(temp + 3);
			ACTIVE_SET->at((insn.FIRST_REG  + 2) & 7 ) = 
				GET_24(temp + 6);
			break;
		case LD3SBD:
			ACTIVE_SET->at(insn.SECOND_REG) -= 9;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			temp = ACTIVE_SET->at(insn.SECOND_REG);
			ACTIVE_SET->at(insn.FIRST_REG) = GET_24(temp);
			ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 ) = 
				GET_24(temp + 3);
			ACTIVE_SET->at((insn.FIRST_REG  + 2) & 7 ) = 
				GET_24(temp + 6);

			break;
		case LD3SAI:
			temp = ACTIVE_SET->at(insn.SECOND_REG);
			ACTIVE_SET->at(insn.FIRST_REG) = GET_24(temp);
			ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 ) = 
				GET_24(temp + 3);
			ACTIVE_SET->at((insn.FIRST_REG  + 2) & 7 ) = 
				GET_24(temp + 6);
			ACTIVE_SET->at(insn.SECOND_REG) += 9;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			break;
		case LD3SAD:
			temp = ACTIVE_SET->at(insn.SECOND_REG);
			ACTIVE_SET->at(insn.FIRST_REG) = GET_24(temp);
			ACTIVE_SET->at((insn.FIRST_REG  + 1) & 7 ) = 
				GET_24(temp + 3);
			ACTIVE_SET->at((insn.FIRST_REG  + 2) & 7 ) = 
				GET_24(temp + 6);
			ACTIVE_SET->at(insn.SECOND_REG) -= 9;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			break;

		case LDRBI:
			ACTIVE_SET->at(insn.FIRST_REG) = GET_8 ((IP - 2) + insn.IMMEDIATE);
			break;
		case LDRWI:
			ACTIVE_SET->at(insn.FIRST_REG) = GET_16((IP - 2) + insn.IMMEDIATE);
			break;
		case LDRSI:
			ACTIVE_SET->at(insn.FIRST_REG) = GET_24((IP - 2) + insn.IMMEDIATE);
			break;

		case LDRB:
			ACTIVE_SET->at(insn.FIRST_REG) = GET_8 (ACTIVE_SET->at(insn.SECOND_REG));
			break;
		case LDRW:
			ACTIVE_SET->at(insn.FIRST_REG) = GET_16(ACTIVE_SET->at(insn.SECOND_REG));
			break;
		case LDRS:
			ACTIVE_SET->at(insn.FIRST_REG) = GET_24(ACTIVE_SET->at(insn.SECOND_REG));
			break;

		case LDRBBI:
			ACTIVE_SET->at(insn.SECOND_REG) += 1;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			ACTIVE_SET->at(insn.FIRST_REG) = GET_8 (ACTIVE_SET->at(insn.SECOND_REG));
			break;
		case LDRWBI:
			ACTIVE_SET->at(insn.SECOND_REG) += 2;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			ACTIVE_SET->at(insn.FIRST_REG) = GET_16(ACTIVE_SET->at(insn.SECOND_REG));
			break;
		case LDRSBI:
			ACTIVE_SET->at(insn.SECOND_REG) += 3;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			ACTIVE_SET->at(insn.FIRST_REG) = GET_24(ACTIVE_SET->at(insn.SECOND_REG));
			break;

		case LDRBAI:
			ACTIVE_SET->at(insn.FIRST_REG) = GET_8 (ACTIVE_SET->at(insn.SECOND_REG));
			ACTIVE_SET->at(insn.SECOND_REG) += 1;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			break;
		case LDRWAI:
			ACTIVE_SET->at(insn.FIRST_REG) = GET_16(ACTIVE_SET->at(insn.SECOND_REG));
			ACTIVE_SET->at(insn.SECOND_REG) += 2;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			break;
		case LDRSAI:
			ACTIVE_SET->at(insn.FIRST_REG) = GET_24(ACTIVE_SET->at(insn.SECOND_REG));
			ACTIVE_SET->at(insn.SECOND_REG) += 3;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			break;

		case LDRBBD:
			ACTIVE_SET->at(insn.SECOND_REG) -= 1;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			ACTIVE_SET->at(insn.FIRST_REG) = GET_8 (ACTIVE_SET->at(insn.SECOND_REG));
			break;
		case LDRWBD:
			ACTIVE_SET->at(insn.SECOND_REG) -= 2;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			ACTIVE_SET->at(insn.FIRST_REG) = GET_16(ACTIVE_SET->at(insn.SECOND_REG));
			break;
		case LDRSBD:
			ACTIVE_SET->at(insn.SECOND_REG) -= 3;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			ACTIVE_SET->at(insn.FIRST_REG) = GET_24(ACTIVE_SET->at(insn.SECOND_REG));
			break;

		case LDRBAD:
			ACTIVE_SET->at(insn.FIRST_REG) = GET_8 (ACTIVE_SET->at(insn.SECOND_REG));
			ACTIVE_SET->at(insn.SECOND_REG) -= 1;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			break;
		case LDRWAD:
			ACTIVE_SET->at(insn.FIRST_REG) = GET_16(ACTIVE_SET->at(insn.SECOND_REG));
			ACTIVE_SET->at(insn.SECOND_REG) -= 2;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			break;
		case LDRSAD:
			ACTIVE_SET->at(insn.FIRST_REG) = GET_24(ACTIVE_SET->at(insn.SECOND_REG));
			ACTIVE_SET->at(insn.SECOND_REG) -= 3;
			ACTIVE_SET->at(insn.SECOND_REG) &= 0x00ffffff;
			break;

		case PSHI:
			//std::printf("Entering PSHI with SP == %d\n", SP);
			SP -= 1;
			SP &= 0xffffff;
			PUT_8 (SP,  u8(insn.IMMEDIATE & 0x0000ff));
			//std::printf("Exiting PSHI with SP == %d\n", SP);
			break;
		case PSHB:
			SP -= 1;
			SP &= 0xffffff;
			PUT_8 (SP,  u8(ACTIVE_SET->at(insn.FIRST_REG) & 0x0000ff));
			break;
		case PSHW:
			SP -= 2;
			SP &= 0xffffff;
			PUT_16(SP, u16(ACTIVE_SET->at(insn.FIRST_REG) & 0x00ffff));
			break;
		case PSHS:
			SP -= 3;
			SP &= 0xffffff;
			PUT_24(SP, u32(ACTIVE_SET->at(insn.FIRST_REG) & 0xffffff));
			break;
		case PSH2W:
			SP -= 2;
			SP &= 0xffffff;
			PUT_16(SP, u16(ACTIVE_SET->at(insn.FIRST_REG) & 0x00ffff));
			SP -= 2;
			SP &= 0xffffff;
			PUT_16(SP, u16(ACTIVE_SET->at((insn.FIRST_REG + 1) & 7)));
			break;
		case PSH2S:
			SP -= 3;
			SP &= 0xffffff;
			PUT_24(SP, u32(ACTIVE_SET->at(insn.FIRST_REG) & 0xffffff));
			SP -= 3;
			SP &= 0xffffff;
			PUT_24(SP, u32(ACTIVE_SET->at((insn.FIRST_REG + 1) & 7)));
			break;
		case PSH3S:
			SP -= 3;
			SP &= 0xffffff;
			PUT_24(SP, u32(ACTIVE_SET->at(insn.FIRST_REG) & 0xffffff));
			SP -= 3;
			SP &= 0xffffff;
			PUT_24(SP, u32(ACTIVE_SET->at((insn.FIRST_REG + 1) & 7)));
			SP -= 3;
			SP &= 0xffffff;
			PUT_24(SP, u32(ACTIVE_SET->at((insn.FIRST_REG + 2) & 7)));
			break;
		case POP2W:
			ACTIVE_SET->at((insn.FIRST_REG + 1) & 7) = GET_16(SP);
			SP += 2;
			SP &= 0xffffff;
			ACTIVE_SET->at(insn.FIRST_REG) = GET_16(SP);
			SP += 2;
			SP &= 0xffffff;
			break;
		case POP2S:
			ACTIVE_SET->at((insn.FIRST_REG + 1) & 7) = GET_24(SP);
			SP += 3;
			SP &= 0xffffff;
			ACTIVE_SET->at(insn.FIRST_REG) = GET_24(SP);
			SP += 3;
			SP &= 0xffffff;
			break;
		case POP3S:
			ACTIVE_SET->at((insn.FIRST_REG + 2) & 7) = GET_24(SP);
			SP += 3;
			SP &= 0xffffff;
			ACTIVE_SET->at((insn.FIRST_REG + 1) & 7) = GET_24(SP);
			SP += 3;
			SP &= 0xffffff;
			ACTIVE_SET->at(insn.FIRST_REG) = GET_24(SP);
			SP += 3;
			SP &= 0xffffff;
			break;
		case JUNKB: // just pops off the stack
			SP += 1;
			SP &= 0xffffff;
			break;
		case JUNKW: // just pops off the stack
			SP += 2;
			SP &= 0xffffff;
			break;
		case JUNKS: // just pops off the stack
			SP += 3;
			SP &= 0xffffff;
			break;
		case POPB:
			ACTIVE_SET->at(insn.FIRST_REG) = GET_8 (SP);
			SP += 1;
			SP &= 0xffffff;
			break;
		case POPW:
			ACTIVE_SET->at(insn.FIRST_REG) = GET_16(SP);
			SP += 2;
			SP &= 0xffffff;
			break;
		case POPS:
			ACTIVE_SET->at(insn.FIRST_REG) = GET_24(SP);
			SP += 3;
			SP &= 0xffffff;
			break;

		case SWVB:
			temp = GET_8 (SP);
			SP += 1;
			SP &= 0xffffff;
			swvt = GET_8 (SP);
			PUT_8 (SP, temp);
			SP -= 1;
			SP &= 0xffffff;
			PUT_8 (SP, swvt);
			break;
					
		case SWVW:
			temp = GET_16(SP);
			SP += 2;
			SP &= 0xffffff;
			swvt = GET_16(SP);
			PUT_8 (SP, temp);
			SP -= 2;
			SP &= 0xffffff;
			PUT_8 (SP, swvt);
			break;
		
		case SWVS:
			temp = GET_24(SP);
			SP += 3;
			SP &= 0xffffff;
			swvt = GET_24(SP);
			PUT_8 (SP, temp);
			SP -= 3;
			SP &= 0xffffff;
			PUT_8 (SP, swvt);
			break;

		case TPAGE:
		case WPAGE:
			break; // WIP !! UNIMPLEMENTED

		case SEED: // move reg to rand_state
			RAND_STATE = 
				(RAND_STATE << 32) | ACTIVE_SET->at(insn.FIRST_REG);
			break;

		case RND:
			RAND_STATE = (RAND_STATE << 32) | hash_a(RAND_STATE);
			ACTIVE_SET->at(insn.FIRST_REG) = hash_a(RAND_STATE) & 0xffffff;
			break;
			
		case ZRO:
			ACTIVE_SET->at(insn.FIRST_REG) = 0;
			break;
		case FIL:
			ACTIVE_SET->at(insn.FIRST_REG) = 0xffffff;
			break;

		case CBITI:
			if(ACTIVE_SET->at(insn.FIRST_REG) & (1 << insn.IMMEDIATE))
				SET_COND();
			else
				CLR_COND();
			break;

		case CBIT:
			if(ACTIVE_SET->at(insn.FIRST_REG) & (1 << ACTIVE_SET->at(insn.SECOND_REG)))
				SET_COND();
			else
				CLR_COND();
			break;

		case TGLI:
			ACTIVE_SET->at(insn.FIRST_REG) =
				ACTIVE_SET->at(insn.FIRST_REG) ^ 
					(1 << insn.IMMEDIATE);
			break;
		case TGL:
			ACTIVE_SET->at(insn.FIRST_REG) =
				ACTIVE_SET->at(insn.FIRST_REG) ^ 
					(1 << ACTIVE_SET->at(insn.SECOND_REG));
			break;

		case RSP:
			ACTIVE_SET->at(insn.FIRST_REG) = SP;
			break;
		case WSP:
			if(PS & 0x0001)
				SP = ACTIVE_SET->at(insn.FIRST_REG);
			break;
		
		case RIP:
			ACTIVE_SET->at(insn.FIRST_REG) = IP;
			break;
		case WIP:
			if(PS & 0x0001)
				IP = ACTIVE_SET->at(insn.FIRST_REG);
			break;

		case RPS:
			ACTIVE_SET->at(insn.FIRST_REG) = PS;
			break;
		case WPS:
			if(PS & 0x0001)
				PS = ACTIVE_SET->at(insn.FIRST_REG);
			break;

		case RXS:
			ACTIVE_SET->at(insn.FIRST_REG) = XS;
			break;
		case WXS:
			if(PS & 0x0001)
				XS = ACTIVE_SET->at(insn.FIRST_REG);
			break;
		case CXS:
			if(XS)
				SET_COND();
			else
				CLR_COND();
			break;

		case RXV:
			ACTIVE_SET->at(insn.FIRST_REG) = XV;
			break;
		case WXV:
			if(PS & 0x0001)
				XV = ACTIVE_SET->at(insn.FIRST_REG);
			break;

		case SWPR:
			temp = ACTIVE_SET->at(insn.FIRST_REG);
			if(ACTIVE_SET == &SET_A)
			{
				SET_A[insn.FIRST_REG] = SET_B[insn.FIRST_REG];
				SET_B[insn.FIRST_REG] = temp;
			}
			else
			{
				SET_B[insn.FIRST_REG] = SET_A[insn.FIRST_REG];
				SET_A[insn.FIRST_REG] = temp;
			}
			break;
			
		case SHDW:
			if(ACTIVE_SET == &SET_A)
				SET_B[insn.FIRST_REG] = SET_A[insn.FIRST_REG];
			else
				SET_A[insn.FIRST_REG] = SET_B[insn.FIRST_REG];
			break;
			
		case LITE:
			if(ACTIVE_SET == &SET_B)
				SET_B[insn.FIRST_REG] = SET_A[insn.FIRST_REG];
			else
				SET_A[insn.FIRST_REG] = SET_B[insn.FIRST_REG];
			break;

		case SWAP:
			if(ACTIVE_SET == &SET_A)
				ACTIVE_SET = &SET_B;
			else
				ACTIVE_SET = &SET_A;
			break;


		case MVLA:
			ACTIVE_SET->at(0) = (insn.IMMEDIATE & 0xff); break;
		case MVLB:
			ACTIVE_SET->at(1) = (insn.IMMEDIATE & 0xff); break;
		case MVLC:
			ACTIVE_SET->at(2) = (insn.IMMEDIATE & 0xff); break;
		case MVLD:
			ACTIVE_SET->at(3) = (insn.IMMEDIATE & 0xff); break;
		case MVLE:
			ACTIVE_SET->at(4) = (insn.IMMEDIATE & 0xff); break;
		case MVLF:
			ACTIVE_SET->at(5) = (insn.IMMEDIATE & 0xff); break;
		case MVLG:
			ACTIVE_SET->at(6) = (insn.IMMEDIATE & 0xff); break;
		case MVLH:
			ACTIVE_SET->at(7) = (insn.IMMEDIATE & 0xff); break;

		case MVMA:
			ACTIVE_SET->at(0) &= 0x000000ff;
			ACTIVE_SET->at(0) |= (insn.IMMEDIATE & 0xff) << 8; break;
		case MVMB:
			ACTIVE_SET->at(1) &= 0x000000ff;
			ACTIVE_SET->at(1) |= (insn.IMMEDIATE & 0xff) << 8; break;
		case MVMC:
			ACTIVE_SET->at(2) &= 0x000000ff;
			ACTIVE_SET->at(2) |= (insn.IMMEDIATE & 0xff) << 8; break;
		case MVMD:
			ACTIVE_SET->at(3) &= 0x000000ff;
			ACTIVE_SET->at(3) |= (insn.IMMEDIATE & 0xff) << 8; break;
		case MVME:
			ACTIVE_SET->at(4) &= 0x000000ff;
			ACTIVE_SET->at(4) |= (insn.IMMEDIATE & 0xff) << 8; break;
		case MVMF:
			ACTIVE_SET->at(5) &= 0x000000ff;
			ACTIVE_SET->at(5) |= (insn.IMMEDIATE & 0xff) << 8; break;
		case MVMG:
			ACTIVE_SET->at(6) &= 0x000000ff;
			ACTIVE_SET->at(6) |= (insn.IMMEDIATE & 0xff) << 8; break;
		case MVMH:
			ACTIVE_SET->at(7) &= 0x000000ff;
			ACTIVE_SET->at(7) |= (insn.IMMEDIATE & 0xff) << 8; break;

		case MVHA:
			ACTIVE_SET->at(0) &= 0x0000ffff;
			ACTIVE_SET->at(0) |= (insn.IMMEDIATE & 0xff) << 16; break;
		case MVHB:
			ACTIVE_SET->at(1) &= 0x0000ffff;
			ACTIVE_SET->at(1) |= (insn.IMMEDIATE & 0xff) << 16; break;
		case MVHC:
			ACTIVE_SET->at(2) &= 0x0000ffff;
			ACTIVE_SET->at(2) |= (insn.IMMEDIATE & 0xff) << 16; break;
		case MVHD:
			ACTIVE_SET->at(3) &= 0x0000ffff;
			ACTIVE_SET->at(3) |= (insn.IMMEDIATE & 0xff) << 16; break;
		case MVHE:
			ACTIVE_SET->at(4) &= 0x0000ffff;
			ACTIVE_SET->at(4) |= (insn.IMMEDIATE & 0xff) << 16; break;
		case MVHF:
			ACTIVE_SET->at(5) &= 0x0000ffff;
			ACTIVE_SET->at(5) |= (insn.IMMEDIATE & 0xff) << 16; break;
		case MVHG:
			ACTIVE_SET->at(6) &= 0x0000ffff;
			ACTIVE_SET->at(6) |= (insn.IMMEDIATE & 0xff) << 16; break;
		case MVHH:
			ACTIVE_SET->at(7) &= 0x0000ffff;
			ACTIVE_SET->at(7) |= (insn.IMMEDIATE & 0xff) << 16; break;

		/* prefixation goes here */

		case PREF:
			SET_PREFIX(insn.IMMEDIATE);
			return 2;
			//break;

		/* prefix 0x01 */

		case LOOP:
			SP -= 3;
			SP &= 0xffffff;
			PUT_24(SP, IP);
			break;
		case LRET:
			temp = GET_24(SP);
			SP += 3;
			SP &= 0xffffff;
			IP = temp;
			break;

		/*
			FP48 is a truncated FP64/double-precision format

			regx_0: S EEE EEEE EEEE MMMM MMMM MMMM
			regx_1:  MMMM MMMM MMMM MMMM MMMM MMMM [ 0000 0000 0000 0000 0000 0000 0000 0000 ]

			the format should keep ~11.14 decimal digits of precision (log10 2^37),
			which is pretty good compared to fp32 (~7.225), and much more precise
			when comapring to fp24 (~4.82). Specifically, the improvement rate between
			fp24 --> fp48 is better (x2.3125) than the rate going from fp32 --> fp64
			(x2.20833..), which makes logarithmic sense.
		*/

		case FMOV_48:
			ACTIVE_SET->at(insn.FIRST_REG + 0) = ACTIVE_SET->at(insn.SECOND_REG + 0);
			ACTIVE_SET->at(insn.FIRST_REG + 1) = ACTIVE_SET->at(insn.SECOND_REG + 1);
			break;

		case FADD_48:
			temp  = ACTIVE_SET->at(insn.FIRST_REG);
			temp <<= 24;
			temp |= ACTIVE_SET->at((insn.FIRST_REG + 1) & 7);
			temp <<= 16;
			f64temp = std::bit_cast<double>(temp);

			temp  = ACTIVE_SET->at(insn.SECOND_REG);
			temp <<= 24;
			temp |= ACTIVE_SET->at((insn.SECOND_REG + 1) & 7);
			temp <<= 16;

			f64temp += std::bit_cast<double>(temp);

			ACTIVE_SET->at(insn.FIRST_REG + 0) = 
				(std::bit_cast<u64>(std::bit_cast<i64>(f64temp)) >> 40) & 0xffffff;
			ACTIVE_SET->at(insn.FIRST_REG + 1) = 
				(std::bit_cast<u64>(std::bit_cast<i64>(f64temp)) >> 16) & 0xffffff;
			break;

		case FSUB_48:
			temp  = ACTIVE_SET->at(insn.FIRST_REG);
			temp <<= 24;
			temp |= ACTIVE_SET->at((insn.FIRST_REG + 1) & 7);
			temp <<= 16;
			f64temp = std::bit_cast<double>(temp);

			temp  = ACTIVE_SET->at(insn.SECOND_REG);
			temp <<= 24;
			temp |= ACTIVE_SET->at((insn.SECOND_REG + 1) & 7);
			temp <<= 16;

			f64temp -= std::bit_cast<double>(temp);

			ACTIVE_SET->at(insn.FIRST_REG + 0) = 
				(std::bit_cast<u64>(std::bit_cast<i64>(f64temp)) >> 40) & 0xffffff;
			ACTIVE_SET->at(insn.FIRST_REG + 1) = 
				(std::bit_cast<u64>(std::bit_cast<i64>(f64temp)) >> 16) & 0xffffff;
			break;

		case FMUL_48:
			temp  = ACTIVE_SET->at(insn.FIRST_REG);
			temp <<= 24;
			temp |= ACTIVE_SET->at((insn.FIRST_REG + 1) & 7);
			temp <<= 16;
			f64temp = std::bit_cast<double>(temp);

			temp  = ACTIVE_SET->at(insn.SECOND_REG);
			temp <<= 24;
			temp |= ACTIVE_SET->at((insn.SECOND_REG + 1) & 7);
			temp <<= 16;

			f64temp *= std::bit_cast<double>(temp);

			ACTIVE_SET->at(insn.FIRST_REG + 0) = 
				(std::bit_cast<u64>(std::bit_cast<i64>(f64temp)) >> 40) & 0xffffff;
			ACTIVE_SET->at(insn.FIRST_REG + 1) = 
				(std::bit_cast<u64>(std::bit_cast<i64>(f64temp)) >> 16) & 0xffffff;
			break;

		case FDIV_48:
			temp  = ACTIVE_SET->at(insn.FIRST_REG);
			temp <<= 24;
			temp |= ACTIVE_SET->at((insn.FIRST_REG + 1) & 7);
			temp <<= 16;
			f64temp = std::bit_cast<double>(temp);

			temp  = ACTIVE_SET->at(insn.SECOND_REG);
			temp <<= 24;
			temp |= ACTIVE_SET->at((insn.SECOND_REG + 1) & 7);
			temp <<= 16;

			f64temp /= std::bit_cast<double>(temp);

			ACTIVE_SET->at(insn.FIRST_REG + 0) = 
				(std::bit_cast<u64>(std::bit_cast<i64>(f64temp)) >> 40) & 0xffffff;
			ACTIVE_SET->at(insn.FIRST_REG + 1) = 
				(std::bit_cast<u64>(std::bit_cast<i64>(f64temp)) >> 16) & 0xffffff;
			break;

		case FCGT_48:
			temp  = ACTIVE_SET->at(insn.FIRST_REG);
			temp <<= 24;
			temp |= ACTIVE_SET->at((insn.FIRST_REG + 1) & 7);
			temp <<= 16;
			f64temp = std::bit_cast<double>(temp);

			temp  = ACTIVE_SET->at(insn.SECOND_REG);
			temp <<= 24;
			temp |= ACTIVE_SET->at((insn.SECOND_REG + 1) & 7);
			temp <<= 16;

			if(f64temp > std::bit_cast<double>(temp))
				SET_COND();
			else
				CLR_COND();
			break;

		case FCLT_48:
			temp  = ACTIVE_SET->at(insn.FIRST_REG);
			temp <<= 24;
			temp |= ACTIVE_SET->at((insn.FIRST_REG + 1) & 7);
			temp <<= 16;
			f64temp = std::bit_cast<double>(temp);

			temp  = ACTIVE_SET->at(insn.SECOND_REG);
			temp <<= 24;
			temp |= ACTIVE_SET->at((insn.SECOND_REG + 1) & 7);
			temp <<= 16;

			if(f64temp < std::bit_cast<double>(temp))
				SET_COND();
			else
				CLR_COND();
			break;

		case FEXP_48:
			temp  = ACTIVE_SET->at(insn.FIRST_REG);
			temp <<= 24;
			temp |= ACTIVE_SET->at((insn.FIRST_REG + 1) & 7);
			temp <<= 16;
			f64temp = std::bit_cast<double>(temp);

			temp  = ACTIVE_SET->at(insn.SECOND_REG);
			temp <<= 24;
			temp |= ACTIVE_SET->at((insn.SECOND_REG + 1) & 7);
			temp <<= 16;

			f64temp = std::exp(f64temp);

			ACTIVE_SET->at(insn.FIRST_REG + 0) = 
				(std::bit_cast<u64>(std::bit_cast<i64>(f64temp)) >> 40) & 0xffffff;
			ACTIVE_SET->at(insn.FIRST_REG + 1) = 
				(std::bit_cast<u64>(std::bit_cast<i64>(f64temp)) >> 16) & 0xffffff;
			break;

		case FMADD_48:
			break;

		case MASK:
			SET_MASKED_INT();
			break;
		case UNMASK:
			CLR_MASKED_INT();
			break;
			
		case WFINT:
			//std::printf("DEBUG: WFI hit! IP: [0x%08x]\n", IP - 2);
			SET_WFI();
			IP -= 2;
			break;

		case KERNI:
			if(!IS_MASKED_INT()) 
			{
				FETCHED_INSN = 0x0000;
				DECODED_INSN = {NOP, -1, -1, -32767, false, false};

				SET_MASKED_INT();
				XS = insn.IMMEDIATE;
				SP -= 3;
				SP &= 0xffffff;
				PUT_24(SP, IP);
				PS |= 0x0001;   // add kernel perms
				IP  = XV;       // jump to exception vector
				//std::printf("\tDEBUG: KERNI hit! XS: [0x%04x]\n", XS);
			}
			else
			{
				//std::printf("\tKERNI masked!\n");
			}

			break;

		case ERET:
			IP = GET_24(SP);
			SP += 3;
			SP &= 0xffffff;
			PS &= 0xfffe;   // and remove kernel perms
			break;

		case TRAPI:
			return (insn.IMMEDIATE) << 8;
			break;

		case SYSCI:
			return EXEC_SYSC(insn.IMMEDIATE, this);
			break;

		default:
			std::printf("Unknown insn encountered!\n");
			return -1;
	}

	if(insn.FIRST_REG >= 0)
		ACTIVE_SET->at(insn.FIRST_REG) &= 0x00ffffff;
	return 0;
}

int EXEC_SYSC(u16 SYSC, CPU* me)
{
	u32 A = me->ACTIVE_SET->at(0) & 0x000000ff;
	u32 B = me->ACTIVE_SET->at(1) & 0x00ffffff;

	switch(SYSC)
	{
		case 0: break;

		case 1:
			if(A >= me->DEVICES.size())
				me->ACTIVE_SET->at(0) = 0x00ffffff;
			else
			{
				me->DEVICES.at(A)->SYSC(B, me);
			}
			break;

		default:  break;
	}
	return 0;
}

int EXEC_JUMP(INSN insn, CPU* me) 
{
	me->SET_MASKED_INT();

	if(insn.OPERATION & LINK)
	{
		me->SP -= 3;
		me->SP &= 0xffffff;
		me->PUT_24(me->SP, me->IP);
	}

	//std::printf("%s\n", unmappings[insn.OPERATION].c_str());

	i32 temp;
	switch(insn.OPERATION)
	{
		case JMFAR:
		case JLFAR:
		case JMA:
		case JLA:
		case JMO:
		case JLO:
			temp = insn.IMMEDIATE * 2;
			break;

		case JMZO:
		case JLZO:
		case JMNZO:
		case JLNZO:
			if(insn.FIRST_REG != -127)
			{
				temp = insn.IMMEDIATE;
				if(temp & 0b100000)
				{
					temp &= 0b011111;
					temp = -temp;
				}
				temp *= 2;
			}
			break;
		default: break;
	}

	me->IP -= 2;
	me->IP &= 0x00ffffff;

	//std::printf("Jump! located @ <0x%08x>, tick # {0x%08x} from [0x%06x]", me->IP, me->TICKS);

	switch(insn.OPERATION)
	{
		case RET:
			
			temp = me->GET_24(me->SP);
			me->SP += 3;
			me->SP &= 0xffffff;
			me->IP = temp;
			break;

		case JMA:
		case JLA:
			me->IP = temp;
			break;
		
		case JMZR:
		case JLZR:
			if(me->ACTIVE_SET->at(insn.FIRST_REG) == 0)
				me->IP = me->ACTIVE_SET->at(insn.SECOND_REG);
			else me->IP += 2;
			break;

		case JMNZR:
		case JLNZR:
			if(me->ACTIVE_SET->at(insn.FIRST_REG) != 0)
				me->IP = me->ACTIVE_SET->at(insn.SECOND_REG);
			else me->IP += 2;
			break;

		case JMR:
		case JLR:
			me->IP = me->ACTIVE_SET->at(insn.FIRST_REG);
			break;

		case JMFAR:
		case JLFAR:
		case JMO:
		case JLO:
			me->IP += temp;
			break;

		case JMRO:
		case JLRO:
			me->IP = me->ACTIVE_SET->at(insn.FIRST_REG) + temp;
			break;

		case JMZO:
		case JLZO:
			if(me->ACTIVE_SET->at(insn.FIRST_REG) == 0)
				me->IP += temp;
			else me->IP += 2;
			break;

		case JMNZO:
		case JLNZO:
			if(me->ACTIVE_SET->at(insn.FIRST_REG) != 0)
				me->IP += temp;
			else me->IP += 2;
			break;
		
		default:
			break;	
	}

	me->IP &= 0x00ffffff;

	//std::printf(" to [0x%06x]\n", me->IP);

	me->CLR_MASKED_INT();

	return 1; 
}
