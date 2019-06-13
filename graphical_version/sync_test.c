/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   main.c                                           .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/30 09:02:45 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/06/13 15:56:05 by mhouppin    ###    #+. /#+    ###.fr     */
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

static void		put_file_contents(const char *file, const void *content, uint32_t length)
{
	const int		fd = open(file, O_WRONLY | O_APPEND | O_CREAT, 0664);

	if (fd == -1)
	{
		fprintf(stderr, "can't open file %s\n", file);
		exit(1);
	}
	if (write(fd, content, length) != length)
	{
		fprintf(stderr, "save failure\n");
		exit(1);
	}
	close(fd);
}

extern void		plog2(const char *s, uint32_t size)
{
	put_file_contents("log_gbmul", s, size);
}

extern void		plog(const char *s)
{
	put_file_contents("log_gbmul", s, strlen(s));
}
/*
uint16_t	get_joypad_event(void)
{
	unsigned long	key = 0;
	unsigned int	button;
	unsigned int	arrows;

	read(0, &key, sizeof(unsigned long));
	if (key == K_ESC)
		exit(0);

	if ((g_memmap.complete_block[P1] & 0x30) == 0x30) {
		g_memmap.complete_block[P1] = 0;
		return (0);
	}
	if ((g_memmap.complete_block[P1] & 0x30) == 0)
		return (0);

	if (key)
	{
		if (g_memmap.complete_block[P1] & 0x10)
		{
			button = 0xff;
			switch (key)
			{
				case A:			button = ~0x01; break;
				case B:			button = ~0x02; break;
				case SELECT:	button = ~0x04; break;
				case START:		button = ~0x08; break;
			}
			g_memmap.complete_block[0xff00] &= button;
			IF_REGISTER |= 0x10;
			return (JOYPAD_INT);
		}
		else if (g_memmap.complete_block[P1] & 0x20)
		{
			arrows = 0xff;
			switch (key)
			{
				case K_RIGHT:	arrows = ~0x01; break;
				case K_LEFT:	arrows = ~0x02; break;
				case K_UP:		arrows = ~0x04; break;
				case K_DOWN:	arrows = ~0x08; break;
			}		
			g_memmap.complete_block[0xff00] &= arrows;
			IF_REGISTER |= 0x10;
			return (JOYPAD_INT);
		}
	}
	return (0);
}
*/


#define	CALL_INTERRUPT(interrupt, regs)	\
do\
{\
	g_memmap.ime = 0;\
	*GET_REAL_ADDR(regs->reg_sp) = regs->reg_pc;\
	regs->reg_pc = interrupt;\
}\
while (0)

void			call_interrupt(registers_t *regs, uint16_t interrupt, uint8_t reset)
{
	// joypad is set before

	// lcdc
	if (interrupt == 0 && (IF_REGISTER & 0x02))
		interrupt = LCDC_INT;

	// vblank
	if (interrupt == 0 && (IF_REGISTER & 0x01))
		interrupt = VBLANK_INT;

	// tima
	if (interrupt == 0 && (IF_REGISTER & 0x04))
		interrupt = TIMAOVF_INT;

	if (interrupt) {
		//plog("INTERRUPT\n");
		IF_REGISTER &= reset;
		CALL_INTERRUPT(interrupt, regs);
	}
	// faire tous les calls dans l'ordre ? (si oui, modifier l'instruction ret)
}

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

typedef struct	object_s
{
	uint8_t		lcd_y;
	uint8_t		lcd_x;
	uint8_t		code;
	uint8_t		attrib;
}				object_t;

typedef struct	oam_s
{
	object_t	obj[40];
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
	for (size_t i = 0; i < 40; i++)
	{
		oam->obj[i].lcd_y =		g_memmap.complete_block[0xFE00u + i * 4];
		oam->obj[i].lcd_x =		g_memmap.complete_block[0xFE01u + i * 4];
		oam->obj[i].code =		g_memmap.complete_block[0xFE02u + i * 4];
		oam->obj[i].attrib =	g_memmap.complete_block[0xFE03u + i * 4];
	}
}

SDL_Window		*window;
SDL_Texture		*texture;
SDL_Renderer	*render;
int				*pixels;

void			draw_line(oam_t *oam, int line)
{
	if (oam->active)
	{
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
				}
				else if ((STAT_REGISTER & BIT_5) == BIT_5 &&
						(IE_REGISTER & BIT_1) == BIT_1)
				{
					IF_REGISTER |= BIT_1;
				}
			}
		}
	}
	if (line_render < 153)
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
	uint8_t						shift;

	DIV_REGISTER += (uint8_t)cycles;
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
		check_what_should_do_lcd(cycles / g_memmap.cpu_speed);
		check_if_timer_needs_to_be_incremented(cycles);
		check_if_dma(cycles);
	}
}

static void		start_game(void)
{
	window = SDL_CreateWindow(g_cart.game_title,
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 160, 144, 0);

	assert(window != NULL);

	render = SDL_CreateRenderer(window, -1, 0);

	assert(render != NULL);

	texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING, 160, 144);

	assert(texture != NULL);

	int pitch;
	SDL_LockTexture(texture, NULL, (void **)&pixels, &pitch);

	LY_REGISTER = 0;
	TMA_REGISTER = 0;
	TAC_REGISTER = 0;
	DMA_REGISTER = 0xffU;
	LCDC_REGISTER = 0; //0x80U;
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
