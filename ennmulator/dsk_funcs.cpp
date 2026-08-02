#include "types.hpp"

bool DSK::LOAD_DSK(std::vector<u8>& whence)
{
    this->RAW_DATA.reserve(2880 * 512);
    for(size_t i = 0; i < whence.size() and i < (2880 * 512); i++)
        this->RAW_DATA.push_back(whence[i]);
    return true;
}

void IDENTIFY(CPU* cpu)
{
	cpu->ACTIVE_SET.at(0) = 0x0044534b; // "DSK"
	cpu->ACTIVE_SET.at(1) = 0x00454e4e; // "ENN"

	cpu->ACTIVE_SET.at(2) = 0x00000003; // three functions

	return;
}

void DSK::SYSC(u32 id, CPU* cpu)
{
	switch(id)
	{
		case 0: break;
		case 1: IDENTIFY(cpu); break;
		case 2: 
			TAKE_SECTOR(
				cpu->ACTIVE_SET.at(2), // C
				cpu->ACTIVE_SET.at(3), // D
				cpu->PS,
				cpu->LINKED_MMU);
			cpu->ACTIVE_SET.at(0) = 0x14;
			break;
		case 3: 
			GIVE_SECTOR(
				cpu->ACTIVE_SET.at(2), // C
				cpu->ACTIVE_SET.at(3), // D
				cpu->PS,
				cpu->LINKED_MMU);
			cpu->ACTIVE_SET.at(0) = 0x24;
			break;
		case 4: break;
		default: break;
	}
	return;
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

void DSK::TAKE_SECTOR(u32 which, u32 whence, u16 proc_state, MMU* mmu)
{
	return;
}