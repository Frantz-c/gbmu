/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   main.c                                           .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/30 09:02:45 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/06/05 13:23:35 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include <stdint.h>
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
#include "test.h"
#include "memory_map.h"
#include "registers.h"
#include "execute.h"
#include <malloc/malloc.h>


#define COL0		"\e[48;5;255m  "
#define COL1		"\e[48;5;249m  "
#define COL2		"\e[48;5;242m  "
#define COL3		"\e[48;5;235m  "


#define REAL_W		(160 * 13 + 1)
#define SCREEN_W	(160)
#define SCREEN_H	(144)
#define SCREEN_SIZE	(SCREEN_H * REAL_W + 13)
#define DEBUG_ZONE	(160 * 13 + 9)

// 18 * 20
#define TILE_OFFSET(x, y)	\
	(((y) * REAL_W * 8) + (13 * 8 * (x)))

// 144 * 160
#define SCREEN_OFFSET(x, y)	\
	(((y) * REAL_W) + (13 * (x)))


#define K_UP		0x415b1bUL
#define K_DOWN		0x425b1bUL
#define K_LEFT		0x445b1bUL
#define K_RIGHT		0x435b1bUL
#define K_ENTER		10UL
#define K_DEL		0x7e335b1bUL
#define K_BACKSP	127UL
#define K_ESC		27UL
#define K_TAB		9UL
#define K_SPACE		32UL

#define A			K_ENTER
#define B			K_BACKSP
#define START		K_SPACE
#define SELECT		's'

#define PIXEL		"  "

#define ON				1
#define OFF				0
#define EVENT_LOOP()	for (;;)
#define SCREEN_LOOP()	for (;;)
#define CPU_LOOP()		for (;;)

#if BYTE_ORDER == LITTLE_ENDIAN
	static const uint16_t	color[4] = {'5' | ('5' << 8), '4' | ('9' << 8), '4' | ('2' << 8), '3' | ('5' << 8)};
#else
	static const uint16_t	color[4] = {'5' | ('5' << 8), '9' | ('4' << 8), '2' | ('4' << 8), '5' | ('3' << 8)};
#endif

#define TRANSPARENT			('5' | ('5' << 8))

#define COPY_2_BYTES(screen, x, y, c)	\
	*(uint16_t*)(screen + ((y) * REAL_W) + (13 * (x)) + 8) = c;

#define COPY_2_BYTES_OBJECT(screen, x, y, c)	\
	if (c != TRANSPARENT) {\
		*(uint16_t*)(screen + ((y) * REAL_W) + (13 * (x)) + 8) = c;\
	}

/*
#define _REGISTER	g_memmap.complete_block[]
#define _REGISTER	g_memmap.complete_block[]
#define _REGISTER	g_memmap.complete_block[]
#define _REGISTER	g_memmap.complete_block[]
*/

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

static int		term_noecho_mode(int stat)
{
	static int				mode = -1;
	static struct termios	default_mode;
	static struct termios	noecho_mode;

	if (mode == -1)
	{
		if (tcgetattr(0, &default_mode) == -1)
			return (-1);
		noecho_mode = default_mode;
		noecho_mode.c_lflag &= ~(ECHO | ICANON);
		noecho_mode.c_cc[VMIN] = 0;
		noecho_mode.c_cc[VTIME] = 0;
		mode = 0;
	}
	if (mode == stat)
		return (0);
	if (stat == 1)
		return (tcsetattr(0, TCSANOW, &noecho_mode));
	if (stat == 0)
		return (tcsetattr(0, TCSANOW, &default_mode));
	return (-1);
}


