#include "types.hpp"

u16 MMU::READ_8(u32 addr, u16 proc_state)
{
	u16 page_nr = (addr >> 12) & 0xfff;
	u16 subaddr = (addr & 0xfff);

	if( PERMS[MAPPINGS[page_nr]] == 0xff )
		return -1; // error: mapped page doesn't exist
	if( ((proc_state & 1) == 0) and 
		((PERMS[MAPPINGS[page_nr]] & 1) == 1)
	)   return -2; // error: no permission to read page
		
	return PAGES[MAPPINGS[page_nr]][subaddr];
}

u32 MMU::READ_16(u32 addr, u16 proc_state)
{
	u16 page_nr = (addr >> 12) & 0xfff;
	u16 subaddr = (addr & 0xfff);

	if( PERMS[MAPPINGS[page_nr]] == 0xff )
		return -1; // error: mapped page doesn't exist
	if( ((proc_state & 1) == 0) and 
		((PERMS[MAPPINGS[page_nr]] & 1) == 1)
	)   return -2; // error: no permission to read page

	u16 temp  = (PAGES[MAPPINGS[page_nr]][subaddr] << 8);
	 subaddr  = (subaddr + 1) & 0xfff;
	    temp |= (PAGES[MAPPINGS[page_nr]][subaddr] << 0);

	return temp;
}

u32 MMU::READ_24(u32 addr, u16 proc_state)
{
	u16 page_nr = (addr >> 12) & 0xfff;
	u16 subaddr = (addr & 0xfff);

	if( PERMS[MAPPINGS[page_nr]] == 0xff )
		return -1; // error: mapped page doesn't exist
	if( ((proc_state & 1) == 0) and 
		((PERMS[MAPPINGS[page_nr]] & 1) == 1)
	)   return -2; // error: no permission to read page
	
	u32 temp  = (PAGES[MAPPINGS[page_nr]][subaddr] << 16);
	 subaddr  = (subaddr + 1) & 0xfff;
	    temp |= (PAGES[MAPPINGS[page_nr]][subaddr] <<  8);
	 subaddr  = (subaddr + 1) & 0xfff;
	    temp |= (PAGES[MAPPINGS[page_nr]][subaddr] <<  0);
 
	return temp;
}

u32 MMU::READ_32(u32 addr, u16 proc_state)
{
	u16 page_nr = (addr >> 12) & 0xfff;
	u16 subaddr = (addr & 0xfff);

	if( PERMS[MAPPINGS[page_nr]] == 0xff )
		return -1; // error: mapped page doesn't exist

	u32 temp  = (PAGES[MAPPINGS[page_nr]][subaddr + 0] << 24);
	 	temp |= (PAGES[MAPPINGS[page_nr]][subaddr + 1] << 16);
	    temp |= (PAGES[MAPPINGS[page_nr]][subaddr + 2] <<  8);
	    temp |= (PAGES[MAPPINGS[page_nr]][subaddr + 3] <<  0);

	return temp;
}

u64 MMU::READ_64(u64 addr, u16 proc_state)
{
	u16 page_nr = (addr >> 12) & 0xfff;
	u16 subaddr = (addr & 0xfff);

	if( PERMS[MAPPINGS[page_nr]] == 0xff )
		return -1; // error: mapped page doesn't exist

	u64 temp  = (PAGES[MAPPINGS[page_nr]][subaddr + 0]); temp <<= 8;
	 	temp |= (PAGES[MAPPINGS[page_nr]][subaddr + 1]); temp <<= 8;
	    temp |= (PAGES[MAPPINGS[page_nr]][subaddr + 2]); temp <<= 8;
	    temp |= (PAGES[MAPPINGS[page_nr]][subaddr + 3]); temp <<= 8;
	 	temp |= (PAGES[MAPPINGS[page_nr]][subaddr + 4]); temp <<= 8;
	 	temp |= (PAGES[MAPPINGS[page_nr]][subaddr + 5]); temp <<= 8;
	    temp |= (PAGES[MAPPINGS[page_nr]][subaddr + 6]); temp <<= 8;
	    temp |= (PAGES[MAPPINGS[page_nr]][subaddr + 7]);

	return temp;
}

