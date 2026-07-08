#include <bit>
#include <algorithm>

#include "utility.hpp"

void PARSE_LINES(
	std::vector<std::vector<std::string>>& lines, 
	std::vector<LINE>& out, 
	u32& current_address,
	std::unordered_map<std::string, u32>& label_addr)
{
	u8 STATE = 0;
	LINE prev_line = { false, NONE, -127, -127, -127, COMMENT, -1024, current_address, -1, -(1 << 24), 0, "", 0};
	out.push_back(prev_line);
	for(auto& i : lines)
	{
		out.push_back(PARSE_THIS(i, current_address, label_addr, out, STATE, out.back()));
	}
	return;
}

LINE PARSE_THIS(
	std::vector<std::string>& line, 
	u32& current_address,
	std::unordered_map<std::string, u32>& label_addr,
	std::vector<LINE>& out,
	u8& state,
	LINE& prev_line)
{
	LINE result;
	if(line.empty())
	{ 
		current_address -= 2;
		return result; 
	}

	if(line.size() > 1 and line[0] == "OR")
	{
		if(line[1] == "CEQ") line[1] = "CEQ_OR"; else
		if(line[1] == "CNE") line[1] = "CNE_OR"; else
		if(line[1] == "CGT") line[1] = "CGT_OR"; else
		if(line[1] == "CLT") line[1] = "CLT_OR"; 
		for(int i = 1; i < line.size(); i++)
		{
			line[i - 1] = line[i];
		}
	}

	if(line.size() > 1 and line[0] == "FAR" and line[1].size() > 1)
	{
		if(line[1] == "JMO") line[1] = "JMFAR"; else
		if(line[1] == "JMO.P") line[1] = "JMFAR.P"; else
		if(line[1] == "JLO") line[1] = "JLFAR"; else
		if(line[1] == "JLO.P") line[1] = "JLFAR.P";

		std::printf("Warning!\n");
	}

	if(line.size() > 1 and (line[0] == "FAR" or line[0] == "PRE") and line[1][0] != '#')
	{
		LINE prefix_line = { false, NONE, -127, -127, -127, PREF, -1024, current_address, -1, -(1 << 24), 0, "", 0};
		current_address += 2;
		for(int i = 0; i < line.size() - 1; i++)
		{
			line[i] = line[i+1];
		}
		/*if(pref1_mappings.contains(line[0]))
		{
			state = 0x01;
			result.STATE = 0x01;
			prefix_line.IMMEDIATE = 1;
		} else */
		if(pref7f_mappings.contains(line[0]))
		{
			state = 0x7f;
			result.STATE = 0x7f;
			prefix_line.IMMEDIATE = 0x7f;
		}
		else if(mappings.contains(line[0]))
		{
			state = 0x00;
			result.STATE = 0x00;
			prefix_line.IMMEDIATE = 0;
		}
		else
		{
			std::printf("Cannot assemble bare prefix for line [");
			for(auto& i : line) { std::printf("%s ", i.c_str()); }
			std::printf("] @ position 0x%06x!\n", current_address);
			throw std::invalid_argument(" PREFIX ERROR ");
		}
		out.push_back({prefix_line});
	}

	result.POSITION = current_address;

	std::string temp = line[0];

	if(temp[0] == '@')
	{
		result.OPERATION = LABEL; // pseudoinsn!
		result.LABELNAME = temp.substr(1, temp.size());
		label_addr[result.LABELNAME] = current_address;
		return result;
	}

	else if(temp[0] == '%')
	{
		result.OPERATION = LABEL; // pseudoinsn!
		result.LABELNAME = std::string("_sec_") + temp.substr(1, temp.size());
		label_addr[result.LABELNAME] = current_address;
		return result;
	}

	if(temp[0] == '.') // directives
	{

		std::unordered_map<std::string, DIRECTIVES> 
			asm_directives =
		{
			{"NUL", NUL},
			{"ORG", ORG },
			{"INT8", INT8 },
			{"INT16", INT16 },
			{"INT24", INT24 },
			{"FP24", FP24 },
			{"INT48", INT48 },
			{"FP48", FP48 },
			{"ASCII", ASCII },
			{"ASCIZ", ASCIZ },
			{"SECTION", SECTION },
			{"SEC", SECTION },
			{"PAD", PAD},
			{"REP", REP},
			{"ALIGN", ALIGN },
		};

		union
		{
			i64 s;
			u64 u;
		} dir_temp;
		dir_temp.u = 0;
		result.OPERATION = DIRECTIVE;
		
		switch(asm_directives[temp.substr(1, temp.size())])
		{
			case SECTION:
				result.DIREC = SECTION;
				if(line[1].size() > 0)
				{
					if(line[1][0] == '%')
						result.LABELNAME = std::string("_sec_") + line[1].substr(1, line[1].size());
					else
					{
						std::printf("Not a section name!\n");
						throw;
					}
				}
				current_address += 8;

				return result;

			case NUL:
				result.DIREC = NUL;
				result.RAW_DATA = 0x00;
				current_address += 0x1;
				return result;

			case ORG:
				result.DIREC = ORG;
				if(line[1].size() > 0)
					dir_temp.u = std::stoi(line[1], 0, 0);
				current_address = dir_temp.u & 0xffffff;
				return result;

			case INT8:
				result.DIREC = INT8;
				if(line[1].size() > 0)
					dir_temp.s = std::clamp(std::stoi(line[1], 0, 0), -128, 255);
				result.RAW_DATA = dir_temp.u & 0xff;
				current_address += 1;
				return result;

			case INT16:
				result.DIREC = INT16;
				if(line[1].size() > 0)
					dir_temp.s = std::clamp(std::stoi(line[1], 0, 0), -32768, 65535);
				result.RAW_DATA = dir_temp.u & 0xffff;
				current_address += 2;
				return result;
			
			case INT24:
				result.DIREC = INT24;
				if(line[1][0] == '@')
				{
					result.RAW_DATA = 0;
					result.LABELNAME = line[1].substr(1, line[1].size());
					current_address += 3;
					return result;
				}
				if(line[1].size() > 0)
					dir_temp.s = std::clamp(std::stoi(line[1], 0, 0), -8388608, 16777216);
				result.RAW_DATA = dir_temp.u & 0xffffff;
				current_address += 3;
				return result;

			case INT48:
				result.DIREC = INT48;
				if(line[1].size() > 0)
					dir_temp.u = std::clamp(std::stoll(line[1], 0, 0), -(1ll<<47), (1ll<<48) - 1);
				result.RAW_DATA = dir_temp.u & 0x0000'ffffff'ffffff;
				current_address += 6;
				return result;

			case REP:
				result.DIREC = REP;
				if(line[1].size() > 0)
					dir_temp.u  = std::bit_cast<u64>(std::stol(line[1], 0, 0)) << 32;
				if(line[2].size() > 0)
					dir_temp.u |= std::stoi(line[2], 0, 0);
				else
					dir_temp.u |= 0xff;
				current_address += std::bit_cast<u32>(std::stoi(line[1], 0, 0));
				result.RAW_DATA = dir_temp.u;
				return result;

			case FP24:
				result.DIREC = FP24;
				if(line[1].size() > 0)
					dir_temp.u = std::bit_cast<u32>(std::stof(line[1])) >> 8;
				result.RAW_DATA = dir_temp.u;
				current_address += 3;
				return result;
			
			case FP48:
				result.DIREC = FP48;
				if(line[1].size() > 0)
					dir_temp.u = std::bit_cast<u64>(std::stod(line[1])) >> 16;
				result.RAW_DATA = dir_temp.u;
				current_address += 6;
				return result;
				
			case ASCII:
			case ASCIZ:
				result.DIREC = asm_directives[temp.substr(1, temp.size())];
				if(line[1].size() > 0)
				{
					std::string tempstr;

					for(int i = 1; i < line.size(); i++)
						if(line[i].size())
							tempstr = tempstr + " " + line[i];

					tempstr = tempstr.substr(1, 33ul);
					if(result.DIREC == ASCIZ)
						tempstr = tempstr + '\0';

					std::string* ptr_str = new std::string(std::move(tempstr));

					result.RAW_DATA = std::bit_cast<u64>(ptr_str);

					current_address += ptr_str->size();
				}
				return result;

			case PAD:
				if(current_address & 0x1)
				{
					result.DIREC = NUL;
					result.RAW_DATA = 0x00;
					current_address += 0x1;
					return result;
				}
				else
				{
					result.DIREC = NONE;
					return result;
				}
			
			default:
				std::printf("Directive %s unknown at line @ 0x%06x\n",
					line[0].c_str(),
					result.POSITION
				);
				throw std::invalid_argument( " UNRECOGNISED DIRECTIVE " );
		}
	}

	if(temp.size() > 2 and temp.substr(temp.size() - 2, temp.size()) == ".P")
	{
		result.PREDICATED = true;
		temp = temp.substr(0, temp.size() - 2);
	}

	if(state and !temp.empty())
	{
		/*if(state == 0x01 and pref1_mappings.contains(temp))
		{
			// std::printf("try previous line?\n");
			if(prev_line.OPERATION != PREF and
				unmappings.contains(prev_line.OPERATION) and
				pref1_mappings.contains(unmappings.at(prev_line.OPERATION)))
				{ 
					// std::printf("it should now keep the prefix: ");
					prev_line.KEEP_PREFIX = true; 
					// std::printf("do we keep the prefix? [%d]\n", prev_line.KEEP_PREFIX);
				}
	
			result.OPERATION = pref1_mappings.at(temp);
			result.STATE = 1;
		} else */
		if(state == 0x7f and pref7f_mappings.contains(temp))
		{
			result.OPERATION = pref7f_mappings.at(temp);
			result.STATE = 0x7f;
		} else 
		if(mappings.contains(temp))
		{
			state = 0;
			// std::printf("we deleted the previous prefix!\n");
			goto default_parse;
		}
		else
		{
			std::printf("UNRECOGNISED mnemonic at location 0x%06x [%s] for state %d from line: ", current_address, temp.c_str(), state);
			for(auto& i : line) { std::printf("%s ", i.c_str()); }
			std::printf("\n");
			throw std::invalid_argument(" UNRECOGNISED MNEMONIC ");
		}

	}

	else if(!temp.empty() and mappings.contains(temp))
	{
		default_parse:
		result.OPERATION = mappings.at(temp);
	}

	else
	{
		std::printf("UNRECOGNISED mnemonic at location 0x%06x [%s] for state %d from line: ", current_address, temp.c_str(), state);
		for(auto& i : line) { std::printf("%s ", i.c_str()); }
		std::printf("\n");
		throw std::invalid_argument(" UNRECOGNISED MNEMONIC ");
	}

	for(int i = 1; i < line.size(); i++)
	{	
		if(!line[i].empty())
			temp = line[i];

		if(temp[0] == '@' and (temp.size() > 1))
		{
			result.LABELNAME = temp.substr(1, temp.size());
			current_address += 2;
			return result;
		}
	
		if(temp[0] == '-' and temp[1] >= 'A' and temp[1] <= 'H') // predec
		{
			switch(result.OPERATION)
			{
				case STRB:
					result.OPERATION = STRBBD; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; temp[0] = temp[1]; } break;
				case STRW:
					result.OPERATION = STRWBD; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; temp[0] = temp[1]; } break;
				case STRS:
					result.OPERATION = STRSBD; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; temp[0] = temp[1]; } break;
				case LDRB:
					result.OPERATION = LDRBBD; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; temp[0] = temp[1]; } break;
				case LDRW:
					result.OPERATION = LDRWBD; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; temp[0] = temp[1]; } break;
				case LDRS:
					result.OPERATION = LDRSBD; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; temp[0] = temp[1]; } break;
				case ST2W:
					result.OPERATION = ST2WBD; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; temp[0] = temp[1]; } break;
				case ST2S:
					result.OPERATION = ST2SBD; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; temp[0] = temp[1]; } break;
				case ST3S:
					result.OPERATION = ST3SBD; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; temp[0] = temp[1]; } break;
				case LD2W:
					result.OPERATION = LD2WBD; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; temp[0] = temp[1]; } break;
				case LD2S:
					result.OPERATION = LD2SBD; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; temp[0] = temp[1]; } break;
				case LD3S:
					result.OPERATION = LD3SBD; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; temp[0] = temp[1]; } break;
				default: break;
			}
			temp[0] = temp[1];
		}
		if(temp[0] == '+' and temp[1] >= 'A' and temp[1] <= 'H') // preinc
		{
			switch(result.OPERATION)
			{
				case STRB:
					result.OPERATION = STRBBI; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; temp[0] = temp[1]; } break;
				case STRW:
					result.OPERATION = STRWBI; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; temp[0] = temp[1]; } break;
				case STRS:
					result.OPERATION = STRSBI; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; temp[0] = temp[1]; } break;
				case LDRB:
					result.OPERATION = LDRBBI; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; temp[0] = temp[1]; } break;
				case LDRW:
					result.OPERATION = LDRWBI; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; temp[0] = temp[1]; } break;
				case LDRS:
					result.OPERATION = LDRSBI; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; temp[0] = temp[1]; } break;
				case ST2W:
					result.OPERATION = ST2WBI; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; temp[0] = temp[1]; } break;
				case ST2S:
					result.OPERATION = ST2SBI; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; temp[0] = temp[1]; } break;
				case ST3S:
					result.OPERATION = ST3SBI; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; temp[0] = temp[1]; } break;
				case LD2W:
					result.OPERATION = LD2WBI; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; temp[0] = temp[1]; } break;
				case LD2S:
					result.OPERATION = LD2SBI; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; temp[0] = temp[1]; } break;
				case LD3S:
					result.OPERATION = LD3SBI; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; temp[0] = temp[1]; } break;
				default: break;
			}
			temp[0] = temp[1];
		}
		if(temp[1] == '-' and temp[0] >= 'A' and temp[0] <= 'H') // postdec
		{
			temp[1] = '\0';
			switch(result.OPERATION)
			{
				case STRB:
					result.OPERATION = STRBAD; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[0] - 'A'; } break;
				case STRW:
					result.OPERATION = STRWAD; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[0] - 'A'; } break;
				case STRS:
					result.OPERATION = STRSAD; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[0] - 'A'; } break;
				case LDRB:
					result.OPERATION = LDRBAD; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[0] - 'A'; } break;
				case LDRW:
					result.OPERATION = LDRWAD; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[0] - 'A'; } break;
				case LDRS:
					result.OPERATION = LDRSAD; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[0] - 'A'; } break;
				case ST2W:
					result.OPERATION = ST2WAD; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; } break;
				case ST2S:
					result.OPERATION = ST2SAD; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; } break;
				case ST3S:
					result.OPERATION = ST3SAD; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; } break;
				case LD2W:
					result.OPERATION = LD2WAD; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; } break;
				case LD2S:
					result.OPERATION = LD2SAD; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; } break;
				case LD3S:
					result.OPERATION = LD3SAD; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; } break;
				default: break;
			}
		}
		if(temp[1] == '+' and temp[0] >= 'A' and temp[0] <= 'H') // postinc
		{
			temp[1] = '\0';
			switch(result.OPERATION)
			{
				case STRB:
					result.OPERATION = STRBAI; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[0] - 'A'; } break;
				case STRW:
					result.OPERATION = STRWAI; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[0] - 'A'; } break;
				case STRS:
					result.OPERATION = STRSAI; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[0] - 'A'; } break;
				case LDRB:
					result.OPERATION = LDRBAI; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[0] - 'A'; } break;
				case LDRW:
					result.OPERATION = LDRWAI; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[0] - 'A'; } break;
				case LDRS:
					result.OPERATION = LDRSAI; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[0] - 'A'; } break;
				case ST2W:
					result.OPERATION = ST2WAI; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; } break;
				case ST2S:
					result.OPERATION = ST2SAI; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; } break;
				case ST3S:
					result.OPERATION = ST3SAI; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; } break;
				case LD2W:
					result.OPERATION = LD2WAI; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; } break;
				case LD2S:
					result.OPERATION = LD2SAI; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; } break;
				case LD3S:
					result.OPERATION = LD3SAI; if(result.REG_A  < 0) { throw; } else { result.REG_B = temp[1] - 'A'; } break;
				default: break;
			}
		}

		if(temp[0] == '#')
		{
			result.IMMEDIATE = std::stoi(temp.substr(1), 0, 0);
			current_address += 2;
			return result;
		}
		if(temp[0] == '\'')
		{
			switch(temp[1])
			{
				case '\\':
					switch(temp[2])
					{
						case 'n' : result.IMMEDIATE = 0x0a; break;
						case '0' : result.IMMEDIATE = 0x00; break;
						case '\'': result.IMMEDIATE = 0x27; break; // '
					}
					break;
				default: 
					result.IMMEDIATE = std::bit_cast<u8>(temp[1]);
					break;
			}
			
			current_address += 2;
			return result;
		}

		if(temp[0] >= 'A' and temp[0] <= 'H')
		{
			if(result.REG_A < 0)
				result.REG_A = temp[0] - 'A';
			else
			if(result.REG_B  < 0)
				result.REG_B = temp[0] - 'A';
			else
			if(result.REG_C  < 0 and result.OPERATION == FMADD_48)
				result.REG_C = temp[0] - 'A';
		}
		else
		{
			std::printf("UNKNOWN register! %s from line: ", temp.c_str());
			for(auto& i : line) { std::printf("%s ", i.c_str()); }
			std::printf("\n");
			throw std::invalid_argument(" UNRECOGNISED REGISTER ");
		}	
	}
	current_address += 2;
	return result;
}