static void		fill_8_pixels_obj(char *screen, uint32_t x, uint32_t y, char *tile)
{
	COPY_2_BYTES_OBJECT(screen, x + 0, y,
		color[ ((tile[0] & 0x80) >> 7) | ((tile[1] & 0x80) >> 6) ]);
	COPY_2_BYTES_OBJECT(screen, x + 1, y,
		color[ ((tile[0] & 0x40) >> 6) | ((tile[1] & 0x40) >> 5) ]);
	COPY_2_BYTES_OBJECT(screen, x + 2, y,
		color[ ((tile[0] & 0x20) >> 5) | ((tile[1] & 0x20) >> 4) ]);
	COPY_2_BYTES_OBJECT(screen, x + 3, y,
		color[ ((tile[0] & 0x10) >> 4) | ((tile[1] & 0x10) >> 3) ]);
	COPY_2_BYTES_OBJECT(screen, x + 4, y,
		color[ ((tile[0] & 0x08) >> 3) | ((tile[1] & 0x08) >> 2) ]);
	COPY_2_BYTES_OBJECT(screen, x + 5, y,
		color[ ((tile[0] & 0x04) >> 2) | ((tile[1] & 0x04) >> 1) ]);
	COPY_2_BYTES_OBJECT(screen, x + 6, y,
		color[ ((tile[0] & 0x02) >> 1) | ((tile[1] & 0x02) >> 0) ]);
	COPY_2_BYTES_OBJECT(screen, x + 7, y,
		color[ ((tile[0] & 0x01) >> 0) | ((tile[1] & 0x01) << 1) ]);
}

static void		fill_8_pixels_end(char *screen, uint32_t x, uint32_t y, char *tile, uint32_t end)
{
	COPY_2_BYTES(screen, x + 0, y,
		color[ ((tile[0] & 0x80) >> 7) | ((tile[1] & 0x80) >> 6) ]);
	if (end == 1) return ;
	COPY_2_BYTES(screen, x + 1, y,
		color[ ((tile[0] & 0x40) >> 6) | ((tile[1] & 0x40) >> 5) ]);
	if (end == 2) return ;
	COPY_2_BYTES(screen, x + 2, y,
		color[ ((tile[0] & 0x20) >> 5) | ((tile[1] & 0x20) >> 4) ]);
	if (end == 3) return ;
	COPY_2_BYTES(screen, x + 3, y,
		color[ ((tile[0] & 0x10) >> 4) | ((tile[1] & 0x10) >> 3) ]);
	if (end == 4) return ;
	COPY_2_BYTES(screen, x + 4, y,
		color[ ((tile[0] & 0x08) >> 3) | ((tile[1] & 0x08) >> 2) ]);
	if (end == 5) return ;
	COPY_2_BYTES(screen, x + 5, y,
		color[ ((tile[0] & 0x04) >> 2) | ((tile[1] & 0x04) >> 1) ]);
	if (end == 6) return ;
	COPY_2_BYTES(screen, x + 6, y,
		color[ ((tile[0] & 0x02) >> 1) | ((tile[1] & 0x02) >> 0) ]);
	if (end == 7) return ;
	COPY_2_BYTES(screen, x + 7, y,
		color[ ((tile[0] & 0x01) >> 0) | ((tile[1] & 0x01) << 1) ]);
}

static void		fill_8_pixels(char *screen, uint32_t x, uint32_t y, char *tile)
{
	COPY_2_BYTES(screen, x + 0, y,
		color[ ((tile[0] & 0x80) >> 7) | ((tile[1] & 0x80) >> 6) ]);
	COPY_2_BYTES(screen, x + 1, y,
		color[ ((tile[0] & 0x40) >> 6) | ((tile[1] & 0x40) >> 5) ]);
	COPY_2_BYTES(screen, x + 2, y,
		color[ ((tile[0] & 0x20) >> 5) | ((tile[1] & 0x20) >> 4) ]);
	COPY_2_BYTES(screen, x + 3, y,
		color[ ((tile[0] & 0x10) >> 4) | ((tile[1] & 0x10) >> 3) ]);
	COPY_2_BYTES(screen, x + 4, y,
		color[ ((tile[0] & 0x08) >> 3) | ((tile[1] & 0x08) >> 2) ]);
	COPY_2_BYTES(screen, x + 5, y,
		color[ ((tile[0] & 0x04) >> 2) | ((tile[1] & 0x04) >> 1) ]);
	COPY_2_BYTES(screen, x + 6, y,
		color[ ((tile[0] & 0x02) >> 1) | ((tile[1] & 0x02) >> 0) ]);
	COPY_2_BYTES(screen, x + 7, y,
		color[ ((tile[0] & 0x01) >> 0) | ((tile[1] & 0x01) << 1) ]);
}

