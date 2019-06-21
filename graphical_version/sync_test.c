/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   main.c                                           .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/30 09:02:45 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/06/21 10:32:54 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <signal.h>
#include <termios.h>
#include "SDL.h"
#include "test.h"
#include "memory_map.h"
#include "registers.h"
#include "execute.h"

#define TILE_TEST	(uint8_t*)"\x22\xc1\xff\xff\x48\x30\x24\x18\x00\xff\xff\xff\x0a\x04\x05\x02"
cartridge_t		g_cart;

uint8_t			*g_get_real_addr[16] = {NULL};
memory_map_t	g_memmap;
uint32_t		GAMEBOY = NORMAL_MODE;

/*
 * add STOP_MODE & HALT_MODE to execute.c
 * http://www.codeslinger.co.uk/pages/projects/gameboy/graphics.html
*/

#define OAM_READ		0
#define OAM_VRAM_READ	1
#define HZ_BLANK		2

int				log_file;

#define OAM_READ_CYCLES			80
#define OAM_VRAM_READ_CYCLES	192
#define HZ_BLANK_CYCLES			204
#define VT_BLANK_WAITING_CYCLES	2879

#define BG_TILE			1
#define WINDOW_TILE		2
#define OBJ_TILE		3

#define BG_BG_PRIOR		49152u
#define OBJ_OBJ_PRIOR	32768u
#define BG_OBJ_PRIOR	16384u
#define OBJ_BG_PRIOR	0u

typedef struct	object_s
{
	uint8_t		lcd_y;
	uint8_t		lcd_x;
	uint8_t		code;
	uint8_t		attrib;
	uint8_t		type;
	uint8_t		next_prior;
	uint16_t	prior;
}				object_t;

typedef struct	oam_s
{
	object_t	obj[104];
	_Bool		active;
}				oam_t;


static void		open_log_file(void)
{
	log_file = open("log_gbmul", O_WRONLY | O_TRUNC | O_CREAT, 0664);

	if (log_file == -1)
	{
		fprintf(stderr, "FUCK YOU !\n");
		exit(1);
	}
}

static void		close_log_file(void)
{
	close(log_file);
}

extern void		plog(const char *s)
{
	write(log_file, s, strlen(s));
}

void			load_oam(oam_t *oam, int line)
{
	if ((LCDC_REGISTER & BIT_1) != BIT_1)
	{
		oam->active = false;
		return ;
	}
	oam->active = true;
	_Bool cgb_mode = (g_cart.cgb_support_code == 0xC0u);
	for (size_t i = 0; i < 40; i++)
	{
		oam->obj[i].lcd_y =		g_memmap.complete_block[0xFE00u + i * 4] - 16;
		oam->obj[i].lcd_x =		g_memmap.complete_block[0xFE01u + i * 4] - 8;
		oam->obj[i].code =		g_memmap.complete_block[0xFE02u + i * 4];
		oam->obj[i].attrib =	g_memmap.complete_block[0xFE03u + i * 4];
		if ((oam->obj[i].attrib & BIT_7) == BIT_7)
			oam->obj[i].prior = OBJ_BG_PRIOR;
		else
			oam->obj[i].prior = OBJ_OBJ_PRIOR;
		oam->obj[i].type =		OBJ_TILE;
		if (!cgb_mode)
			oam->obj[i].prior += (255 - oam->obj[i].lcd_x);
		oam->obj[i].next_prior = (255 - oam->obj[i].code);
//		printf("Object %zx: y(%hhu), x(%hhu), code(0x%hhx), attrib(0x%hhx)\n",
//				i,
//				oam->obj[i].lcd_y,
//				oam->obj[i].lcd_x,
//				oam->obj[i].code,
//				oam->obj[i].attrib);
	}
}

SDL_Window		*window;
SDL_Texture		*texture;
SDL_Renderer	*render;
int				*pixels;
uint32_t		ticks;
cycle_count_t	elapsed_cycles = 0;

int				cmp_prior(const void *l, const void *r)
{
	const object_t	*left = l;
	const object_t	*right = r;

	if (left->prior - right->prior != 0)
		return (left->prior - right->prior);
	else if (left->next_prior - right->next_prior != 0)
		return (left->next_prior - right->next_prior);
	else
		return (left->type - right->type);
}

void			draw_cgb_line(object_t *obj, uint8_t size, uint8_t line)
{
}

