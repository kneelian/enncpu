#include "types.hpp"

void IDENTIFY(CPU* cpu)
{
	cpu->ACTIVE_SET->at(0) = 0x0044534b; // "DSK"
	cpu->ACTIVE_SET->at(1) = 0x00454e4e; // "ENN"

	cpu->ACTIVE_SET->at(2) = 0x00000003; // three functions

	return;
}

void DSK::SYSC(u32 id, CPU* cpu)
{
	switch(id)
	{
		case 1: IDENTIFY(cpu); break;
		case 2: break;
		case 3: 
			GIVE_SECTOR(
				cpu->ACTIVE_SET->at(2),
				cpu->ACTIVE_SET->at(3),
				cpu->PS,
				cpu->LINKED_MMU);
			cpu->ACTIVE_SET->at(0) = 0x24;
			break;
		case 4:
			TAKE_SECTOR(
				cpu->ACTIVE_SET->at(2),
				cpu->ACTIVE_SET->at(3),
				cpu->PS,
				cpu->LINKED_MMU);
			cpu->ACTIVE_SET->at(0) = 0x34;
			break;
		default: break;
	}
	return;
}

bool DSK::LOAD_DSK(std::vector<u8>& whence)
{
    this->RAW_DATA.reserve(2880 * 512);
    for(size_t i = 0; i < whence.size() and i < (2880 * 512); i++)
        this->RAW_DATA.push_back(whence[i]);
    return true;
}

void DSK::GIVE_SECTOR(u32 which, u32 where, u16 proc_state, MMU* mmu)
{
	u32 sec_base = which << 9;
	for(u32 i = sec_base; i < (sec_base + 512); i++)
	{
		mmu->WRITE_8(where, proc_state, RAW_DATA[i]);
		where++;
	}
	return;
}

void DSK::TAKE_SECTOR(u32 which, u32 where, u16 proc_state, MMU* mmu)
{
	return;
}

/*

void DSK::LINK_MMU(MMU* which)
{
	LINKED_MMU = which;
	if(LINKED_MMU->CHECK_PAGE_EXISTS(0x7e) == false)
	{
		u8 raw_page_nr = LINKED_MMU->ADD_PAGE();
		LINKED_MMU->SET_MAPPING(raw_page_nr, 0x7e);
	}
	if(LINKED_MMU->CHECK_PAGE_EXISTS(0x7f) == false)
	{
		u8 raw_page_nr = LINKED_MMU->ADD_PAGE();
		LINKED_MMU->SET_MAPPING(raw_page_nr, 0x7f);
		LINKED_MMU->SET_READONLY(0x7f);
	}
}

void DSK::READ_SECTOR(u8 which, u16 proc_state)
{
	
	LINKED_MMU->CLEAR_READONLY(0x7f);
	for(size_t i = 2048 * which, j = 0; 
		j < 2048; 
		i++, j++)
	{
		LINKED_MMU->WRITE_8(0x7f'00'00 + j, proc_state, RAW_DATA[i]);
	}
	LINKED_MMU->WRITE_8(0x7f'00'00 + 2048, proc_state, 0xb4);
	LINKED_MMU->SET_READONLY(0x7f);
}

void DSK::WRITE_SECTOR(u8 which, u16 proc_state)
{
	if(LINKED_MMU->CHECK_PAGE_EXISTS(0x7e) == false)
		return; // nowhere to read from

	for(size_t i = 2048 * which, j = 0; 
		i < 2048 * (which + 1); 
		i++, j++)
	{
		RAW_DATA[i] = LINKED_MMU->READ_8(0x7f'00'00 + j, proc_state);
	}
	LINKED_MMU->WRITE_8(0x7f'00'00 + 2048, proc_state, 0xc3);
}*/