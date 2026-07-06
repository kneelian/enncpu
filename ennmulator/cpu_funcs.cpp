#include <bit>
#include "types.hpp"

void CPU::PUT_8(u32 addr, u8 payload)
{
	if(LINKED_MMU != nullptr)
	{
		LINKED_MMU->WRITE_8(addr, PS, payload);
		return;
	}
	else return;
}
	
void CPU::PUT_16(u32 addr, u16 payload)
{	
	u16 page_nr = (addr >> 12) & 0xfff;
	u16 subaddr = (addr & 0xfff);

	if(LINKED_MMU != nullptr)
	{
		LINKED_MMU->WRITE_16(addr, PS, (payload >> 0));
		return;
	}
	else return;
}

void CPU::PUT_24(u32 addr, u32 payload)
{
	u16 page_nr = (addr >> 12) & 0xfff;
	u16 subaddr = (addr & 0xfff);

	if(LINKED_MMU != nullptr)
	{
		LINKED_MMU->WRITE_24(addr, PS, (payload >> 0));
		return;
	}
	else return;
}

u8 CPU::GET_8(u32 addr)
{
	return LINKED_MMU->READ_8(addr, PS);
}

// reads and writes are big endian!
u16 CPU::GET_16(u32 addr)
{
	return LINKED_MMU->READ_16(addr, PS);
}
u32 CPU::GET_24(u32 addr)
{
	return LINKED_MMU->READ_24(addr, PS);
}

bool CPU::LOAD_NEW_FORMAT(std::vector<u8>& bstream)
{
	bool reading_section = false;
	u32  sec_start  = 0;
	u32  sec_length = 0;
	u32  sec_base   = 0;

	if(bstream.size () < 8 or (bstream[0] != 0x64 and bstream[1] != 0x04))
	{
		std::printf("Cannot load bytestream as new format executable!\n");
		return false;
	}

	for(u32 j = 2; j < bstream.size() - 6; j += 8 + sec_length)
	{
		u16 stamp  = (bstream[j - 1] <<  0) | (bstream[j - 2] << 8); 
			if(stamp != 0x6404) { break; }
		sec_start  = (bstream[j + 0] << 16) | (bstream[j + 1] << 8) | (bstream[j + 2] << 0);
		sec_length = (bstream[j + 3] << 16) | (bstream[j + 4] << 8) | (bstream[j + 5] << 0);
		sec_base   = sec_start - 8;

		PUT_16(sec_base + 0, 0x6404);
		PUT_24(sec_base + 2, sec_start);
		PUT_24(sec_base + 5, sec_length);
		
		for(u32 i = 0; i < sec_length; i++)
		{
			PUT_8 (sec_start + i, bstream[i + j + 6]);
		}
	}
	return true;
}