void			draw_dmg_line(object_t *obj, uint8_t size, uint8_t line)
{
	int32_t	bgp[4];
	int32_t	obp[8];

	uint8_t	*vram = g_memmap.vram;

	bgp[0] = (int32_t)(BGP_REGISTER & (BIT_0 | BIT_1));
	bgp[1] = (int32_t)(BGP_REGISTER & (BIT_2 | BIT_3)) >> 2;
	bgp[2] = (int32_t)(BGP_REGISTER & (BIT_4 | BIT_5)) >> 4;
	bgp[3] = (int32_t)(BGP_REGISTER & (BIT_6 | BIT_7)) >> 6;
	obp[0] = (int32_t)(OBP0_REGISTER & (BIT_0 | BIT_1));
	obp[1] = (int32_t)(OBP0_REGISTER & (BIT_2 | BIT_3)) >> 2;
	obp[2] = (int32_t)(OBP0_REGISTER & (BIT_4 | BIT_5)) >> 4;
	obp[3] = (int32_t)(OBP0_REGISTER & (BIT_6 | BIT_7)) >> 6;
	obp[4] = (int32_t)(OBP1_REGISTER & (BIT_0 | BIT_1));
	obp[5] = (int32_t)(OBP1_REGISTER & (BIT_2 | BIT_3)) >> 2;
	obp[6] = (int32_t)(OBP1_REGISTER & (BIT_4 | BIT_5)) >> 4;
	obp[7] = (int32_t)(OBP1_REGISTER & (BIT_6 | BIT_7)) >> 6;

	for (size_t i = 0; i < 4; i++)
	{
		bgp[i] = 0xffffff - (bgp[i] * 0x555555);
	}
	for (size_t i = 0; i < 8; i++)
	{
		obp[i] = 0xffffff - (obp[i] * 0x555555);
	}

	for (uint8_t i = 0; i < size; i++)
	{
		if (obj[i].type == OBJ_TILE)
		{
			// Object tiles' handler

			uint8_t obj_ypos = obj[i].lcd_y;
			uint8_t	obj_xpos = obj[i].lcd_x;

			obj_ypos = line - obj_ypos;

			// Check if the object isn't displayed on this line
			if (obj_ypos > 7)
				continue;

			uint8_t attrib = obj[i].attrib;

			if ((attrib & BIT_6) == BIT_6)
				obj_ypos = 7 - obj_ypos;
			for (uint8_t x = 0; x < 8; x++)
			{
				uint8_t rx = obj_xpos;

				if ((attrib & BIT_5) == BIT_5)
					rx += x;
				else
					rx += 7 - x;

				if (rx < 160)
				{

					uint8_t	dot1 = (vram[(uint16_t)obj[i].code * 16 + (uint16_t)obj_ypos * 2] >> x) & 1;
					uint8_t	dot2 = (vram[(uint16_t)obj[i].code * 16 + (uint16_t)obj_ypos * 2 + 1] >> x) & 1;
					int32_t	pxl;
					if (dot1 + dot2 + dot2 != 0)
					{
						if ((attrib & BIT_4) == BIT_4)
							pxl = obp[4 + dot1 + dot2 + dot2];
						else
							pxl = obp[dot1 + dot2 + dot2];
						pixels[(uint16_t)rx + (uint16_t)line * 160] = pxl;
					}
				}
				if ((LCDC_REGISTER & BIT_2) == 0)
					continue ;

				rx = obj_xpos + 8;

				if ((attrib & BIT_5) == BIT_5)
					rx += x;
				else
					rx += 7 - x;

				if (rx < 160)
				{
					uint8_t dot1 = (vram[(uint16_t)obj[i].code * 16 + (uint16_t)obj_ypos * 2 + 16] >> x) & 1;
					uint8_t dot2 = (vram[(uint16_t)obj[i].code * 16 + (uint16_t)obj_ypos * 2 + 17] >> x) & 1;
					int32_t	pxl;
					if (dot1 + dot2 + dot2 != 0)
					{
						if ((attrib & BIT_4) == BIT_4)
							pxl = obp[4 + dot1 + dot2 + dot2];
						else
							pxl = obp[dot1 + dot2 + dot2];
						pixels[(uint16_t)rx + (uint16_t)line * 160] = pxl;
					}
				}
			}
		}
		else if (obj[i].type == WINDOW_TILE)
		{
			// Window tiles' handler
		}
		else
		{
			uint8_t obj_ypos = obj[i].lcd_y;
			uint8_t	obj_xpos = obj[i].lcd_x + SCX_REGISTER;

			obj_ypos -= (SCY_REGISTER + line) & ~(7);

			for (uint8_t x = 0; x < 8; x++)
			{
				uint8_t	rx = obj_xpos + 7 - x;
				if (rx >= 160)
					continue;

				uint8_t dot1, dot2;
				if ((LCDC_REGISTER & BIT_4) == BIT_4 || obj[i].code >= 0x80u)
				{
					dot1 = (vram[(uint16_t)obj[i].code * 16 + (uint16_t)obj_ypos * 2] >> x) & 1;
					dot2 = (vram[(uint16_t)obj[i].code * 16 + (uint16_t)obj_ypos * 2 + 1] >> x) & 1;
				}
				else
				{
					dot1 = (vram[(uint16_t)obj[i].code * 16 + 4096 + (uint16_t)obj_ypos * 2] >> x) & 1;
					dot2 = (vram[(uint16_t)obj[i].code * 16 + 4097 + (uint16_t)obj_ypos * 2] >> x) & 1;
				}
				int32_t pxl = bgp[dot1 + dot2 + dot2];
				pixels[(uint16_t)rx + (uint16_t)line * 160] = pxl;
			}
		}
	}
}

