/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   main.c                                           .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/30 09:02:45 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/06/19 18:30:24 by fcordon     ###    #+. /#+    ###.fr     */
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

#define TILE_TEST	(uint8_t*)"\x22\xc1\xff\xff\x48\x30\x24\x18\x00\xff\xff\xff\x0a\x04\x05\x02"

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


#define STAT_HBLANK	(STAT_REGISTER & 0x8U)
#define STAT_VBLANK	(STAT_REGISTER & 0x10U)
#define STAT_OAM	(STAT_REGISTER & 0x20U)
#define STAT_LYC	(STAT_REGISTER & 0x40U)

#define JOYPAD_INT_ENABLE()		(IE_REGISTER & 0x10U)
#define TIMA_INT_ENABLE()		(IE_REGISTER & 0x04U)
#define LCDC_INT_ENABLE()		(IE_REGISTER & 0x02U)
#define VBLANK_INT_ENABLE()		(IE_REGISTER & 0x01U)

#define LCDC_LYC_INT_ENABLE()		\
	(\
		STAT_LYC && LCDC_INT_ENABLE()\
	)

#define LCDC_VBLANK_INT_ENABLE()		\
	(\
		STAT_VBLANK && LCDC_INT_ENABLE()\
	)

#define LCDC_HBLANK_INT_ENABLE()		\
	(\
		STAT_HBLANK && LCDC_INT_ENABLE()\
	)



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
int				log_file;

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

static void		open_log_file(void)
{
	log_file = open("log_gbmul", O_WRONLY | O_TRUNC | O_CREAT, 0664);

	if (log_file == -1)
	{
		fprintf(stderr, "FUCK YOU !\n");
		exit (1);
	}
}

static void		close_log_file(void)
{
	close(log_file);
}

extern void		plog2(const char *s, uint32_t size)
{
	put_file_contents("log_gbmul", s, size);
}

