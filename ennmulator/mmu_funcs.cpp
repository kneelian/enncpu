#include "types.hpp"

struct addr_t
{
	u32 page : 12;
	u32 byte : 12;
};

u16 MMU::READ_8(u32 addr, u16 proc_state)
{
	addr_t a;
	a.page = (addr >> 12);
	a.byte = (addr >>  0);

	if((proc_state & 0x0001) == 0)    // usermode perms
		a.page = MAPPINGS[a.page];    // mean mmu translation

	if((proc_state & 0x0001) or (CHECK_USERPERM(a.page) and CHECK_READABLE(a.page)))
	{
		u16 payload = 0;
		payload = DATA[(a.page << 12) | (a.byte)];
		return payload;
	}
	else return 0xffff;
}

u32 MMU::READ_16(u32 addr, u16 proc_state)
{
	addr_t a;
	a.page = (addr >> 12);
	a.byte = (addr >>  0);

	if((proc_state & 0x0001) == 0)    // usermode perms
		a.page = MAPPINGS[a.page];    // mean mmu translation

	if((proc_state & 0x0001) or (CHECK_USERPERM(a.page) and CHECK_READABLE(a.page)))
	{
		u32 payload = 0;
		payload |= DATA[(u32(a.page) << 12) | (a.byte)];
		payload = payload << 8;
		a.byte++;
		payload |= DATA[(u32(a.page) << 12) | (a.byte)];
		return payload;
	}
	else return 0xffffffff;
}

u32 MMU::READ_24(u32 addr, u16 proc_state)
{
	addr_t a;
	a.page = (addr >> 12);
	a.byte = (addr >>  0);

	if((proc_state & 0x0001) == 0)    // usermode perms
		a.page = MAPPINGS[a.page];    // mean mmu translation

	if((proc_state & 0x0001) or (CHECK_USERPERM(a.page) and CHECK_READABLE(a.page)))
	{
		u32 payload = 0;
		payload |= DATA[(a.page << 12) | (a.byte)];
		payload = payload << 8;
		a.byte++;
		payload |= DATA[(a.page << 12) | (a.byte)];
		payload = payload << 8;
		a.byte++;
		payload |= DATA[(a.page << 12) | (a.byte)];
		return payload;
	}
	else return 0xffffffff;
}

// executable code reads

u32 MMU::READ_16_CODE(u32 addr, u16 proc_state)
{
	addr_t a;
	a.page = (addr >> 12);
	a.byte = (addr >>  0);

	if((proc_state & 0x0001) == 0)    // usermode perms
		a.page = MAPPINGS[a.page];    // mean mmu translation

	if((proc_state & 0x0001) or (
		CHECK_USERPERM(a.page) and 
		CHECK_READABLE(a.page) and
		CHECK_EXECUTABLE(a.page))
		)
	{
		u32 payload = 0;
		payload |= DATA[(u32(a.page) << 12) | (a.byte)];
		payload = payload << 8;
		a.byte++;
		payload |= DATA[(u32(a.page) << 12) | (a.byte)];
		return payload;
	}
	else return 0xffffffff;
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

u64 MMU::READ_64(u32 addr, u16 proc_state)
{
	addr_t a;
	a.page = (addr >> 12);
	a.byte = (addr >>  0);

	u64 temp  = (DATA[(u32(a.page) << 12) | (a.byte + 0)]); temp <<= 8;
	 	temp |= (DATA[(u32(a.page) << 12) | (a.byte + 1)]); temp <<= 8;
	    temp |= (DATA[(u32(a.page) << 12) | (a.byte + 2)]); temp <<= 8;
	    temp |= (DATA[(u32(a.page) << 12) | (a.byte + 3)]); temp <<= 8;
	 	temp |= (DATA[(u32(a.page) << 12) | (a.byte + 4)]); temp <<= 8;
	 	temp |= (DATA[(u32(a.page) << 12) | (a.byte + 5)]); temp <<= 8;
	    temp |= (DATA[(u32(a.page) << 12) | (a.byte + 6)]); temp <<= 8;
	    temp |= (DATA[(u32(a.page) << 12) | (a.byte + 7)]);

	return temp;
}

void MMU::WRITE_8(u32 addr, u16 proc_state, u8 payload)
{
	addr_t a;
	a.page = (addr >> 12);
	a.byte = (addr >>  0);

	if((proc_state & 0x0001) == 0)    // usermode perms
		a.page = MAPPINGS[a.page];    // mean mmu translation

	if((proc_state & 0x0001) or (CHECK_USERPERM(a.page) and CHECK_WRITABLE(a.page)))
	{
		DATA[(a.page << 12) | (a.byte)] = (payload >> 0) & 0xff;
	};

	return;
}

void MMU::WRITE_16(u32 addr, u16 proc_state, u16 payload)
{
	addr_t a;
	a.page = (addr >> 12);
	a.byte = (addr >>  0);

	if((proc_state & 0x0001) == 0)    // usermode perms
		a.page = MAPPINGS[a.page];    // mean mmu translation

	if((proc_state & 0x0001) or (CHECK_USERPERM(a.page) and CHECK_WRITABLE(a.page)))
	{
		DATA[(a.page << 12) | (a.byte)] = (payload >> 8) & 0xff;
		a.byte++;
		DATA[(a.page << 12) | (a.byte)] = (payload >> 0) & 0xff;
	}
	return;
}

void MMU::WRITE_24(u32 addr, u16 proc_state, u32 payload)
{
	addr_t a;
	a.page = (addr >> 12);
	a.byte = (addr >>  0);

	if((proc_state & 0x0001) == 0)    // usermode perms
		a.page = MAPPINGS[a.page];    // mean mmu translation

	if((proc_state & 0x0001) or (CHECK_USERPERM(a.page) and CHECK_WRITABLE(a.page)))
	{
		DATA[(a.page << 12) | (a.byte)] = (payload >> 16) & 0xff;
		a.byte++;
		DATA[(a.page << 12) | (a.byte)] = (payload >>  8) & 0xff;
		a.byte++;
		DATA[(a.page << 12) | (a.byte)] = (payload >>  0) & 0xff;
	}
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
	if(addr >= 0x00'ffd'fff) // last 4096 entries
		addr = 0x00'ffd'fff;

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