void			draw_line(oam_t *oam, int line)
{
	if ((LCDC_REGISTER & BIT_7) != BIT_7)
		return ;

	if (g_cart.cgb_support_code == 0x0u)
		VBK_REGISTER = 0;
	else
		VBK_REGISTER &= BIT_0;

	uint8_t	offset = (oam->active) ? 40 : 0;
	if ((LCDC_REGISTER & BIT_5) == BIT_5 && WY_REGISTER <= (uint8_t)line)
	{
		// Load window tiles (only if not off the window)

		uint16_t	address = ((LCDC_REGISTER & BIT_6) == BIT_6) ?
			0x400u : 0x0u;

		uint8_t align_wy = (WY_REGISTER + (uint8_t)line) & ~(7);
		for (uint8_t x = 0; x < 32; x++)
		{
			oam->obj[offset + x].lcd_y = (WY_REGISTER + (uint8_t)line);
			oam->obj[offset + x].lcd_x = x << 3;

			oam->obj[offset + x].code =
				g_memmap.vram_bg[0][address + (uint16_t)align_wy * 32 + x];

			if (g_cart.cgb_support_code == 0x0u)
			{
				oam->obj[offset + x].attrib = 0;
			}
			else
			{
				oam->obj[offset + x].attrib =
					g_memmap.vram_bg[1][address + align_wy * 32 + x];
			}

			if ((oam->obj[offset + x].attrib & BIT_7) == BIT_7)
				oam->obj[offset + x].prior = BG_BG_PRIOR;
			else
				oam->obj[offset + x].prior = BG_OBJ_PRIOR;

			oam->obj[offset + x].type = WINDOW_TILE;
			oam->obj[offset + x].next_prior = 0;
		}
		offset += 32;
	}
	if ((LCDC_REGISTER & BIT_0) == BIT_0 || g_cart.cgb_support_code == 0xC0u)
	{
		// Load BG tiles


		uint16_t	address = ((LCDC_REGISTER & BIT_3) == BIT_3) ?
			0x400u : 0x0u;

		uint8_t		align_scy = (SCY_REGISTER + (uint8_t)line) & ~(7);
		for (uint8_t x = 0; x < 32; x++)
		{
			oam->obj[offset + x].lcd_y = (SCY_REGISTER + (uint8_t)line);
			oam->obj[offset + x].lcd_x = x << 3;

			oam->obj[offset + x].code =
				g_memmap.vram_bg[0][address + (uint16_t)align_scy * 4 + x];

			if (g_cart.cgb_support_code == 0x0u)
			{
				oam->obj[offset + x].attrib = 0;
			}
			else
			{
				oam->obj[offset + x].attrib =
					g_memmap.vram_bg[1][address + (uint16_t)align_scy * 4 + x];
			}

			if ((oam->obj[offset + x].attrib & BIT_7) == BIT_7)
				oam->obj[offset + x].prior = BG_BG_PRIOR;
			else
				oam->obj[offset + x].prior = BG_OBJ_PRIOR;

			oam->obj[offset + x].type = BG_TILE;
			oam->obj[offset + x].next_prior = 0;
//			printf("BGtile %hhx: y(%hhu), x(%hhu), code(0x%hhx), attrib(0x%hhx)\n",
//					x,
//					oam->obj[offset + x].lcd_y,
//					oam->obj[offset + x].lcd_x,
//					oam->obj[offset + x].code,
//					oam->obj[offset + x].attrib);
		}
		offset += 32;
	}

	qsort((object_t *)oam->obj, offset, sizeof(object_t), &cmp_prior);

	if (g_cart.cgb_support_code == 0x0u)
		draw_dmg_line((object_t *)oam->obj, offset, (uint8_t)line);
	else
		draw_cgb_line((object_t *)oam->obj, offset, (uint8_t)line);
}

