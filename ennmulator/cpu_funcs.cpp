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
	if(LINKED_MMU != nullptr)
	{
		LINKED_MMU->WRITE_16(addr, PS, (payload >> 0));
		return;
	}
	else return;
}

void CPU::PUT_24(u32 addr, u32 payload)
{
	if(LINKED_MMU != nullptr)
	{
		LINKED_MMU->WRITE_24(addr, PS, payload);
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

u16 CPU::GET_16_CODE(u32 addr)
{
	return LINKED_MMU->READ_16_CODE(addr, PS);
}

u32 CPU::GET_24(u32 addr)
{
	return LINKED_MMU->READ_24(addr, PS);
}

bool CPU::LOAD_NEW_FORMAT(std::vector<u8>& bstream, u32 length)
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

	printf("bitstream size is 0x%06x\n", bstream.size());

	u32 j = 0;

	while(j + 8 <= length)
	{
		if(bstream[j] != 0x64 or bstream[j + 1] != 0x04)
		{
			std::printf("No section header at 0x%06x (found %02x %02x)\n",
				j, bstream[j], bstream[j + 1]);
			return false;
		}

		sec_start  = (bstream[j + 2] << 16) | (bstream[j + 3] << 8) | (bstream[j + 4] << 0);
		sec_length = (bstream[j + 5] << 16) | (bstream[j + 6] << 8) | (bstream[j + 7] << 0);
		sec_base   = sec_start - 8;

		std::printf("Loading: 0x%06x : 0x%06x, 0x%06x, from j = 0x%06x\n", 
			sec_start, 
			sec_length, 
			sec_base,
			j);

		PUT_16(sec_base + 0, 0x6404);
		PUT_24(sec_base + 2, sec_start);
		PUT_24(sec_base + 5, sec_length);
		
		for(u32 i = 0; i < sec_length; i++)
		{
			PUT_8 (sec_start + i, bstream[j + 8 + i]);
		}

		j += 8 + sec_length;
	}
	return true;
}