void MMU::WRITE_8(u32 addr, u16 proc_state, u8 payload)
{
	u8  page_nr = (addr >> 12) & 0xfff;
	u16 subaddr = (addr & 0xfff);

	if( CHECK_READONLY(page_nr) or
	(((proc_state & 1) == 0) and CHECK_USERPERM(page_nr)))
		return; // error: no permission to write page

	PAGES[MAPPINGS[page_nr]][subaddr] = payload;
	return;
}

void MMU::WRITE_16(u32 addr, u16 proc_state, u16 payload)
{
	u8  page_nr = (addr >> 12) & 0xfff;
	u16 subaddr = (addr & 0xfff);

	if( CHECK_READONLY(page_nr) or
	(((proc_state & 1) == 0) and CHECK_USERPERM(page_nr)))
		return; // error: no permission to write page

	PAGES[MAPPINGS[page_nr]][subaddr] = (payload >> 8) & 0xff;
	subaddr = (subaddr + 1) & 0xfff;
	PAGES[MAPPINGS[page_nr]][subaddr] = (payload >> 0) & 0xff;
	return;
}

void MMU::WRITE_24(u32 addr, u16 proc_state, u32 payload)
{
	u8  page_nr = (addr >> 12) & 0xfff;
	u16 subaddr = (addr & 0xfff);

	payload &= 0x00ffffff;

	if( CHECK_READONLY(page_nr) or
	(((proc_state & 1) == 0) and CHECK_USERPERM(page_nr)))
		return; // error: no permission to write page

	PAGES[MAPPINGS[page_nr]][subaddr] = (payload >> 16) & 0xff;
	subaddr = (subaddr + 1) & 0xfff;
	PAGES[MAPPINGS[page_nr]][subaddr] = (payload >>  8) & 0xff;
	subaddr = (subaddr + 1) & 0xfff;
	PAGES[MAPPINGS[page_nr]][subaddr] = (payload >>  0) & 0xff;
	return;
}

bool MMU::CHECK_PAGE_EXISTS(u16 page)
{
	page &= 0xfff;
	if(PERMS[MAPPINGS[page]] == 0xff)
		 return false;
	else return true;
}
bool MMU::CHECK_USERPERM(u16 page)
{
	page &= 0xfff;
	return PERMS[MAPPINGS[page]] & 0x01;
}
void MMU::CLEAR_USERPERM(u16 page)
{
	page &= 0xfff;
	PERMS[MAPPINGS[page]] &= 0xfe;
}
void MMU::SET_USERPERM(u16 page)
{
	page &= 0xfff;
	PERMS[MAPPINGS[page]] |= 0x01;
}
bool MMU::CHECK_READONLY(u16 page)
{ 
	page &= 0xfff;
	return PERMS[MAPPINGS[page]] & 0x04; 
}
void MMU::CLEAR_READONLY(u16 page)
{
	page &= 0xfff;
	PERMS[MAPPINGS[page]] &= 0xfb;
}
void MMU::SET_READONLY(u16 page)
{
	page &= 0xfff;
	PERMS[MAPPINGS[page]] |= 0x04;
}

u8 MMU::CHECK_MAPPING(u16 page)
{
	page &= 0xfff;
	return MAPPINGS[page];
}

void MMU::SET_MAPPING(u16 from, u16 to)
{
	from &= 0xfff;
	to   &= 0xfff;
	MAPPINGS[from] = to;
	return;
}

i16 MMU::ADD_PAGE()
{
	if(LAST_PAGE == 4095) return -1;

	if(PAGES[LAST_PAGE] == nullptr)
		PAGES[LAST_PAGE] = new u8[4096];

	if(PAGES[LAST_PAGE] == nullptr)
		return -1;

	PERMS[LAST_PAGE] = 0x00;
	MAPPINGS[LAST_PAGE] = LAST_PAGE;
	
	LAST_PAGE++;

	return i16(LAST_PAGE) - 1;
}

i16 MMU::ADD_CHUNK()
{
	for(int i = 0; i < 16; i++)
		ADD_PAGE();
	return i16(LAST_PAGE) - 1;
}