static void		fill_8_pixels_start(char *screen, uint32_t x, uint32_t y, char *tile, uint32_t start)
{
	static const void *const	jump[] = {
		&&PIXEL_0, &&PIXEL_1, &&PIXEL_2, &&PIXEL_3,
		&&PIXEL_4, &&PIXEL_5, &&PIXEL_6, &&PIXEL_7
	};

	goto *jump[start];

PIXEL_0:
	COPY_2_BYTES(screen, x + 0, y,
		color[ ((tile[0] & 0x80) >> 7) | ((tile[1] & 0x80) >> 6) ]);
PIXEL_1:
	COPY_2_BYTES(screen, x + 1, y,
		color[ ((tile[0] & 0x40) >> 6) | ((tile[1] & 0x40) >> 5) ]);
PIXEL_2:
	COPY_2_BYTES(screen, x + 2, y,
		color[ ((tile[0] & 0x20) >> 5) | ((tile[1] & 0x20) >> 4) ]);
PIXEL_3:
	COPY_2_BYTES(screen, x + 3, y,
		color[ ((tile[0] & 0x10) >> 4) | ((tile[1] & 0x10) >> 3) ]);
PIXEL_4:
	COPY_2_BYTES(screen, x + 4, y,
		color[ ((tile[0] & 0x08) >> 3) | ((tile[1] & 0x08) >> 2) ]);
PIXEL_5:
	COPY_2_BYTES(screen, x + 5, y,
		color[ ((tile[0] & 0x04) >> 2) | ((tile[1] & 0x04) >> 1) ]);
PIXEL_6:
	COPY_2_BYTES(screen, x + 6, y,
		color[ ((tile[0] & 0x02) >> 1) | ((tile[1] & 0x02) >> 0) ]);
PIXEL_7:
	COPY_2_BYTES(screen, x + 7, y,
		color[ ((tile[0] & 0x01) >> 0) | ((tile[1] & 0x01) << 1) ]);
}

static inline void	lcd_display_screen(char *screen, char *debug)
{
	write(1, screen - 3, SCREEN_SIZE + 3);
	if (debug) {
		write(1, debug, strlen(debug));
	}
}

static void		screen_init(char *screen)
{
	uint32_t	x = 0, y = 0;

	screen[-1] = 'H';
	screen[-2] = '[';
	screen[-3] = '\e';
	while (y < SCREEN_H)
	{
		memcpy(screen + SCREEN_OFFSET(x, y), COL0, 13);
		x++;
		if (x == SCREEN_W) {
			screen[y * REAL_W + REAL_W - 1] = '\n';
			y++;
			x = 0;
		}
	}
	memcpy(screen + (REAL_W + 143 * REAL_W), "\e[0m", 4);
}

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

static void		dma_transfer(void)
{
	uint8_t	*src = (uint8_t*)(DMA_REGISTER << 8);
	uint8_t	*dst = g_memmap.complete_block + 0xfe00;

	memcpy(dst, src, 0xa0);
}