static void		lcd_function(int line, int type)
{
	static oam_t	oam;

	if (type == OAM_READ)
	{
		load_oam(&oam, line);
		LY_REGISTER = line;
		if (LYC_REGISTER == LY_REGISTER)
		{
			STAT_REGISTER |= BIT_2;
			if ((STAT_REGISTER & BIT_6) == BIT_6 && (IE_REGISTER & BIT_1) == BIT_1)
			{
				IF_REGISTER |= BIT_1;
			}
		}
		else
		{
			STAT_REGISTER &= ~(BIT_2);
		}
	}
	else
	{
		draw_line(&oam, line);
	}
}

static void		wait_some_time(void)
{
	uint32_t	cur_ticks;

	if (true)
	{
		cur_ticks = SDL_GetTicks();
		if (cur_ticks - ticks < 17)
			SDL_Delay(17 - (cur_ticks - ticks));
		ticks = SDL_GetTicks();
	}
}

static void		check_what_should_do_lcd(cycle_count_t cycles)
{
	static cycle_count_t	lcd_cycles = 0;
	static int				line_render = 0;
	static int				render_status = HZ_BLANK;

	if ((LCDC_REGISTER & BIT_7) == 0 || GAMEBOY == STOP_MODE)
	{
		lcd_cycles = 0;
		line_render = 0;
		render_status = OAM_READ;
		return ;
	}
	lcd_cycles += cycles;
	if (line_render < 144)
	{
		if (render_status == OAM_READ)
		{
			if (lcd_cycles >= OAM_READ_CYCLES)
			{
				render_status = OAM_VRAM_READ;
				lcd_function(line_render, OAM_VRAM_READ);
				lcd_cycles -= OAM_READ_CYCLES;
				STAT_REGISTER &= ~(BIT_0 | BIT_1);
				STAT_REGISTER |= (BIT_0 | BIT_1);
			}
		}
		if (render_status == OAM_VRAM_READ)
		{
			if (lcd_cycles >= OAM_VRAM_READ_CYCLES)
			{
				render_status = HZ_BLANK;
				lcd_cycles -= OAM_VRAM_READ_CYCLES;
				if ((STAT_REGISTER & BIT_3) == BIT_3 &&
						(IE_REGISTER & BIT_1) == BIT_1)
				{
					IF_REGISTER |= BIT_1;
				}
				STAT_REGISTER &= ~(BIT_0 | BIT_1);
			}
		}
		if (render_status == HZ_BLANK)
		{
			if (lcd_cycles >= HZ_BLANK_CYCLES)
			{
				line_render++;
				STAT_REGISTER &= ~(BIT_0 | BIT_1);
				STAT_REGISTER |= (BIT_1);
				render_status = OAM_READ;
				lcd_function(line_render, OAM_READ);
				lcd_cycles -= HZ_BLANK_CYCLES;
				if (line_render == 144)
				{
					if ((STAT_REGISTER & BIT_4) == BIT_4)
					{
						if ((IE_REGISTER & BIT_1) == BIT_1)
						{
							IF_REGISTER |= BIT_1;
						}
					}
					if ((IE_REGISTER & BIT_0) == BIT_0)
					{
						IF_REGISTER |= BIT_0;
					}
					STAT_REGISTER &= ~(BIT_0 | BIT_1);
					STAT_REGISTER |= (BIT_0);
					SDL_UnlockTexture(texture);
					SDL_RenderCopy(render, texture, NULL, NULL);
					SDL_RenderPresent(render);
					int pitch;
					SDL_LockTexture(texture, NULL, (void **)&pixels, &pitch);
					for (size_t i = 0; i < 144 * 160; i++)
						pixels[i] = 0xffffff;
					wait_some_time();
				}
				else if ((STAT_REGISTER & BIT_5) == BIT_5 &&
						(IE_REGISTER & BIT_1) == BIT_1)
				{
					IF_REGISTER |= BIT_1;
				}
			}
		}
	}
	if (line_render >= 144 && line_render < 153)
	{
		if (lcd_cycles >= HZ_BLANK_CYCLES)
		{
			line_render++;
			LY_REGISTER = line_render;
			lcd_cycles -= HZ_BLANK_CYCLES;
			if (LYC_REGISTER == LY_REGISTER)
			{
				STAT_REGISTER |= BIT_2;
				if ((STAT_REGISTER & BIT_6) == BIT_6 && (IE_REGISTER & BIT_1) == BIT_1)
				{
					IF_REGISTER |= BIT_1;
				}
			}
			else
			{
				STAT_REGISTER &= ~(BIT_2);
			}
		}
	}
	if (line_render == 153)
	{
		if (lcd_cycles >= VT_BLANK_WAITING_CYCLES)
		{
			line_render = 0;
			render_status = OAM_READ;
			lcd_function(0, OAM_READ);
			lcd_cycles -= VT_BLANK_WAITING_CYCLES;
			if ((STAT_REGISTER & BIT_5) == BIT_5 && (IE_REGISTER & BIT_1) == BIT_1)
			{
				IF_REGISTER |= BIT_1;
			}
			STAT_REGISTER &= ~(BIT_0 | BIT_1);
			STAT_REGISTER |= (BIT_1);
		}
	}
}

