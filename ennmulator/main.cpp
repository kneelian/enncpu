
#include <cstdio>
#include <bit>
#include <fstream>
#include <sstream>
#include <filesystem>

#include<thread>

#include "types.hpp"
#include "random_module.hpp"

#include "raylib-cpp.hpp"

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KPEE  "\x1B[93m"
#define KPOO  "\x1B[33m"
#define KWHT  "\x1B[37m"

void __DEBUG_PRINT_STATE(CPU& cpu)
{
	std::printf(" ! ---\nSTATE:"
		"\n\tA: 0x%06x\t B: 0x%06x"
		"\n\tC: 0x%06x\t D: 0x%06x"
		"\n\tE: 0x%06x\t F: 0x%06x"
		"\n\tG: 0x%06x\t H: 0x%06x\n",
		cpu.ACTIVE_SET.at(0), cpu.ACTIVE_SET.at(1),
		cpu.ACTIVE_SET.at(2), cpu.ACTIVE_SET.at(3),
		cpu.ACTIVE_SET.at(4), cpu.ACTIVE_SET.at(5),
		cpu.ACTIVE_SET.at(6), cpu.ACTIVE_SET.at(7)
		);
	std::printf("-------------------\n");
	std::printf("\tIP: 0x%04x\tSP: 0x%04x\n\tPS: 0x%04x\tXS: 0x%04x\n",
		cpu.IP, cpu.SP, cpu.PS, cpu.XS );
	std::printf("\tFETCHED INSN: 0x%04x\n", cpu.FETCHED_INSN);
	std::printf("\tTICKS: %ld\n", cpu.TICKS);
}

void VM(CPU& us)
{
	for(u64 i = 0; i < (1ULL << 48); i++)
		if(us.STEP() == false)
			break;

	__DEBUG_PRINT_STATE(us);

	std::printf("\n");
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

	int screenWidth = 650;
    int screenHeight = 400;

    SetTraceLogLevel(LOG_NONE);
    raylib::Window w(screenWidth, screenHeight, "Raylib C++", 0, LOG_NONE);
    SetTraceLogLevel(LOG_NONE);
    
    w.SetTargetFPS(30);

    raylib::Shader s = LoadShader(0, TextFormat("crst.fs", 330));

    Image img = GenImageColor(650, 400, WHITE);
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R5G6B5);
    img.data = basic_cpu.LINKED_MMU->DATA.data() + 0x80'00'00;
    Texture2D tex;

    u8* MMIO_BASE = basic_cpu.LINKED_MMU->DATA.data() + 0x87'F0'00;
    u8* FLPY_BASE = basic_cpu.LINKED_MMU->DATA.data() + 0x87'F0'00;
    u8* KEYB_BASE = basic_cpu.LINKED_MMU->DATA.data() + 0x87'F4'00;
    u8* MOUS_BASE = basic_cpu.LINKED_MMU->DATA.data() + 0x87'F4'10;

    RenderTexture2D target;
    target = LoadRenderTexture(screenWidth, screenHeight);

    std::thread t (VM, std::ref(basic_cpu));

    t.detach();

    while (!w.ShouldClose()) // Detect window close button or ESC key
    {
        float time32 = float(GetTime());
        SetShaderValue(s, GetShaderLocation(s, "time"), &time32, RL_SHADER_UNIFORM_FLOAT);

        tex = LoadTextureFromImage(img);

        BeginTextureMode(target);
            ClearBackground(RAYWHITE);
            DrawTexture(tex, 0, 0, WHITE);
        EndTextureMode();

        BeginDrawing();
            BeginShaderMode(s);
                DrawTextureRec(target.texture, (Rectangle){ 0, 0, (float)target.texture.width, (float)-target.texture.height }, (Vector2){ 0, 0 }, WHITE);
            EndShaderMode();
        EndDrawing();

        UnloadTexture(tex);
    
        int key = GetCharPressed();
        if(key)
        {
        	KEYB_BASE[0] = u8(key);
        	basic_cpu.TRAP(0x00'04);
        }
    }

	return 0;
}
