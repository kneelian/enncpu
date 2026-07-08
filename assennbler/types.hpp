#pragma once

#define u8  uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define i8   int8_t
#define i16  int16_t
#define i32  int32_t
#define i64  int64_t 

#include "operations.hpp"

enum DIRECTIVES : uint8_t
{
	NONE = 0x00,
	ORG,
	NUL,
	INT8,
	INT16,
	INT24,
	FP24,
	INT48,
	FP48,
	ASCII,
	ASCIZ,
	SECTION,
	PAD,
	REP,
	ALIGN,
};

struct LINE
{
	bool PREDICATED = false;
	DIRECTIVES DIREC = NONE;
	i8   REG_A = -127;
	i8   REG_B = -127;
	i8   REG_C = -127;
	OP   OPERATION = NOP;
	i16  IMMEDIATE = -1024;
	u32  POSITION = -1;
	i32  LABEL_POS = -1;
	i32  LABEL_OFF = -(1 << 24);
	u64  RAW_DATA = 0;
	std::string LABELNAME = "";
	u8   STATE = 0;
};