static void		check_if_timer_needs_to_be_incremented(cycle_count_t cycles)
{
	static cycle_count_t		timer_cycles = 0;
	static cycle_count_t		div_cycles = 0;
	uint8_t						shift;

	div_cycles += cycles;
	if (div_cycles > 256)
	{
		div_cycles -= 256;
		DIV_REGISTER += 1;
	}
	if ((TAC_REGISTER & BIT_2) == BIT_2)
	{
		shift = TAC_REGISTER & (BIT_0 | BIT_1);
		if (shift == 0)
			timer_cycles += cycles * 1024;
		else
			timer_cycles += cycles * (1024 << (4 - shift));
		if (timer_cycles > 4194304)
		{
			TIMA_REGISTER += 1;
			if (TIMA_REGISTER == 0)
			{
				TIMA_REGISTER = TMA_REGISTER;
				if ((IE_REGISTER & BIT_2) == BIT_2)
					IF_REGISTER |= BIT_2;
			}
		}
	}
}

static void		check_if_dma(cycle_count_t cycles)
{
	if (DMA_REGISTER < 0xE0u)
	{
		uint8_t	*src = GET_REAL_ADDR(((uint16_t)DMA_REGISTER) << 8);
		uint8_t	*dst = g_memmap.complete_block + 0xfe00;

		dst = __builtin_assume_aligned(dst, 32);
		src = __builtin_assume_aligned(src, 32);

		for (size_t i = 0; i < 160; i++)
		{
			dst[i] = src[i];
		}
		DMA_REGISTER = 0xFFu;
	}
}

void __attribute__((noreturn))	quit_program(void)
{
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(render);
	SDL_DestroyWindow(window);
	SDL_Quit();
	close_log_file();
	printf("\nElapsed cycles: %lu\n", elapsed_cycles);
	exit(0);
}

int				my_event_filter(void *userdata, SDL_Event *ev)
{
	return (ev->type == SDL_QUIT || ev->type == SDL_KEYDOWN);
}

void			check_if_events(cycle_count_t cycles)
{
	static cycle_count_t	ev_cycles = 0;

	ev_cycles += cycles;
	if (ev_cycles > 10000)
	{
		ev_cycles -= 10000;
		SDL_Event	ev;

		while (SDL_PollEvent(&ev))
		{
			if (ev.type == SDL_QUIT)
				quit_program();
			if (ev.type == SDL_KEYDOWN)
			{
				switch (ev.key.keysym.sym)
				{
					case SDLK_ESCAPE:
						quit_program();
						break ;

					case SDLK_DOWN:
					case SDLK_s:
						if ((P1_REGISTER & BIT_4) == 0)
							P1_REGISTER &= ~(BIT_3);
						goto int_maybe;

					case SDLK_UP:
					case SDLK_w:
						if ((P1_REGISTER & BIT_4) == 0)
							P1_REGISTER &= ~(BIT_2);
						goto int_maybe;

					case SDLK_LEFT:
					case SDLK_a:
						if ((P1_REGISTER & BIT_4) == 0)
							P1_REGISTER &= ~(BIT_1);
						goto int_maybe;

					case SDLK_RIGHT:
					case SDLK_d:
						if ((P1_REGISTER & BIT_4) == 0)
							P1_REGISTER &= ~(BIT_0);
						goto int_maybe;

					case SDLK_KP_SPACE:
						if ((P1_REGISTER & BIT_5) == 0)
							P1_REGISTER &= ~(BIT_3);
						goto int_maybe;

					case SDLK_LALT:
					case SDLK_RALT:
						if ((P1_REGISTER & BIT_5) == 0)
							P1_REGISTER &= ~(BIT_2);
						goto int_maybe;

					case SDLK_BACKSPACE:
						if ((P1_REGISTER & BIT_5) == 0)
							P1_REGISTER &= ~(BIT_1);
						goto int_maybe;

					case SDLK_RETURN:
						if ((P1_REGISTER & BIT_5) == 0)
							P1_REGISTER &= ~(BIT_0);
						goto int_maybe;

					default:
						goto not_handled_input;
				}
int_maybe:
				if ((IE_REGISTER & BIT_4) == BIT_4)
					IF_REGISTER |= BIT_4;

not_handled_input:
				ev.key.keysym.sym = 0;
			}
		}
	}
}