void DISAMBIGUATE(std::vector<LINE>& lines)
{
	bool problem = false;

	for(auto& i : lines)
	{
		switch(i.OPERATION)
		{
			case MVH:
			case MVM:
			case MVL:
			default: break;
		}
		switch(i.OPERATION)
		{
			case MVH:
				{
					switch(i.REG_A + 'A')
					{
						case 'A': i.OPERATION = MVHA; break;
						case 'B': i.OPERATION = MVHB; break;
						case 'C': i.OPERATION = MVHC; break;
						case 'D': i.OPERATION = MVHD; break;
						case 'E': i.OPERATION = MVHE; break;
						case 'F': i.OPERATION = MVHF; break;
						case 'G': i.OPERATION = MVHG; break;
						case 'H': i.OPERATION = MVHH; break;

						default: problem = true; break;
					}
					i.REG_A = -127;
				}
				break;
			case MVM:
				{
					switch(i.REG_A + 'A')
					{
						case 'A': i.OPERATION = MVMA; break;
						case 'B': i.OPERATION = MVMB; break;
						case 'C': i.OPERATION = MVMC; break;
						case 'D': i.OPERATION = MVMD; break;
						case 'E': i.OPERATION = MVME; break;
						case 'F': i.OPERATION = MVMF; break;
						case 'G': i.OPERATION = MVMG; break;
						case 'H': i.OPERATION = MVMH; break;

						default: problem = true; break;
					}
					i.REG_A = -127;
				}
				break;
			case MVL:
				{
					switch(i.REG_A + 'A')
					{
						case 'A': i.OPERATION = MVLA; break;
						case 'B': i.OPERATION = MVLB; break;
						case 'C': i.OPERATION = MVLC; break;
						case 'D': i.OPERATION = MVLD; break;
						case 'E': i.OPERATION = MVLE; break;
						case 'F': i.OPERATION = MVLF; break;
						case 'G': i.OPERATION = MVLG; break;
						case 'H': i.OPERATION = MVLH; break;

						default: problem = true; break;
					}
					i.REG_A = -127;
				}
				break;
			case ADD:	if((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) i.OPERATION = ADDI;	 break;
			case SUB:	if((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) i.OPERATION = SUBI;	 break;
			case LSHL:	if((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) i.OPERATION = LSHLI;	 break;
			case LSHR:	if((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) i.OPERATION = LSHRI;	 break;
			case SET:	if((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) i.OPERATION = SETI;	 break;
			case CLR:	if((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) i.OPERATION = CLRI;	 break;
			case CBIT:	if((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) i.OPERATION = CBITI;	 break;
			case TGL:	if((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) i.OPERATION = TGLI;	 break;

			case CEQ_OR:if((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) i.OPERATION = CEQI_OR; break;
			case CNE_OR:if((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) i.OPERATION = CNEI_OR; break;
			case CGT_OR:if((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) i.OPERATION = CGTI_OR; break;
			case CLT_OR:if((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) i.OPERATION = CLTI_OR; break;
			
			case CEQ:	if((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) i.OPERATION = CEQI;	 break;
			case CNE:	if((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) i.OPERATION = CNEI;	 break;
			case CGT:	if((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) i.OPERATION = CGTI;	 break;
			case CLT:	if((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) i.OPERATION = CLTI;	 break;
			case COR:	if((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) i.OPERATION = CORI;	 break;
			case CAND:	if((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) i.OPERATION = CANDI;	 break;
			case BOR:	if((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) i.OPERATION = BORI;	 break;
			case BAND:	if((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) i.OPERATION = BANDI;	 break;
			
			case MOV:	if((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) i.OPERATION = MOVI;	 break;

			case STRB:	
				if(((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) or i.LABELNAME.size() > 1) 
					i.OPERATION = STRBI;
				break;
			case STRW:	
				if(((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) or i.LABELNAME.size() > 1) 
					i.OPERATION = STRWI;
				break;
			case STRS:	
				if(((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) or i.LABELNAME.size() > 1) 
					i.OPERATION = STRSI;
				break;
			case LDRB:	
				if(((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) or i.LABELNAME.size() > 1) 
					i.OPERATION = LDRBI;
				break;
			case LDRW:	
				if(((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) or i.LABELNAME.size() > 1) 
					i.OPERATION = LDRWI;
				break;
			case LDRS:	
				if(((i.IMMEDIATE != -1024) and (i.REG_B  < 0)) or i.LABELNAME.size() > 1) 
					i.OPERATION = LDRSI;
				break;

			default: continue;
		}
		if(problem)
		{
			std::printf("Can't disambiguate @ 0x%06x line: %s%s %c %c %s", 
				i.POSITION,
				unmappings[i.OPERATION].c_str(),
				(i.PREDICATED)?".P":"",
				(i.REG_A != -127)?(i.REG_A + 'A'):'\0',
				(i.REG_B != -127)?(i.REG_B + 'A'):'\0',
				std::to_string(i.IMMEDIATE).c_str()
				);
			throw std::invalid_argument(" CAN'T DISAMBIGUATE ");
		}
	}
	return;
}