/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   main.c                                           .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/30 09:02:45 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/06/17 16:10:25 by mhouppin    ###    #+. /#+    ###.fr     */
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
#include <malloc/malloc.h>

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

#define OAM_READ_CYCLES			68
#define OAM_VRAM_READ_CYCLES	216
#define HZ_BLANK_CYCLES			177
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
		oam->obj[i].lcd_y =		g_memmap.complete_block[0xFE00u + i * 4];
		oam->obj[i].lcd_x =		g_memmap.complete_block[0xFE01u + i * 4];
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
	}
}

SDL_Window		*window;
SDL_Texture		*texture;
SDL_Renderer	*render;
int				*pixels;
uint32_t		ticks;
cycle_count_t	elapsed_cycles = 0;

void			draw_line(oam_t *oam, int line)
{
	if ((LCDC_REGISTER & BIT_7) != BIT_7)
		return ;

	uint8_t	offset = 40u;
	if ((LCDC_REGISTER & BIT_5) == BIT_5)
	{
		// Load window tiles

		uint16_t	address = ((LCDC_REGISTER & BIT_6) == BIT_6) ?
			0x400u : 0x0u;

		uint8_t align_wy = WY_REGISTER & ~(7);
		for (uint8_t x = 0; x < 32; x++)
		{
			oam->obj[offset + x].lcd_y = align_wy;
			oam->obj[offset + x].lcd_x = x << 3;

			oam->obj[offset + x].code =
				g_memmap.vram_banks[0][address + align_wy * 32 + x];
			oam->obj[offset + x].attrib =
				g_memmap.vram_banks[1][address + align_wy * 32 + x];

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

		uint8_t		align_scy = SCY_REGISTER & ~(7);
		for (uint8_t x = 0; x < 32; x++)
		{
			oam->obj[offset + x].lcd_y = align_scy;
			oam->obj[offset + x].lcd_x = x << 3;

			oam->obj[offset + x].code =
				g_memmap.vram_banks[0][address + align_scy * 32 + x];
			oam->obj[offset + x].attrib =
				g_memmap.vram_banks[1][address + align_scy * 32 + x];

			if ((oam->obj[offset + x].attrib & BIT_7) == BIT_7)
				oam->obj[offset + x].prior = BG_BG_PRIOR;
			else
				oam->obj[offset + x].prior = BG_OBJ_PRIOR;

			oam->obj[offset + x].type = BG_TILE;
			oam->obj[offset + x].next_prior = 0;
		}
		offset += 32;
	}
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
	static int				render_status = OAM_READ;

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
						if ((IE_REGISTER & BIT_0) == BIT_0)
						{
							IF_REGISTER |= BIT_0;
						}
					}
					SDL_UnlockTexture(texture);
					SDL_RenderCopy(render, texture, NULL, NULL);
					SDL_RenderPresent(render);
					int pitch;
					SDL_LockTexture(texture, NULL, (void **)&pixels, &pitch);
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
			lcd_cycles -= HZ_BLANK_CYCLES;
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
	if (div_cycles > 512)
	{
		div_cycles -= 512;
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
	if (g_cart.cgb_support_code == 0 && DMA_REGISTER < 0x80u)
	{
		// No ROM copy on DMG cartridges

		return ;
	}
	if (DMA_REGISTER < 0xE0u)
	{
		long	*src = (long *)(GET_REAL_ADDR(((uint16_t)DMA_REGISTER) << 8));
		long	*dst = (long *)(g_memmap.complete_block + 0xfe00);

		dst = __builtin_assume_aligned(dst, 32);
		src = __builtin_assume_aligned(src, 32);

		for (size_t i = 0; i < 0xA0u / sizeof(long); i++)
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
	printf("Elapsed cycles: %lu\n", elapsed_cycles);
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

void			auto_rst(registers_t *regs, uint16_t new_pc, uint8_t if_mask)
{
	uint8_t		*address;

	IF_REGISTER &= ~(if_mask);
	address = GET_REAL_ADDR(regs->reg_sp);
	regs->reg_sp -= 2;
	address[-1] = (uint8_t)(regs->reg_pc >> 8);
	address[-2] = (uint8_t)(regs->reg_pc);
	regs->reg_pc = new_pc;
}

static void		check_if_ime(registers_t *regs)
{
	if (g_memmap.ime == false)
		return ;
	if ((IF_REGISTER & BIT_0) == BIT_0)
		auto_rst(regs, 0x40, BIT_0);
	else if ((IF_REGISTER & BIT_1) == BIT_1)
		auto_rst(regs, 0x48, BIT_1);
	else if ((IF_REGISTER & BIT_2) == BIT_2)
		auto_rst(regs, 0x50, BIT_2);
	else if ((IF_REGISTER & BIT_4) == BIT_4)
		auto_rst(regs, 0x60, BIT_4);
}

static void		start_cpu_lcd_events(void)
{
	cycle_count_t	cycles;
	registers_t		registers;

	registers.reg_sp = 0xFFFEu;
	registers.reg_pc = 0x100u;
	while (1)
	{
		if (GAMEBOY == NORMAL_MODE)
			cycles = execute(&registers);
		else
			cycles = 4;
		elapsed_cycles += cycles;
		check_what_should_do_lcd(cycles / g_memmap.cpu_speed);
		check_if_timer_needs_to_be_incremented(cycles);
		check_if_events(cycles);
		check_if_dma(cycles);
		check_if_ime(&registers);
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
	bzero(pixels, sizeof(int) * 160 * 144);

	ticks = SDL_GetTicks();

	g_memmap.cpu_speed = 1;
	P1_REGISTER = 0;
	TIMA_REGISTER = 0;
	TAC_REGISTER = 0;
	IE_REGISTER = 0;
	LCDC_REGISTER = 0x83u;
	SCY_REGISTER = 0;
	SCX_REGISTER = 0;
	LYC_REGISTER = 0;
	LY_REGISTER = 0;
	WY_REGISTER = 0;
	WX_REGISTER = 0;
	DMA_REGISTER = 0xffU;
	start_cpu_lcd_events();
	//	joypad_control_loop();
	/*
	   if (g_memmap.save_name)
	   save_external_ram();
	   */
}

int		main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "%s \"cartridge path\"\n", argv[0]);
		return (1);
	}

	remove("log_gbmul");

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