static uint16_t		lcd_write_line(char *screen, uint8_t *reset)
{
	char		*bg_chr_code = (char *)((LCDC_REGISTER & 0x08) ? g_memmap.vram_bg + 0x400 : g_memmap.vram_bg);
	char		*bg_data = (char *)((LCDC_REGISTER & 0x10) ? g_memmap.vram : g_memmap.vram + 0x800);
	char		*win_chr_code = (char *)((LCDC_REGISTER & 0x40) ? g_memmap.vram_bg + 0x400 : g_memmap.vram_bg);
	uint32_t	offset = (LCDC_REGISTER & 0x10) ? 0 : 128;
	uint32_t	tile_num, tile_end, line_end;
	uint32_t	padding;
	uint32_t	x, y;

	// background
	if (LCDC_REGISTER & 0x01)
	{
		x = SCX_REGISTER / 8;
		y = SCY_REGISTER / 8;
		tile_num = x + y * 32; // 32 = BG_W_TILES
		tile_end = tile_num + 20;
		line_end = y * 32 + 32;
		if (tile_end >= line_end)
			tile_end -= 32;
		padding = (SCX_REGISTER & 0x7);

		x = 0;
		y = LY_REGISTER;
		if (padding)
		{
			fill_8_pixels_start(
				screen, x, y,
				CHR_DATA( bg_chr_code[(uint8_t)(tile_num + offset)], (LY_REGISTER & 0x7), bg_data),
				padding
			);
			tile_num++;
			x += (LY_REGISTER & 0x7);
		}
		for (; tile_num != tile_end; tile_num++, x += 8)
		{
			if (tile_num == line_end)
			{
				for (tile_num -= 32; tile_num != tile_end; tile_num++, x += 8)
					fill_8_pixels(
						screen, x, y,
						CHR_DATA( bg_chr_code[(uint8_t)(tile_num + offset)], (LY_REGISTER & 0x7), bg_data)
					);
				break;
			}
			fill_8_pixels(
				screen, x, y,
				CHR_DATA( bg_chr_code[(uint8_t)(tile_num + offset)], (LY_REGISTER & 0x7), bg_data)
			);
		}
		if (padding)
		{
			fill_8_pixels_end(
				screen, x, y,
				CHR_DATA( bg_chr_code[(uint8_t)(tile_num + offset)], (LY_REGISTER & 0x7), bg_data),
				8U - padding
			);
		}
	}
	// window
	if (LCDC_REGISTER & 0x20)
	{
		tile_num = ((WX_REGISTER - 7) / 8) + (WY_REGISTER / 8) * 32; // 32 = BG_W_TILES
	}
	// objects
	if (LCDC_REGISTER & 0x02)
	{
		tile_num = 0; //remove this
	}

	if (STAT_REGISTER & 0x8)
	{
		if (reset)
			*reset = 0xfeU;
		IF_REGISTER |= 0x01U;
		return (LCDC_INT);
	}
	return (0);
}