void			auto_sri(registers_t *regs, uint16_t new_pc, uint8_t if_mask)
{
	uint8_t		*address;

	if (GAMEBOY != NORMAL_MODE)
	{
		GAMEBOY = NORMAL_MODE;
		return ;
	}
	g_memmap.ime = false;
	IF_REGISTER &= ~(if_mask);
	address = GET_REAL_ADDR(regs->reg_sp);
	regs->reg_sp -= 2;
	address[-1] = (uint8_t)(regs->reg_pc >> 8);
	address[-2] = (uint8_t)(regs->reg_pc);
	regs->reg_pc = new_pc;
}

static void		check_if_ime(registers_t *regs)
{
	if (g_memmap.ime == false && GAMEBOY == NORMAL_MODE)
		return ;
	if ((IF_REGISTER & BIT_0) == BIT_0 && GAMEBOY != STOP_MODE)
		auto_sri(regs, 0x40, BIT_0);
	else if ((IF_REGISTER & BIT_1) == BIT_1 && GAMEBOY != STOP_MODE)
		auto_sri(regs, 0x48, BIT_1);
	else if ((IF_REGISTER & BIT_2) == BIT_2 && GAMEBOY != STOP_MODE)
		auto_sri(regs, 0x50, BIT_2);
	else if ((IF_REGISTER & BIT_4) == BIT_4)
		auto_sri(regs, 0x60, BIT_4);
}

const uint8_t	bootstrap_code[256] =
"\x31\xfe\xff\xaf\x21\xff\x9f\x32\xcb\x7c\x20\xfb\x21\x26\xff\x0e"
"\x11\x3e\x80\x32\xe2\x0c\x3e\xf3\xe2\x32\x3e\x77\x77\x3e\xfc\xe0"
"\x47\x11\x04\x01\x21\x10\x80\x1a\xcd\x95\x00\xcd\x96\x00\x13\x7b"
"\xfe\x34\x20\xf3\x11\xd8\x00\x06\x08\x1a\x13\x22\x23\x05\x20\xf9"
"\x3e\x19\xea\x10\x99\x21\x2f\x99\x0e\x0c\x3d\x28\x08\x32\x0d\x20"
"\xf9\x2e\x0f\x18\xf3\x67\x3e\x64\x57\xe0\x42\x3e\x91\xe0\x40\x04"
"\x1e\x02\x0e\x0c\xf0\x44\xfe\x90\x20\xfa\x0d\x20\xf7\x1d\x20\xf2"
"\x0e\x13\x24\x7c\x1e\x83\xfe\x62\x28\x06\x1e\xc1\xfe\x64\x20\x06"
"\x7b\xe2\x0c\x3e\x87\xe2\xf0\x42\x90\xe0\x42\x15\x20\xd2\x05\x20"
"\x4f\x16\x20\x18\xcb\x4f\x06\x04\xc5\xcb\x11\x17\xc1\xcb\x11\x17"
"\x05\x20\xf5\x22\x23\x22\x23\xc9\xce\xed\x66\x66\xcc\x0d\x00\x0b"
"\x03\x73\x00\x83\x00\x0c\x00\x0d\x00\x08\x11\x1f\x88\x89\x00\x0e"
"\xdc\xcc\x6e\xe6\xdd\xdd\xd9\x99\xbb\xbb\x67\x63\x6e\x0e\xec\xcc"
"\xdd\xdc\x99\x9f\xbb\xb9\x33\x3e\x3c\x42\xb9\xa5\xb9\xa5\x42\x3c"
"\x21\x04\x01\x11\xa8\x00\x1a\x13\xbe\x20\xfe\x23\x7d\xfe\x34\x20"
"\xf5\x06\x19\x78\x86\x23\x05\x20\xfb\x86\x20\xfe\x3e\x01\xe0\x50";

uint8_t			bootstrap_overwrite[256];

static void		check_if_bootstrap(void)
{
	uint8_t	*address = GET_REAL_ADDR(0xFF50u);

	if (*address == 0x01)
	{
		*address = 0x0;
		memcpy(g_memmap.fixed_rom, bootstrap_overwrite, 256);
	}
}

#define TVALUE 40

