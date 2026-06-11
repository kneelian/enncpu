#include <array>
#include <cstdio>
#include <string>
#include <sstream>
#include <fstream>
#include <numeric>
#include <stdexcept>
#include <unordered_map>

#include "utility.hpp"
#include "operations.hpp"
#include "types.hpp"

int main(int argc, char** argv)
{
	if(argc < 2)
	{
		std::printf("This program requires you to give the filename of the .asm file to load\n");
		return 1;
	}

	std::ifstream file(argv[1]);

	if(!file.is_open())
	{
		std::printf("Failed to open file %s; exiting...\n", argv[1]);
		return 1;
	}

	u32 current_address = 0x0;
	const char REG_NAMES[] = "ABCDEFGH";
	std::unordered_map<std::string, u32> label_addresses;

	std::vector<std::vector<std::string>> broken_lines;
	std::vector<std::string> raw_lines;
	std::vector<LINE> parsed_lines;

	std::string temp_line;

	while(std::getline(file, temp_line))
		if(!temp_line.empty())
			raw_lines.push_back(temp_line);

	file.close();

	PREPROCESS(raw_lines);

	for(auto& i : raw_lines)
		BREAK_LINES(broken_lines, i);

	std::printf("-----------------------\n");

	for(auto& i : broken_lines)
	{
		for(auto& j : i)
			std::printf("%s ", j.c_str());
		std::printf("\n");
	}

	PARSE_LINES(
		broken_lines, 
		parsed_lines, 
		current_address, 
		label_addresses);

	DISAMBIGUATE(parsed_lines);
	RESOLVE_LABELS(parsed_lines, label_addresses);
	SANITY_CHECK(parsed_lines);

	std::printf("-----------------------\n");

	for(auto& i : parsed_lines)
	{
		if(i.OPERATION == LABEL)
		{
			std::printf("0x%06x: label @%s\n",
				i.POSITION, i.LABELNAME.c_str());
			continue;
		}
		if(i.OPERATION == DIRECTIVE)
		{
			std::unordered_map<DIRECTIVES, std::string> dirnames =
			{
				{ ASCII, "ASCII"},
				{ FP24, "FP24"  },
				{ FP48, "FP48"  },
				{ INT8, "INT8"  },
				{ INT16, "INT16"  },
				{ INT24, "INT24"  },
				{ INT48, "INT48"  },
				{ ORG, "ORG"  },
				{ NUL, "NUL"  },
				{ PAD, "PAD"  },
				{ SECTION, "SEC" },
			};
			std::printf("0x%06x:\t%s%s %lu\n",
				i.POSITION,
				"D .",
				dirnames[i.DIREC].c_str(),
				i.RAW_DATA
				);
		} else
		std::printf("0x%06x:\t%s%s %c %c %c %s\n",
			i.POSITION,
			unmappings[i.OPERATION].c_str(),
			(i.PREDICATED)?".P":"",
			(i.REG_A != -127)?i.REG_A+'A':'\0',
			(i.REG_B != -127)?i.REG_B+'A':'\0',
			(i.REG_C != -127)?i.REG_C+'A':'\0',
			(i.IMMEDIATE != -1024)?std::to_string(i.IMMEDIATE).c_str():"");
	}

	std::vector<u8> assembly;
	ASSEMBLE(parsed_lines, current_address, assembly);

	std::printf("assembly output:\n");

	for(auto& i : assembly)
		std::printf("%02x ", i);

	std::printf("\n");

	if(argc == 3)
	{
		std::ofstream file(argv[2], std::ios_base::out | std::ios_base::binary );
		if(!file.is_open())
		{
			std::printf("Failed to open file %s; exiting...\n", argv[2]);
			return 1;
		}
		for(auto& i : assembly)
			file << i;

		file << 0 << 0;

		file.close();
	}
	return 0;
}