/*
 * add STOP_MODE & HALT_MODE to execute.c
 * http://www.codeslinger.co.uk/pages/projects/gameboy/graphics.html
*/
static void		start_cpu_lcd_events(void)
{
	char			screen[SCREEN_SIZE + 8];
	char			*true_screen = screen + 8;
	registers_t		regs = {{0}};
	cycle_count_t	cycles;
	uint16_t		interrupt = 0;
	uint8_t			reset_int_flag;
	uint8_t			counter = 0;
	//uint16_t		tmp_interrupt;

	regs.reg_pc = 0x100U;
	regs.reg_sp = 0xfffeU;
	cycles = 0;

	screen_init(true_screen);

	for (;;)
	{
		// Execute cpu instruction
		if (GAMEBOY == NORMAL_MODE) {
			//plog("\nexecute_start\n");
			cycles = execute(&regs);
			//plog("execute_end\n");
		}
		counter++;

		// Check joypad events
		if (IF_REGISTER & 0x10U) {
			if ((interrupt = get_joypad_event()) != 0)
				reset_int_flag = 0xefU;
		}
		else
			get_joypad_event();

		if ((counter & 0x3) == 0)
		{
			// write one line to the buffer and display screen if all lines filled
			if (((LCDC_REGISTER & 0x80U) && GAMEBOY != STOP_MODE) || LY_REGISTER != 0)
			{
				if ((STAT_REGISTER & 0x40) && LY_REGISTER == LYC_REGISTER)
				{
					if (interrupt == 0) {
						interrupt = LCDC_INT;
						reset_int_flag = 0xfdU;
					}
					IF_REGISTER |= 0x02U;
				}
				if (LY_REGISTER < 144) {
					//plog("\nprint_line_start\n");
					if (interrupt)
						lcd_write_line(true_screen, NULL);
					else
						interrupt = lcd_write_line(true_screen, &reset_int_flag);

					//plog("print_lin_end\n");
				}
				if (LY_REGISTER == 143U)
					lcd_display_screen(true_screen, NULL);
				LY_REGISTER = (LY_REGISTER == 153U) ? 0 : LY_REGISTER + 1;
			}
		}

		// DMA transfer if any
		if (GAMEBOY != HALT_MODE && GAMEBOY != STOP_MODE)
		{
			if (DMA_REGISTER < 0xe0U && DMA_REGISTER > 0x7fU) // CGB ? DMA >= 0
			{
				//plog("\nDMA transfer start\n");
				dma_transfer();
				//plog("DMA transfer end\n");
			}
		}

		if (GAMEBOY == HALT_MODE && interrupt) {
			GAMEBOY = NORMAL_MODE;
			call_interrupt(&regs, interrupt, reset_int_flag);
		}
		else if (GAMEBOY == STOP_MODE && (interrupt || IF_REGISTER)) {
			GAMEBOY = NORMAL_MODE;
			call_interrupt(&regs, interrupt, reset_int_flag);
		}
		else {
			// call interrupt if any
			if (g_memmap.ime)
				call_interrupt(&regs, interrupt, reset_int_flag);
		}

		// sleep after put a frame
		if (LY_REGISTER == 0)
			usleep(16500);

	}
}

static void		*div_thread(void *unused)
{
	while (1)
	{
		DIV_REGISTER = (DIV_REGISTER == 0xffU) ? 0: DIV_REGISTER + 1;
		usleep(4000);
	}
}

static void		*tima_thread(void *unused)
{
	uint32_t	freq[4] = {
		16000, 250, 1000, 4000
	};
	uint32_t	usleep_time;
	uint32_t	started = 0;

	while (1)
	{
		if (!started)
		{
			if (TAC_REGISTER & 0x04)
			{
				usleep_time = freq[(TAC_REGISTER & 0x03)];
				started = 1;
			}
			else
				usleep_time = 16000;
		}
		else
		{
			if ((TAC_REGISTER & 0x04) == 0)
				started = 0;
			else
			{
				if (TIMA_REGISTER == 0xffU)
				{
					IF_REGISTER |= 0x04;
					TIMA_REGISTER = TMA_REGISTER;
				}
				else
					TIMA_REGISTER++;
			}
		}
		usleep(usleep_time);
	}
}

static void		start_game(void)
{
	pthread_t	div;
	pthread_t	tima;

	LY_REGISTER = 0;
	TMA_REGISTER = 0;
	TAC_REGISTER = 0;
	DMA_REGISTER = 0xffU;
	LCDC_REGISTER = 0; //0x80U;
	pthread_create(&div, NULL, div_thread, NULL);
	pthread_create(&tima, NULL, tima_thread, NULL);
	term_noecho_mode(ON);
	start_cpu_lcd_events();
//	joypad_control_loop();
	term_noecho_mode(OFF);
/*
	if (g_memmap.save_name)
		save_external_ram();
*/
}

static void		restore_terminal(int sig)
{
	(void)sig;
	term_noecho_mode(OFF);
	exit(1);
}

int		main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "%s \"cartridge path\"\n", argv[0]);
		return (1);
	}

	signal(SIGINT, restore_terminal);
	
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