static void		start_cpu_lcd_events(void)
{
	const char		*stable[3] = {" normal", " halted", "stopped"};
	cycle_count_t	tinsns = 0;
	cycle_count_t	cycles;
	registers_t		registers;

	registers.reg_sp = 0xFFFEu;
	registers.reg_pc = 0x100u;
	registers.reg_a = 0x01u;
	registers.reg_f = 0xB0u;
	registers.reg_b = 0x00u;
	registers.reg_c = 0x13u;
	registers.reg_d = 0x00u;
	registers.reg_e = 0xD8u;
	registers.reg_h = 0x01u;
	registers.reg_l = 0x4Du;

#if (_REG_DUMP == true)

	printf("\e[12B");

#endif

	while (1)
	{
		if (GAMEBOY == NORMAL_MODE)
		{

#if (_CPU_LOG == true)

			dprintf(log_file, "Instruction nº %lu\n", tinsns);
#endif

			cycles = execute(&registers);
		}
		else
			cycles = 4;
		tinsns++;
		elapsed_cycles += cycles;

#if (_REG_DUMP == true)

		if (tinsns % TVALUE == 0)
		{
			printf(	"\e[12A\rA  %4hhx F  %4hhx B  %4hhx C  %4hhx D  %4hhx E  %4hhx H  %4hhx L  %4hhx\n"
					"AF %4hx         BC %4hx         DE %4hx         HL %4hx\n"
					"PC %4hx         SP %4hx\n\nSTATUS %s IF %c%c%c%c%c IE %c%c%c%c%c IME %s TIMA %3hhu TMA %3hhu TAC %2hhx\n"
					"LCDC %c%c%c%c%c%c%c%c STAT %c%c%c%c%c%hhx\nINSTS %9lu\n", registers.reg_a, registers.reg_f,
					registers.reg_b, registers.reg_c, registers.reg_d, registers.reg_e,
					registers.reg_h, registers.reg_l, registers.reg_af, registers.reg_bc,
					registers.reg_de, registers.reg_hl, registers.reg_pc, registers.reg_sp,
					stable[GAMEBOY],
					(IF_REGISTER & BIT_4) ? 'J' : '.', (IF_REGISTER & BIT_3) ? 'S' : '.',
					(IF_REGISTER & BIT_2) ? 'T' : '.', (IF_REGISTER & BIT_1) ? 'L' : '.',
					(IF_REGISTER & BIT_0) ? 'V' : '.',
					(IE_REGISTER & BIT_4) ? 'J' : '.', (IE_REGISTER & BIT_3) ? 'S' : '.',
					(IE_REGISTER & BIT_2) ? 'T' : '.', (IE_REGISTER & BIT_1) ? 'L' : '.',
					(IE_REGISTER & BIT_0) ? 'V' : '.',
					g_memmap.ime ? " enabled" : "disabled",
					TIMA_REGISTER, TMA_REGISTER, TAC_REGISTER,
					(LCDC_REGISTER & BIT_7) ? 'L' : '.', (LCDC_REGISTER & BIT_6) ? '1' : '0',
					(LCDC_REGISTER & BIT_5) ? 'W' : '.', (LCDC_REGISTER & BIT_4) ? '1' : '0',
					(LCDC_REGISTER & BIT_3) ? '1' : '0', (LCDC_REGISTER & BIT_2) ? 'D' : 'S',
					(LCDC_REGISTER & BIT_1) ? 'O' : '.', (LCDC_REGISTER & BIT_0) ? 'B' : '.',
					(STAT_REGISTER & BIT_6) ? 'L' : '.', (STAT_REGISTER & BIT_5) ? 'O' : '.',
					(STAT_REGISTER & BIT_4) ? 'V' : '.', (STAT_REGISTER & BIT_3) ? 'H' : '.',
					(STAT_REGISTER & BIT_2) ? 'Y' : '.', (STAT_REGISTER & (BIT_0 | BIT_1)),
					tinsns);
			print_memory(g_memmap.complete_block + 0xFE00u, 160);
			fflush(stdout);
			usleep(15000);
		}

#endif

		check_what_should_do_lcd(cycles / g_memmap.cpu_speed);
		if (GAMEBOY != STOP_MODE)
			check_if_timer_needs_to_be_incremented(cycles);
		check_if_events(cycles);
		check_if_dma(cycles);
		check_if_ime(&registers);
		check_if_bootstrap();
	}
}