extern void		plog(const char *s)
{
	write(log_file, s, strlen(s));
//	put_file_contents("log_gbmul", s, strlen(s));
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

void	get_joypad_event(void)
{
	unsigned long	key = 0UL;
	unsigned int	button;
	unsigned int	arrows;

	read(0, &key, sizeof(unsigned long));
	if (key == K_ESC)
	{
		close_log_file();
		exit(0);
	}

	if ((P1_REGISTER & 0x30U) == 0x30U) {
		P1_REGISTER = 0U;
		return;
	}
	if ((P1_REGISTER & 0x30U) == 0U)
		return;
	
	if (key)
	{
		if ((P1_REGISTER & 0x30U) == 0x10U)
		{
			button = 0xffU;
			switch (key)
			{
				case A:			button = ~0x01U; break;
				case B:			button = ~0x02U; break;
				case SELECT:	button = ~0x04U; break;
				case START:		button = ~0x08U; break;
			}
		}
		else // if ((P1_REGISTER & 0x30U) == 0x20U)
		{
			button = 0xffU;
			switch (key)
			{
				case K_RIGHT:	button = ~0x01; break;
				case K_LEFT:	button = ~0x02; break;
				case K_UP:		button = ~0x04; break;
				case K_DOWN:	button = ~0x08; break;
			}		
		}
		if (JOYPAD_INT_ENABLE() && button != 0xffU)
			IF_REGISTER |= 0x10U;
		P1_REGISTER &= button;
	}
}


#define	CALL_INTERRUPT(interrupt, regs)	\
do\
{\
	dprintf(log_file, "\n~~ INTERRUPT 0x%hX ~~\n\n", interrupt);\
	g_memmap.ime = 0;\
	*(short*)GET_REAL_ADDR(regs->reg_sp) = (short)regs->reg_pc;\
	regs->reg_sp -= 2;\
	regs->reg_pc = interrupt;\
}\
while (0)

#define	CALL_INTERRUPT_2(interrupt, regs)	\
do\
{\
	dprintf(log_file, "\n~~ INTERRUPT 0x%hX ~~\n\n", interrupt);\
	g_memmap.ime = 0;\
	*(short*)GET_REAL_ADDR(regs.reg_sp) = (short)regs.reg_pc;\
	regs.reg_sp -= 2;\
	regs.reg_pc = interrupt;\
}\
while (0)


void			call_interrupt(registers_t *regs)
{
	if (IF_REGISTER)
	{
		// vblank
		if (VBLANK_INT_ENABLE() && (IF_REGISTER & 0x01U)) {
			IF_REGISTER ^= 0x01U;
			CALL_INTERRUPT((uint16_t)0x0040U, regs);
		}

		// lcdc
		if (LCDC_INT_ENABLE() && (IF_REGISTER & 0x02U)) {
			IF_REGISTER ^= 0x01U;
			CALL_INTERRUPT((uint16_t)0x0048U, regs);
		}

		// tima
		if (TIMA_INT_ENABLE() && (IF_REGISTER & 0x04U)) {
			IF_REGISTER ^= 0x01U;
			CALL_INTERRUPT((uint16_t)0x0050U, regs);
		}

		if (JOYPAD_INT_ENABLE() && (IF_REGISTER & 0x10U)) {
			IF_REGISTER ^= 0x01U;
			CALL_INTERRUPT((uint16_t)0x0060U, regs);
		}
	}
}

static void		dma_transfer(void)
{
	uint8_t	*src = GET_REAL_ADDR((unsigned long)(DMA_REGISTER) << 8);
	uint8_t	*dst = g_memmap.complete_block + 0xfe00;

	memcpy(dst, src, 0xa0);
}


static void		lcd_write_line(char *screen)
{
	uint8_t	*data_area;
	uint8_t	*bg_code_area;
	uint8_t	*wn_code_area;

	uint8_t		offset = (LCDC_REGISTER & 0x10) ? 0x00u : 0x80u;
	uint32_t	y, px;
	uint32_t	case_num;
	uint32_t	case_end;
	uint8_t		tile_num;
	char		*tile;

	data_area		= (LCDC_REGISTER & 0x10) ?
						GET_REAL_ADDR(0x8000) : GET_REAL_ADDR(0x8800);
	bg_code_area	= (LCDC_REGISTER & 0x08) ?
						g_memmap.vram_bg[0] + 0x400 : g_memmap.vram_bg[0];
	wn_code_area	= (LCDC_REGISTER & 0x40) ?
						g_memmap.vram_bg[0] + 0x400 : g_memmap.vram_bg[0];
						//GET_REAL_ADDR(0x9c00) : GET_REAL_ADDR(0x9800);

	/*
	 *	print background
	*/
	y = SCY_REGISTER & ~7u;
	case_num = y * 32 + (SCX_REGISTER & ~7u);
	case_end = case_num + 21;
	if (case_end > 31)
		case_end -= 32;

	px = 0;
	// print truncated left tile
	if (SCX_REGISTER & 7u)
	{
		tile_num = *(bg_code_area + case_num);
		tile = (char*)(data_area + (uint8_t)(tile_num - offset));
		fill_8_pixels_start(screen, px, LY_REGISTER, tile + ((LY_REGISTER & 7u) * 2), (SCX_REGISTER & 7u));
		px += 8;
	}
	// print full tiles
	for (++case_num; case_num != case_end; px += 8)
	{
		tile_num = *(bg_code_area + case_num);
		tile = (char*)(data_area + (uint8_t)(tile_num - offset));
		fill_8_pixels(screen, px, LY_REGISTER, tile + ((LY_REGISTER & 7u) * 2));
		if ((++case_num & 0x1f) == 0)
			case_num -= 32;
	}
	// print truncated right tile
	if (SCX_REGISTER & 7u)
	{
		tile_num = *(bg_code_area + case_num);
		tile = (char*)(data_area + (uint8_t)(tile_num - offset));
		fill_8_pixels_start(screen, px, LY_REGISTER, tile + ((LY_REGISTER & 7u) * 2), (8u - (SCX_REGISTER & 7u)));
	}

}


/*
 * add STOP_MODE & HALT_MODE to execute.c
 * http://www.codeslinger.co.uk/pages/projects/gameboy/graphics.html
*/


/*
 * set STAT for each mode (HBlank, ...)
*/
static void		start_cpu_lcd_events(void)
{
	char			screen[SCREEN_SIZE + 8];
	char			*true_screen = screen + 8;
	registers_t		regs = {{0}};
	cycle_count_t	cycles = 0;
	uint64_t		counter = 0;
	uint64_t		instruction_no = 0;

	regs.reg_pc = 0x100U;
	regs.reg_sp = 0xfffeU;

	screen_init(true_screen);

//	alarm(240);
	for (;;)
	{
		// Execute cpu instruction
		if (GAMEBOY == NORMAL_MODE) {
			//plog("\nexecute_start\n");
			dprintf(log_file, ">>>> Instruction numero %llu\n", instruction_no++);
			cycles += execute(&regs);
			//plog("execute_end\n");
		}

		counter++;

		if ((counter & 0x7U) == 0)
		{
			if ((LCDC_REGISTER & 0x80U) && GAMEBOY != STOP_MODE)
			{
				if (LY_REGISTER < 144U)
					lcd_write_line(true_screen);
				if (LY_REGISTER == 143U)
				{
					lcd_display_screen(true_screen, NULL);
					if (LCDC_VBLANK_INT_ENABLE())
						IF_REGISTER |= 0x02U;
					if (VBLANK_INT_ENABLE())
						IF_REGISTER |= 0x01U;
				}
				LY_REGISTER = (LY_REGISTER == 153U) ? 0 : LY_REGISTER + 1;
				if (LCDC_LYC_INT_ENABLE() && LY_REGISTER == LYC_REGISTER)
					IF_REGISTER |= 0x2U;
			}
			else
				LY_REGISTER = 0;
		}

		// DMA transfer if any
		if (GAMEBOY == NORMAL_MODE)
		{
			if (DMA_REGISTER < 0xe0U && DMA_REGISTER > 0x7fU) // CGB ? DMA >= 0
			{
				plog("\nDMA transfer start\n");
				dma_transfer();
				DMA_REGISTER = 0xffU;
				plog("DMA transfer end\n");
			}
		}

		// Check joypad events
		get_joypad_event();

		if (IF_REGISTER)
		{
			if (GAMEBOY == HALT_MODE)
			{
				GAMEBOY = NORMAL_MODE;
				if (IME_REGISTER)
					call_interrupt(&regs);
			}
			else if (GAMEBOY == STOP_MODE && (IF_REGISTER & 0x10U))
			{
				GAMEBOY = NORMAL_MODE;
				if (IME_REGISTER)
				{
					if (JOYPAD_INT_ENABLE()) {
						IF_REGISTER ^= 0x01U;
						CALL_INTERRUPT_2((uint16_t)0x0060U, regs);
					}
				}
			}
			else
			{
				// call interrupt if any
				if (IME_REGISTER)
					call_interrupt(&regs);
			}
		}
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

void	write_background_in_vram(void)
{
//	uint8_t	*tile = TILE_TEST;
	uint8_t	*bg_data;
	uint8_t	*bg_chr;

	bg_data = GET_REAL_ADDR(0x8800);
	bg_chr = g_memmap.vram_bg[0];
	
	memcpy(bg_data, TILE_TEST, 16);
	for (uint32_t i = 0; i < 0xffU; i++)
	{
		bg_chr[i] = 0x80U;
	}
}

static void		start_game(void)
{
	pthread_t	div;
	pthread_t	tima;

	IF_REGISTER = 0;
	IE_REGISTER = 0;
	SCY_REGISTER = 0;
	SCX_REGISTER = 0;
	LY_REGISTER = 0;
	TIMA_REGISTER = 0;
	TAC_REGISTER = 0;
	DMA_REGISTER = 0xffU;
	LCDC_REGISTER = 0x83U;
	g_memmap.cart_reg[0] = 0;
	g_memmap.cart_reg[1] = 0;
	g_memmap.cart_reg[2] = 0;
	g_memmap.cart_reg[3] = 0;
	g_memmap.cart_reg[4] = 0;
	g_memmap.cart_reg[5] = 0;
	g_memmap.cart_reg[6] = 0;
	g_memmap.cart_reg[7] = 0;
	g_memmap.ime = false;

	write_background_in_vram();

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
	close_log_file();
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
	signal(SIGSEGV, restore_terminal);
	signal(SIGFPE, restore_terminal);
	signal(SIGBUS, restore_terminal);

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
