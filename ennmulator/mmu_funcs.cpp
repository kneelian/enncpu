#include "types.hpp"

// raw reads that bypass MMU remapping

u16 MMU::READ_8_RAW (u32 addr) 
{
	addr &= 0x00'ff'ff'ff;
	return DATA[addr];
}

u32 MMU::READ_16_RAW(u32 addr)
{
	addr &= 0x00'ff'ff'ff;
	u16 temp = u16(DATA[addr]) << 8;
	addr = (addr + 1) & 0x00'ff'ff'ff;
		temp |= (DATA[addr] << 0);
	return temp;
}

u32 MMU::READ_24_RAW(u32 addr)
{
	addr &= 0x00'ff'ff'ff;
	u32 temp = u32(DATA[addr]) << 16;
	addr = (addr + 1) & 0x00'ff'ff'ff;
		temp |= (DATA[addr] << 8);
	addr = (addr + 1) & 0x00'ff'ff'ff;
		temp |= (DATA[addr] << 0);
	return temp;
}

// permission-checked reads with mapping

u16 MMU::READ_8(u32 addr, u16 proc_state)
{	
	if(proc_state & 0x0001)
		return READ_8_RAW(addr);

	u32 page = (addr & 0x00'ff'f0'00) >> 12;
	addr = (u32(MAPPINGS[page]) << 12) | (addr & 0x00'00'0f'ff);

	if( (PERMS[MAPPINGS[page]] & 0b0011) == 0b0011) // userperm and readable
		return DATA[addr];
	else return 0x0000;
}

u32 MMU::READ_16(u32 addr, u16 proc_state)
{
	if(proc_state & 0x0001)
		return READ_16_RAW(addr);

	u32 page = (addr & 0x00'ff'f0'00) >> 12;
	addr = (u32(MAPPINGS[page]) << 12) | (addr & 0x00'00'0f'ff);
	u16 subp = addr & 0x0fff;

	if( (PERMS[MAPPINGS[page]] & 0b0011) == 0b0011) // userperm and readable
	{
		u16 temp  = (DATA[addr] << 8);
		 addr = (page | ((subp + 1) & 0x0fff)) & 0x00ffffff;
		    temp |= (DATA[addr] << 0);
		return temp;
	}
	else return 0x0000;
}

u32 MMU::READ_24(u32 addr, u16 proc_state)
{
	if(proc_state & 0x0001)
		return READ_24_RAW(addr);

	u32 page = (addr & 0x00'ff'f0'00) >> 12;
	addr = (u32(MAPPINGS[page]) << 12) | (addr & 0x00'00'0f'ff);
	u16 subp = addr & 0x0fff;

	if( (PERMS[MAPPINGS[page]] & 0b0011) == 0b0011) // userperm and readable
	{
		u32 temp  = (DATA[addr] << 16);
		 addr = (page | ((subp + 1) & 0x0fff)) & 0x00ffffff;
		    temp |= (DATA[addr] <<  8);
		 addr = (page | ((subp + 2) & 0x0fff)) & 0x00ffffff;
		    temp |= (DATA[addr] <<  0);
		return temp;
	}
	else return 0x0000;
}

// executable code reads

u32 MMU::READ_16_CODE(u32 addr, u16 proc_state)
{
	if(proc_state & 0x0001)
		return READ_16_RAW(addr);

	u32 page = (addr & 0x00'ff'f0'00) >> 12;
	addr = (u32(MAPPINGS[page]) << 12) | (addr & 0x00'00'0f'ff);
	u16 subp = addr & 0x0fff;

	if( (PERMS[MAPPINGS[page]] & 0b0011) == 0b1011) // userperm and rx
	{
		u16 temp  = (DATA[addr] << 8);
		 addr = (page | ((subp + 1) & 0x0fff)) & 0x00ffffff;
		    temp |= (DATA[addr] << 0);
		return temp;
	}
	else return 0x0000;
}

// for the instruction cache thing we used to do

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

// perm checks

bool MMU::CHECK_USERPERM(u16 page)
{
	page &= 0xfff;
	return PERMS[page] & 0x01;
}
bool MMU::CHECK_READABLE(u16 page)
{
	page &= 0xfff;
	return PERMS[page] & 0x02;
}
bool MMU::CHECK_WRITABLE(u16 page)
{
	page &= 0xfff;
	return PERMS[page] & 0x04;
}
bool MMU::CHECK_EXECUTABLE(u16 page)
{
	page &= 0xfff;
	return PERMS[page] & 0x08;
}
bool MMU::CHECK_READONLY(u16 page)
{ return !(this->CHECK_WRITABLE(page)); }

// individual page perm mods

void MMU::CLR_USERPERM(u16 page)
{
	page &= 0xfff;
	PERMS[page] &= 0xfe;
}
void MMU::SET_USERPERM(u16 page)
{
	page &= 0xfff;
	PERMS[page] |= 0x01;
}
void MMU::CLR_READABLE(u16 page)
{
	page &= 0xfff;
	PERMS[page] &= 0xfd;
}
void MMU::SET_READABLE(u16 page)
{
	page &= 0xfff;
	PERMS[page] |= 0x02;
}
void MMU::CLR_WRITABLE(u16 page)
{
	page &= 0xfff;
	PERMS[page] &= 0xfb;
}
void MMU::SET_WRITABLE(u16 page)
{
	page &= 0xfff;
	PERMS[page] |= 0x04;
}
void MMU::CLR_EXECUTABLE(u16 page)
{
	page &= 0xfff;
	PERMS[page] &= 0xf7;
}
void MMU::SET_EXECUTABLE(u16 page)
{
	page &= 0xfff;
	PERMS[page] |= 0x08;
}

// mapping page to virtpage

u16 MMU::CHECK_MAPPING(u16 page)
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

void MMU::FROM_TBL(u32 addr)
{
	addr &= 0x00'ff'ff'ff;
	if(addr >= 0x00'ff'df'ff) // last 4096 entries
		addr = 0x00'ff'df'ff;

	for(u16 i = 0; i < 4096; i++)
	{
		u16 tmp   = (u16(DATA[addr]) << 8) | (u16(DATA[addr + 1]));
		u8  flags = tmp >> 12;
		tmp &= 0xfff;

		MAPPINGS[i] = tmp;
		PERMS[i] = flags;

		addr += 2;
	}
}