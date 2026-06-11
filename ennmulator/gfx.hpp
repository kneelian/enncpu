#pragma once

#include <vector>

extern "C" {
	#include <tfblib/tfblib.h>
	#include <tfblib/tfb_colors.h>
}

#include "types.hpp"

#define u32 uint32_t
#define u16 uint16_t
#define  u8  uint8_t

struct GFX
{
	bool VALID = false;

	u16 WIDTH, HEIGHT;

	u16 FB_WIDTH, FB_HEIGHT;

	u8 SCALE = 1;

	u16 MODE = 64;

	std::vector<std::vector<uint32_t>> FB; // framebuffer

	GFX()
	{
		{
			int tfb_status;
			if((tfb_status = tfb_acquire_fb(0, NULL, NULL)) != TFB_SUCCESS)
			{
				std::printf("tfb_acquire_fb() failed! %s\n", tfb_strerror(tfb_status));
				return;	
			}
		}
		tfb_clear_screen(tfb_black);

		WIDTH  = tfb_screen_width();
		HEIGHT = tfb_screen_height();
	}

	~GFX()
	{
		tfb_release_fb();
	}

	inline void SETUP_FB(int x, int y, int scale = 1, int mode = 63)
	{
		MODE = mode;

		FB_WIDTH  = x;
		FB_HEIGHT = y;
		
		SCALE = scale;

		FB.resize(y * SCALE);
		for(auto & i : FB) { i.resize(x * SCALE); }
		for(auto & i : FB)
			for(auto & j : i) 
				{ j = 0x00; }
	}

	inline static void DRAW_RECT(int x, int y, int wx, int wy, uint32_t col)
	{ tfb_draw_rect(x, y, wx, wy, col); }
	inline static void DRAW_STRING(int x, int y, uint32_t fg, uint32_t bg, const char* str)
	{ tfb_draw_string(x, y, fg, bg, str); }
	inline static void DRAW_PX(int x, int y, uint32_t col)
	{ tfb_draw_pixel(x, y, col); }

	inline void PUT_PX_IN_FB(int x, int y, uint32_t col)
	{
		try { FB.at(y).at(x) = col; }
		catch ( ... ) { std::printf("couldn't put %d %d in fb!\n", x, y); }
	}

	inline void UPSCALE_FB()
	{
		if(SCALE == 1) { return; }
		for(int j = 0; j < FB[0].size() - 2; j+=2)
			for(int i = 0; i < FB.size() - 2; i+=2)
			{
				try{
					 u32 us   = FB.at(i+0).at(j+0);
					 u32 down = FB.at(i+2).at(j+0);
					 u32 rite = FB.at(i+0).at(j+2);

					 u8  usr = us >> 16;
					 u8  usg = us >>  8;
					 u8  usb = us >>  0;
					 
					 u8  dnr = down >> 16;
					 u8  dng = down >>  8;
					 u8  dnb = down >>  0;

					 u8  rtr = rite >> 16;
					 u8  rtg = rite >>  8;
					 u8  rtb = rite >>  0;

					 u16 amidr = (usr + dnr) >> 1;
					 u16 amidg = (usg + dng) >> 1;
					 u16 amidb = (usb + dnb) >> 1;

					 u32 amid = tfb_make_color(amidr, amidg, amidb);

					 u16 bmidr = (usr + rtr) >> 1;
					 u16 bmidg = (usg + rtg) >> 1;
					 u16 bmidb = (usb + rtb) >> 1;

					 u32 bmid = tfb_make_color(bmidr, bmidg, bmidb);
		
					 FB.at(i+1).at(j+0) = amid;
					 FB.at(i+0).at(j+1) = bmid;

					 u16 dgr = (amidr + bmidr) >> 1;
					 u16 dgg = (amidg + bmidg) >> 1;
					 u16 dgb = (amidb + bmidb) >> 1;

					 u32 diag = tfb_make_color(dgr, dgg, dgb);

					 FB.at(i+1).at(j+1) = diag;
					 
				} catch( ... ) { break; }
			}
		return;
	}

	void LOAD_FB(CPU& cpu, u32 baseaddr)
	{
		size_t wy = FB_HEIGHT;
		size_t wx = FB_WIDTH;
		if(MODE == 63)
		{
			for(int i = 0; i < wy; i++)
				for(int j = 0; j < wx; j++)
				{
					u8  cx = cpu.GET_8(baseaddr + (i * (wx + 1)) + j);
					u32 cl = tfb_make_color_hsv((i % 360) * TFB_HUE_DEGREE, 128, (256 * cx)/64 );
					try{
						FB.at(SCALE*i).at(SCALE*j) = cl;
					} catch(...) { break; }
				}
		}
		else
		{
			if(MODE == 1)
			{
				for(u32 i = 0; i < (2 * wx * wy); i+=2)
				{
					u16 cx   = cpu.GET_16(baseaddr + i);
					u32 b   = ((cx & 0b00000'000000'11111) >>  0) << 2;  // 0000'0000'0001'1111
					u32 g   = ((cx & 0b00000'111111'00000) >>  0) << 5;  // 0000'0111'1110'0000
					u32 r   = ((cx & 0b11111'000000'00000) >>  0) << 8; // 1111'1000'0000'0000

					u32 cl  = r | g | b;

					if(false) 
					{ 
						//std::printf("%02x %02x %0x --> %06x\n", r, g, b, cl); 
						tfb_draw_string(900, 64 - 16, tfb_gray,  tfb_black, std::to_string(cx).c_str());
						tfb_draw_string(900, 64 +  0, tfb_white, tfb_black, std::to_string(cl).c_str());
						tfb_draw_string(900, 64 + 16, tfb_red,   tfb_black, std::to_string(r).c_str());
						tfb_draw_string(900, 64 + 32, tfb_green, tfb_black, std::to_string(g).c_str());
						tfb_draw_string(900, 64 + 48, tfb_blue,  tfb_black, std::to_string(b).c_str());
					}
					try{
						FB.at(SCALE*((i / 2) / wx)).at(SCALE*((i / 2) % wx)) = cl;
					} 
					catch(...) 
					{	
						std::printf("could not put pixel in mode 1 at %d %d!\n", int(i % wx), int(i / wx));
						break;
					}
				}
			}
		}
	}

	inline void SHOW_FB(int xoff, int yoff)
	{
		for(int i = 0; i < FB.size(); i++)
			for(int j = 0; j < FB.at(i).size(); j++)
				try
				{
					tfb_draw_pixel(xoff + j, xoff + i, FB.at(i).at(j));
				}
				catch (...)
				{
					std::printf("Failed to draw pixel %d x %d in FB!\n", j, i);
				}
	}
};
