#include <SDL2/SDL.h>

struct SDL_GFX
{
	SDL_Window  * window;
	SDL_Surface * canvas;
	SDL_Surface * window_surface;

	MMU * mmu;

	uint32_t base 	= 0;
	uint32_t size 	= 0;
	bool     scale  = true;

	uint16_t WINDOW_WIDTH  = 372;
	uint16_t WINDOW_HEIGHT = 264;

	SDL_GFX() = delete;
	SDL_GFX(CPU& cpu, uint32_t base_addr)
	{
		mmu  = cpu.LINKED_MMU;
		base = base_addr + 0x80; 

		//WINDOW_WIDTH  = cpu.GET_16(base_addr + 0);
		//WINDOW_HEIGHT = cpu.GET_16(base_addr + 2);
		// mode;
		// scale;

		window = SDL_CreateWindow
		(
			"Ennmulator",
			SDL_WINDOWPOS_UNDEFINED, 
		    SDL_WINDOWPOS_UNDEFINED,
		    WINDOW_WIDTH * 2,
		    WINDOW_HEIGHT* 2,
		    0
		);

		window_surface = SDL_GetWindowSurface(window);

		canvas = SDL_CreateRGBSurfaceWithFormat
		(
			0,
			WINDOW_WIDTH * 2,
			WINDOW_HEIGHT * 2,
			16,
			SDL_PIXELFORMAT_RGB565
		);

		size = WINDOW_WIDTH * WINDOW_HEIGHT;

		{
			SDL_LockSurface(canvas);
			uint16_t *buffer = (uint16_t*) canvas->pixels;

			for(int x = 0; x < WINDOW_WIDTH; x++)
				for(int y = 0; y < WINDOW_HEIGHT; y++)
					draw_scaled_px(x, y, buffer, 0x47e7);

			SDL_UnlockSurface(canvas);
			SDL_BlitSurface(canvas, 0, window_surface, 0);
			SDL_UpdateWindowSurface(window);
		}
		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	}

	~SDL_GFX()
	{
	    SDL_Quit();
	}

	inline void draw_scaled_px(uint16_t x, uint16_t y, uint16_t* buf, uint16_t col)
	{
		buf[(WINDOW_WIDTH * 2 * 2 * y) + (2 * x)] = col;
		buf[(WINDOW_WIDTH * 2 * 2 * y) + (2 * x+1)] = col;
		buf[(WINDOW_WIDTH * 2 * (2 * y+1)) + (2 * x)] = col;
		buf[(WINDOW_WIDTH * 2 * (2 * y+1)) + (2 * x+1)] = col;
	}

	void RENDER()
	{
		SDL_LockSurface(canvas);
		uint16_t *buffer = (uint16_t*) canvas->pixels;

		uint32_t i = base;

		for(int y = 0; y < WINDOW_HEIGHT; y++)
			for(int x = 0; x < WINDOW_WIDTH;)
			{
				uint64_t temp = mmu->READ_64(i, 0x00ff);
				draw_scaled_px(x, y, buffer, uint16_t(temp >> 48)); x++;
				draw_scaled_px(x, y, buffer, uint16_t(temp >> 32)); x++;
				draw_scaled_px(x, y, buffer, uint16_t(temp >> 16)); x++;
				draw_scaled_px(x, y, buffer, uint16_t(temp >>  0)); x++;

				i += 8;
			}
		
		mmu->WRITE_16(0x01'00'40, 0x00ff, 0xffff);

		SDL_UnlockSurface(canvas);
		SDL_BlitSurface(canvas, 0, window_surface, 0);
		SDL_UpdateWindowSurface(window);
	}
};