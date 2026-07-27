#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <array>
#include <functional>
#include <bit>

#include "operations.hpp"

#define i64  int64_t
#define i32  int32_t
#define i16  int16_t
#define i8   int8_t
#define u64 uint64_t
#define u32 uint32_t
#define u16 uint16_t
#define u8  uint8_t

struct MMU;
struct CPU; 

struct MMU
{
	bool MMU_VALID = false;

	u8 PERMS[4096];
	/* perms are a bitmap of
	   conditions;
	   		b0 - usermode
	   		b1 -
	   		b2 - readonly
	*/
	std::array<u8, 4096 * 4096 > DATA;

	u16 MAPPINGS[4096];

	MMU()
	{
		for(int i = 0; i < 4096; i++)
		{ 
			PERMS[i] = 0x0; 
			MAPPINGS[i] = i;
		} 
		MMU_VALID = true;
	}
	~MMU()
	{
	}

	bool CHECK_READONLY(u16 page);
	void CLEAR_READONLY(u16 page);
	void   SET_READONLY(u16 page);

	bool CHECK_PAGE_EXISTS(u16 page);

	bool CHECK_USERPERM(u16 page);
	void CLEAR_USERPERM(u16 page);
	void   SET_USERPERM(u16 page);

	u8 CHECK_MAPPING(u16 page);
	void SET_MAPPING(u16 from, u16 to);

	u16 READ_8 (u32 addr, u16 proc_state);
	u32 READ_16(u32 addr, u16 proc_state);
	u32 READ_24(u32 addr, u16 proc_state);
	u32 READ_32(u32 addr, u16 proc_state);
	u64 READ_64(u64 addr, u16 proc_state);

	void WRITE_8 (u32 addr, u16 proc_state, u8  payload);
	void WRITE_16(u32 addr, u16 proc_state, u16 payload);
	void WRITE_24(u32 addr, u16 proc_state, u32 payload);
};

struct DEVICE
{
	virtual void SYSC(u32 id, CPU* cpu) {}
};

struct DSK : public DEVICE
{
/*
	new format resembles a floppy. 512 bytes per
	sector, 18 * 160 sectors = 1440KB in total
	it should be able to read an image file raw
	and then seek through it.	
*/

	std::vector<u8> RAW_DATA;

	void SYSC(u32 id, CPU* cpu) override;

	bool LOAD_DSK(std::vector<u8>& whence);

	void GIVE_SECTOR(u32 which, u32 where,  u16 proc_state, MMU* mmu);
	void TAKE_SECTOR(u32 which, u32 whence, u16 proc_state, MMU* mmu);
};

struct INSN
{
	OP OPERATION = NOP;	

	i8  FIRST_REG = -1;
	i8  SECOND_REG = -1;

	i32 IMMEDIATE = -32767;

	bool HOLD_PREFIX = false;
	bool PREDICATED = false;
};

/*
	processor state PS bitmap
		0001  - kernelmode
		0002  - has MMU
		0004  - predicated
		0008  - is WFI
		0010  - masking interrupts

		0100  - interrupt pending
		0200  - is in interrupt
*/

struct CPU
{
	std::array<u32, 8> ACTIVE_SET, SHADOW_SET;

	u32 SP = 0x0000; // stack pointer
	u32 IP = 0x0000; // instruction pointer
	u16 PS = 0x0000; // processor state
	u16 XS = 0x0000; // exception state

	u32 XV = 0x0000; // exception vector
	u32 RA = 0x0000; // return address

	u8 PREFIX = 0x00;

	u64 RAND_STATE = 0x12345678deadbeef;

	inline bool IS_COND_SET() { return PS & 0x0004; }
	inline void SET_COND()    { PS |=  0x0004; }
	inline void CLR_COND()    { PS &=(~0x0004); }

	inline bool  IS_WFI() { return PS & 0x0008; }
	inline void SET_WFI() { PS |=   0x0008; }
	inline void CLR_WFI() { PS &= (~0x0008); }

	inline bool   IS_MASKED_INT() { return PS & 0x0010; }
	inline void  SET_MASKED_INT() { PS |=   0x0010;  }
	inline void  CLR_MASKED_INT() { PS &= (~0x0010); }
	inline bool  IS_PENDING_INT() { return PS & 0x0100; }
	inline void SET_PENDING_INT() { PS |=   0x0100;  }
	inline void CLR_PENDING_INT() { PS &= (~0x0100); }

	inline bool  IS_IN_INTERRUPT() { return PS & 0x0200; }
	inline void SET_IN_INTERRUPT() { PS |=   0x0200;  }
	inline void CLR_IN_INTERRUPT() { PS &= (~0x0200); }

	inline u8 PREFIX_STATE()  { return PREFIX; }
	inline void CLR_PREFIX()  { SET_PREFIX(0); }

	inline void SET_PREFIX(u8 p) 
	{ 
		if(PREFIX == p) { return; }
		PREFIX = p;
	}

	u64 TICKS = 0;

	u16  FETCHED_INSN = 0x0000;
	INSN DECODED_INSN;

	std::vector<DEVICE*> DEVICES;

	MMU* LINKED_MMU = nullptr;

	u8 *FALLBACK_PAGE = nullptr;

	CPU()
	{
		for(int i = 0; i < 8; i++)
		{
			ACTIVE_SET[i] = 0;
			SHADOW_SET[i] = 0;
		}
		PS = 0x0013;
		LINKED_MMU = new MMU;
	}
	~CPU()
	{
		if(LINKED_MMU != nullptr)
			delete LINKED_MMU;
		if(FALLBACK_PAGE != nullptr)
			delete FALLBACK_PAGE;
	}

	void PUT_8 (u32 addr, u8  payload);
	void PUT_16(u32 addr, u16 payload);
	void PUT_24(u32 addr, u32 payload);

	u8  GET_8 (u32);
	u16 GET_16(u32);
	u32 GET_24(u32);

	void TRAP(u16 WHAT)
	{
		if(IS_MASKED_INT()) return;

		XS = WHAT;
		SET_PENDING_INT();
	}

	bool SYSC(u16 ID);

	bool LOAD_NEW_FORMAT(std::vector<u8>& bytestream);

	void FETCH()
	{
		FETCHED_INSN = GET_16(IP);
		IP += 2;
	}

	inline void IRQ()
	{
		std::printf("IRQ! PS was: 0x%04x -->", PS);
		SET_MASKED_INT();
		CLR_PENDING_INT();
		SET_IN_INTERRUPT();

		std::printf(" 0x%04x\n", PS);

		std::printf(" > TOP OF SP IS [%06x] [%06x] [%06x] [%06x]\n", GET_24(SP+6), GET_24(SP+3), GET_24(SP), GET_24(SP-3));
		std::printf(" > PUSHING OLD IP: 0x%06x\n", IP);

		SP -= 3;
		SP &= 0x00ff'ffff;
		PUT_24(SP, IP);

		IP = XV;

		std::printf(" > NEW IP @ 0x%06x\n", IP); 
		std::printf(" > TOP OF SP IS [%06x] [%06x] [%06x] [%06x]\n", GET_24(SP+6), GET_24(SP+3), GET_24(SP), GET_24(SP-3));
	}

	INSN DECODE(u16);
	i32 EXECUTE(INSN);
	bool STEP()
	{
		TICKS++;
		if(!IS_MASKED_INT() and IS_PENDING_INT()) 
		{ 
			IRQ();  
		}
		FETCH();
		DECODED_INSN = DECODE(FETCHED_INSN);
		if(EXECUTE(DECODED_INSN) < 0) 
			 return false;
		else return true;
	}
};
