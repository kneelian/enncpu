#include "types.hpp"

u16 MMU::READ_8(u32 addr, u16 proc_state)
{	
	u32 page = (addr & 0x00'ff'f0'00) >> 12;
	addr = (MAPPINGS[page] << 12) | (addr & 0x00'00'0f'ff);

	return DATA[addr];
}

u32 MMU::READ_16(u32 addr, u16 proc_state)
{
	u32 page = (addr & 0x00'ff'f0'00) >> 12;
	page = (MAPPINGS[page] << 12);
	u32 subp = (addr & 0x00'00'0f'ff) >> 0;
	addr = page | subp;

	u16 temp  = (DATA[addr] << 8);
	 addr = (page | ((subp + 1) & 0x0fff)) & 0x00ffffff;
	    temp |= (DATA[addr] << 0);

	return temp;
}

u32 MMU::READ_24(u32 addr, u16 proc_state)
{
	u32 page = (addr & 0x00'ff'f0'00) >> 12;
	page = (MAPPINGS[page] << 12);
	u32 subp = (addr & 0x00'00'0f'ff) >> 0;
	addr = page | subp;

	u32 temp  = (DATA[addr] << 16);
	 addr = (page | ((subp + 1) & 0x0fff)) & 0x00ffffff;
	    temp |= (DATA[addr] <<  8);
	 addr = (page | ((subp + 1) & 0x0fff)) & 0x00ffffff;
	    temp |= (DATA[addr] <<  0);
 
	return temp;
}

u32 MMU::READ_32(u32 addr, u16 proc_state)
{
	u32 page = (addr & 0x00'ff'f0'00) >> 12;
	addr = (MAPPINGS[page] << 12) | (addr & 0x00'00'0f'ff);

	u32 temp  = (DATA[addr + 0] << 24);
	 	temp |= (DATA[addr + 1] << 16);
	    temp |= (DATA[addr + 2] <<  8);
	    temp |= (DATA[addr + 3] <<  0);

	return temp;
}

u64 MMU::READ_64(u64 addr, u16 proc_state)
{
	u32 page = (addr & 0x00'ff'f0'00) >> 12;
	addr = (MAPPINGS[page] << 12) | (addr & 0x00'00'0f'ff);

	u64 temp  = (DATA[addr + 0]); temp <<= 8;
	 	temp |= (DATA[addr + 1]); temp <<= 8;
	    temp |= (DATA[addr + 2]); temp <<= 8;
	    temp |= (DATA[addr + 3]); temp <<= 8;
	 	temp |= (DATA[addr + 4]); temp <<= 8;
	 	temp |= (DATA[addr + 5]); temp <<= 8;
	    temp |= (DATA[addr + 6]); temp <<= 8;
	    temp |= (DATA[addr + 7]);

	return temp;
}

void MMU::WRITE_8(u32 addr, u16 proc_state, u8 payload)
{
	u32 page = (addr & 0x00'ff'f0'00) >> 12;
	addr = (MAPPINGS[page] << 12) | (addr & 0x00'00'0f'ff);

	DATA[addr] = (payload >> 0) & 0xff;
	return;
}

void MMU::WRITE_16(u32 addr, u16 proc_state, u16 payload)
{
	u32 page = (addr & 0x00'ff'f0'00) >> 12;
	page = (MAPPINGS[page] << 12);
	u32 subp = (addr & 0x00'00'0f'ff) >> 0;
	addr = page | subp;

	DATA[addr] = (payload >> 8) & 0xff;
	addr = (page | ((subp + 1) & 0x0fff)) & 0x00ffffff;
	DATA[addr] = (payload >> 0) & 0xff;
	return;
}

void MMU::WRITE_24(u32 addr, u16 proc_state, u32 payload)
{
	u32 page = (addr & 0x00'ff'f0'00) >> 12;
	page = (MAPPINGS[page] << 12);
	u32 subp = (addr & 0x00'00'0f'ff) >> 0;
	addr = page | subp;

	DATA[addr] = (payload >> 16) & 0xff;
	addr = (page | ((subp + 1) & 0x0fff)) & 0x00ffffff;
	DATA[addr] = (payload >> 8) & 0xff;
	addr = (page | ((subp + 2) & 0x0fff)) & 0x00ffffff;
	DATA[addr] = (payload >> 0) & 0xff;
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