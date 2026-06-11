#pragma once

#include <sstream>
#include <vector>
#include <string>

#include "types.hpp"

void PREPROCESS(std::vector<std::string>& raw_lines);

void BREAK_LINES(std::vector<std::vector<std::string>>& lines, std::string& in);

void PARSE_LINES(
	std::vector<std::vector<std::string>>& lines, 
	std::vector<LINE>& out, 
	u32& current_address,
	std::unordered_map<std::string, u32>& label_addr);

LINE PARSE_THIS(
	std::vector<std::string>& line, 
	u32& current_address,
	std::unordered_map<std::string, u32>& label_addr,
	std::vector<LINE>& out,
	u8& state,
	LINE& prev_line);

void DISAMBIGUATE(std::vector<LINE>& lines);

void RESOLVE_LABELS(
	std::vector<LINE>& lines,
	std::unordered_map<std::string, u32>& label_addr);

void SANITY_CHECK(std::vector<LINE>& lines);

void ASSEMBLE(
	std::vector<LINE>& lines,
	u32& current_address,
	std::vector<u8>& out_asm);