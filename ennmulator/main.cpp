#include <cstdio>
#include <bit>
#include <fstream>
#include <sstream>
#include <filesystem>

#include <chrono>
using namespace std::chrono_literals;

#include "types.hpp"
#include "random_module.hpp"


#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KPEE  "\x1B[93m"
#define KPOO  "\x1B[33m"
#define KWHT  "\x1B[37m"

void __DOUBLE_FRAMEBUFFER(std::vector<std::vector<u32>>&);

u32 TIMER(u32 INTERVAL, void* ARG)
{
	CPU* us = reinterpret_cast<CPU*>(ARG);
	if(us->XS) { return INTERVAL; }
	if(us->IS_MASKED_INT()) { return INTERVAL; }

	us->CLR_WFI();

	if(!us->IS_MASKED_INT())
	{	
		us->EXECUTE({
			KERNI,
			-1, -1,
			0x23,				// timer exception is int 0x23
			false,
			false
		});
	}
	return 0;
}

void __DEBUG_PRINT_STATE(CPU& cpu)
{
	std::printf(" ! ---\nSTATE:"
		"\n\tA: 0x%06x\t B: 0x%06x"
		"\n\tC: 0x%06x\t D: 0x%06x"
		"\n\tE: 0x%06x\t F: 0x%06x"
		"\n\tG: 0x%06x\t H: 0x%06x\n",
		cpu.ACTIVE_SET->at(0), cpu.ACTIVE_SET->at(1),
		cpu.ACTIVE_SET->at(2), cpu.ACTIVE_SET->at(3),
		cpu.ACTIVE_SET->at(4), cpu.ACTIVE_SET->at(5),
		cpu.ACTIVE_SET->at(6), cpu.ACTIVE_SET->at(7)
		);
	std::printf("-------------------\n");
	std::printf("\tIP: 0x%04x\tSP: 0x%04x\n\tPS: 0x%04x\tXS: 0x%04x\n",
		cpu.IP, cpu.SP, cpu.PS, cpu.XS );
	std::printf("\tFETCHED INSN: 0x%04x\n", cpu.FETCHED_INSN);
	std::printf("\tTICKS: %ld\n", cpu.TICKS);
}

int main(int argc, char** argv)
{
	CPU basic_cpu;

	if(argc > 2)
	{
		std::vector<u8> bytes;
		bytes.resize(2880 * 512);

		for(int i = 1; i < argc; i++)
		{
			if(std::string(argv[i]) == "-b")
			{
				std::filesystem::path inpath{argv[i + 1]};
				u32 length = std::filesystem::file_size(inpath);
				std::ifstream file(argv[i + 1], std::ios_base::binary);

				if(!file.is_open() or length == 0)
					throw("Failed to open .bin!");

				std::printf("binary length is %d\n", length);
				file.read(reinterpret_cast<char*>(bytes.data()), length);

				if(!basic_cpu.LOAD_NEW_FORMAT(bytes))
				{
					std::printf("Could not load .bin as program!\n\n");
					return 1;
				}

				file.close();
				i++;
				continue;
			} else
			if(std::string(argv[i]) == "-f")
			{
				DSK* new_dsk = new DSK;
				basic_cpu.DEVICES.push_back(new_dsk);
				std::filesystem::path inpath{argv[i + 1]};
				u32 length = std::filesystem::file_size(inpath);
				std::ifstream file(argv[i + 1], std::ios_base::binary);
				if(!file.is_open() or length == 0)
					throw("Failed to open .img!");
				std::printf("floppy length is %d\n", length);
				file.read(reinterpret_cast<char*>(bytes.data()), length);
				if(!new_dsk->LOAD_DSK(bytes))
				{
					std::printf("Could not load .img as floppy!\n\n");
					return 1;
				}
				file.close();
				i++;
				continue;
			}
			else
			{
				std::printf("Unknown option! [%s] not recognised.\n", argv[i]);
				return 1;
			}
		}
	}
	else
	{
		std::printf("Wrong usage (insufficient arguments)!\nYou must provide");
		std::printf(" the following options:");
		std::printf("\n\t-b [bios filename]   - name of the executable to load");
		std::printf("\n\t-f [floppy filename] - (optional) floppy image to load\n");
		return 1;
	}

	std::printf("\n        ");
	for(int i = 0; i < 32; i++)
		std::printf("%02x ", i);
	std::printf("\n        ");
	for(int i = 0; i < 32; i++)
		std::printf("---");
	for(int i = 0; i < 2048; i++)
	{
		u8 x = basic_cpu.GET_8(i);
		if((i % 32 == 0)) { std::printf(" |\n0x%04x: ", i); }
		std::printf("%s%02x%s ", (x!=0)?KPEE:KNRM, x, KNRM);
	}
	std::printf("\n\n");

	bool err = false;
	for(int i = 0; i < (1 << 24); i++)
		if(!basic_cpu.IS_WFI())
			for(int j = 0; j < 1024; j++)
				if(basic_cpu.STEP() == false)
					{ err = true; goto err1; }

	err1:;

	__DEBUG_PRINT_STATE(basic_cpu);

	std::printf("\n");

	return 0;
}