void		write_background_in_vram(void)
{
	uint8_t		*bg_data;
	uint8_t		*bg_chr;

	bg_data = GET_REAL_ADDR(0x9000);
	bg_chr = GET_REAL_ADDR(0x9800);

	memcpy(bg_data, TILE_TEST, 16);
	for (uint32_t i = 0; i < 0xFFu; i++)
	{
		bg_chr[i] = 0;
	}
}

static void		start_game(void)
{
	pthread_t	p;

	assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == 0);

	window = SDL_CreateWindow(g_cart.game_title,
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 160, 144,
			0);

	assert(window != NULL);

	render = SDL_CreateRenderer(window, -1, 0);

	assert(render != NULL);

	texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING, 160, 144);

	assert(texture != NULL);

	SDL_SetEventFilter(&my_event_filter, NULL);

	int pitch;
	assert(SDL_LockTexture(texture, NULL, (void **)&pixels, &pitch) == 0);
	for (size_t i = 0; i < 144 * 160; i++)
		pixels[i] = 0xffffff;

	ticks = SDL_GetTicks();

	g_memmap.complete_block[0xFF50] = 0;
	g_memmap.cpu_speed = 1;
	LCDC_REGISTER = 0x91u;
	OBP0_REGISTER = 0xFFu;
	OBP1_REGISTER = 0xFFu;
	BGP_REGISTER = 0xFCu;
	P1_REGISTER = 0xCFu;
	TIMA_REGISTER = 0;
	TMA_REGISTER = 0;
	TAC_REGISTER = 0xF8u;
	IF_REGISTER = 0xE1u;
	IE_REGISTER = 0;
	SCY_REGISTER = 0;
	SCX_REGISTER = 0;
	LYC_REGISTER = 0;
	LY_REGISTER = 0;
	WY_REGISTER = 0;
	WX_REGISTER = 0;
	DMA_REGISTER = 0xFFu;
	DIV_REGISTER = 0xD3u;
	g_memmap.cart_reg[0] = 0;
	g_memmap.cart_reg[1] = 0;
	g_memmap.cart_reg[2] = 0;
	g_memmap.cart_reg[3] = 0;
	g_memmap.cart_reg[4] = 0;
	g_memmap.cart_reg[5] = 0;
	g_memmap.cart_reg[6] = 0;
	g_memmap.cart_reg[7] = 0;
	g_memmap.ime = false;

//	memcpy((uint8_t *)bootstrap_overwrite, g_memmap.fixed_rom, 256);
//	memcpy(g_memmap.fixed_rom, (const uint8_t *)bootstrap_code, 256);

//	write_background_in_vram();
	start_cpu_lcd_events();
}

int		main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "%s \"cartridge path\"\n", argv[0]);
		return (1);
	}

//	remove("log_gbmul");
	open_log_file();

	open_cartridge(argv[1]);
	start_game();

	return (0);
}


/*
   static void		write_tile(char *screen, char *tile, size_t tile_size,
   unsigned int x, unsigned int y)
   {
   unsigned int		i = 0;

   if (tile_size == 64)
   {
   while (i < 16)
   {
   COPY_2_BYTES(screen, x + 0, ((i / 2) + y),
   color2[ ((tile[i] & 0x80) >> 7) | ((tile[i + 1] & 0x80) >> 6) ]);
   COPY_2_BYTES(screen, x + 1, ((i / 2) + y),
   color2[ ((tile[i] & 0x40) >> 6) | ((tile[i + 1] & 0x40) >> 5) ]);
   COPY_2_BYTES(screen, x + 2, ((i / 2) + y),
   color2[ ((tile[i] & 0x20) >> 5) | ((tile[i + 1] & 0x20) >> 4) ]);
   COPY_2_BYTES(screen, x + 3, ((i / 2) + y),
   color2[ ((tile[i] & 0x10) >> 4) | ((tile[i + 1] & 0x10) >> 3) ]);
   COPY_2_BYTES(screen, x + 4, ((i / 2) + y),
   color2[ ((tile[i] & 0x08) >> 3) | ((tile[i + 1] & 0x08) >> 2) ]);
   COPY_2_BYTES(screen, x + 5, ((i / 2) + y),
   color2[ ((tile[i] & 0x04) >> 2) | ((tile[i + 1] & 0x04) >> 1) ]);
   COPY_2_BYTES(screen, x + 6, ((i / 2) + y),
   color2[ ((tile[i] & 0x02) >> 1) | ((tile[i + 1] & 0x02) >> 0) ]);
   COPY_2_BYTES(screen, x + 7, ((i / 2) + y),
   color2[ ((tile[i] & 0x01) >> 0) | ((tile[i + 1] & 0x01) << 1) ]);

   i += 2;
   }
   }
   else
   {
   return ;
   }
   }
   */
