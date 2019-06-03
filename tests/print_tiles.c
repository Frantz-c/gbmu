/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   main.c                                           .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/30 09:02:45 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/06/03 13:39:35 by fcordon     ###    #+. /#+    ###.fr     */
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
//#include "tab.c"

unsigned int	g_i = 0;
unsigned int	g_next = 0;
unsigned int	g_x = 80;
char			*bin;
uint32_t		size;


#define COL0		"\e[48;5;255m  "
#define COL1		"\e[48;5;249m  "
#define COL2		"\e[48;5;242m  "
#define COL3		"\e[48;5;235m  "


#define REAL_W		(161 * 13 + 80 + 1)
#define SCREEN_W	(160)
#define SCREEN_H	(144)
#define SCREEN_SIZE	(SCREEN_H * REAL_W)
#define DEBUG_ZONE	(160 * 13 + 9)

// 18 * 20
#define TILE_OFFSET(x, y)	\
	(((y) * REAL_W * 8) + (13 * 8 * (x)))

// 160 * 144
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

//uint8_t			*g_get_real_addr[16] = {NULL};
//memory_map_t	g_memmap;

static char	*get_file_contents(const char *file, uint32_t *length)
{
	char			*content;
	const int		fd = open(file, O_RDONLY);
	struct stat		filestat;

	if (fd == -1)
	{
		fprintf(stderr, "can't open file %s\n", file);
		return (NULL);
	}
	lstat(file, &filestat);
	if (filestat.st_size == 0)
	{
		fprintf(stderr, "empty file\n");
		return (NULL);
	}
	else if (filestat.st_size > 0x1000000)
	{
		fprintf(stderr, "too heavy file\n");
		return (NULL);
	}
	if ((content = valloc(filestat.st_size)) == NULL)
	{
		fprintf(stderr, "malloc fatal error\n");
		return (NULL);
	}
	*length = read(fd, content, filestat.st_size);
	close(fd);
	return (content);
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

static void		joypad_control_loop(void)
{
	uint32_t			button = 0;
	uint32_t			arrows = 0;
	register uint32_t	tmp;
	uint64_t			key;


	EVENT_LOOP()
	{
		key = 0;
		if (read(0, &key, sizeof(long)) > 0)
		{
			switch (key)
			{
				case K_ESC:		return;
				// BUTTONS
				case A:			button = ~0x01; g_next = 1; break;
				case B:			button = ~0x02; break;
				case SELECT:	button = ~0x04; break;
				case START:		button = ~0x08; break;
				case '+':		g_i += 16; break;
				case '-':		g_i -= 16; break;
				//case 'r':		g_rev
				// ARROWS
				case K_RIGHT:	arrows = ~0x01; g_i++; break;
				case K_LEFT:	arrows = ~0x02; g_i--; break;
				case K_UP:		arrows = ~0x04; if (g_x < SCREEN_W) g_x += 8; break;
				case K_DOWN:	arrows = ~0x08; if (g_x > 8) g_x -= 8; break;
			}
		}
		/*
		if ((tmp = (g_memmap.complete_block[0xff00] & 0x30)) != 0x0)
		{
			if (tmp == 0x30)
				g_memmap.complete_block[0xff00] = 0;
			else if (tmp == 0x20)
				g_memmap.complete_block[0xff00] |= (arrows & 0x0f);
			else
				g_memmap.complete_block[0xff00] |= (button & 0x0f);
		}
		*/
	}
}

static void		write_tile(char *screen, char *tile, unsigned int x, unsigned int y)
{
	unsigned int		i = 0;
//	static const char	color[4][2] = {{'5', '5'}, {'4', '9'}, {'4', '2'}, {'3', '5'}};
#if BYTE_ORDER == LITTLE_ENDIAN
	static const uint16_t	color2[4] = {'5' | ('5' << 8), '4' | ('9' << 8), '4' | ('2' << 8), '3' | ('5' << 8)};
#else
	static const uint16_t	color2[4] = {'5' | ('5' << 8), '9' | ('4' << 8), '2' | ('4' << 8), '5' | ('3' << 8)};
#endif
/*	
	#define COPY_2_BYTES(screen, x, y, c)	\
		screen[ ((y) * SCREEN_W) + (13 * (x)) + 8] = c[0];\
		screen[ ((y) * SCREEN_W) + (13 * (x)) + 9] = c[1];
*/
	#define COPY_2_BYTES_ONE_SHOT(screen, x, y, c)	\
		*(uint16_t*)(screen + ((y) * REAL_W) + (13 * (x)) + 8) = c;\

	while (i < 16)
	{
		COPY_2_BYTES_ONE_SHOT(screen, x + 0, ((i / 2) + y),
			color2[ ((tile[i] & 0x80) >> 7) | ((tile[i + 1] & 0x80) >> 6) ]);
		COPY_2_BYTES_ONE_SHOT(screen, x + 1, ((i / 2) + y),
			color2[ ((tile[i] & 0x40) >> 6) | ((tile[i + 1] & 0x40) >> 5) ]);
		COPY_2_BYTES_ONE_SHOT(screen, x + 2, ((i / 2) + y),
			color2[ ((tile[i] & 0x20) >> 5) | ((tile[i + 1] & 0x20) >> 4) ]);
		COPY_2_BYTES_ONE_SHOT(screen, x + 3, ((i / 2) + y),
			color2[ ((tile[i] & 0x10) >> 4) | ((tile[i + 1] & 0x10) >> 3) ]);
		COPY_2_BYTES_ONE_SHOT(screen, x + 4, ((i / 2) + y),
			color2[ ((tile[i] & 0x08) >> 3) | ((tile[i + 1] & 0x08) >> 2) ]);
		COPY_2_BYTES_ONE_SHOT(screen, x + 5, ((i / 2) + y),
			color2[ ((tile[i] & 0x04) >> 2) | ((tile[i + 1] & 0x04) >> 1) ]);
		COPY_2_BYTES_ONE_SHOT(screen, x + 6, ((i / 2) + y),
			color2[ ((tile[i] & 0x02) >> 1) | ((tile[i + 1] & 0x02) >> 0) ]);
		COPY_2_BYTES_ONE_SHOT(screen, x + 7, ((i / 2) + y),
			color2[ ((tile[i] & 0x01) >> 0) | ((tile[i + 1] & 0x01) << 1) ]);
		/*
		COPY_2_BYTES(screen, x + 0, ((i / 2) + y), color[ ((tile[i] & 0x80) >> 7) | ((tile[i + 1] & 0x80) >> 6) ]);
		COPY_2_BYTES(screen, x + 1, ((i / 2) + y), color[ ((tile[i] & 0x40) >> 6) | ((tile[i + 1] & 0x40) >> 5) ]);
		COPY_2_BYTES(screen, x + 2, ((i / 2) + y), color[ ((tile[i] & 0x20) >> 5) | ((tile[i + 1] & 0x20) >> 4) ]);
		COPY_2_BYTES(screen, x + 3, ((i / 2) + y), color[ ((tile[i] & 0x10) >> 4) | ((tile[i + 1] & 0x10) >> 3) ]);
		COPY_2_BYTES(screen, x + 4, ((i / 2) + y), color[ ((tile[i] & 0x08) >> 3) | ((tile[i + 1] & 0x08) >> 2) ]);
		COPY_2_BYTES(screen, x + 5, ((i / 2) + y), color[ ((tile[i] & 0x04) >> 2) | ((tile[i + 1] & 0x04) >> 1) ]);
		COPY_2_BYTES(screen, x + 6, ((i / 2) + y), color[ ((tile[i] & 0x02) >> 1) | ((tile[i + 1] & 0x02) >> 0) ]);
		COPY_2_BYTES(screen, x + 7, ((i / 2) + y), color[ ((tile[i] & 0x01) >> 0) | ((tile[i + 1] & 0x01) << 1) ]);
		*/
		i += 2;
	}
}

static void		write_debug(char *screen, int line, char *str)
{
	register uint32_t	len;

	if ((len = strlen(str)) > 80)
		len = 80;
	memcpy(screen + SCREEN_OFFSET(SCREEN_W + 1, line - 1) + 4, str, len);
}

static void		put_screen(char	*screen)
{
	write(1, "\e[2J\e[H", 7);
	for (unsigned int i = 0; i < SCREEN_H; i++)
	{
		memcpy(screen + (REAL_W - 90 + i * REAL_W), "\e[0m", 4);
		screen[i * REAL_W + REAL_W - 1] = '\n';
	}
	write(1, screen, SCREEN_SIZE);
}

static void		init_screen(char *screen)
{
	uint32_t	x = 0, y = 0;

	while (y < SCREEN_H)
	{
		memcpy(screen + SCREEN_OFFSET(x, y), COL3, 13);
		x++;
		if (x == SCREEN_W) {
			y++;
			x = 0;
		}
	}
}

static void		*screen_control_thread(void *unused)
{
	char			screen[SCREEN_SIZE] = {0};
	char			tile[16];
	char			buf[128];
	unsigned int	x, y, lastx, new_i, i_backup;

	(void)unused;
	init_screen(screen);
	put_screen(screen);
	for (; g_i < size - (SCREEN_H * SCREEN_W); )
	{

load:
		sprintf(buf, "\e[0m    offset = 0x%X  ", g_i);
		write_debug(screen, 144, buf);
		
		i_backup = g_i;
		lastx = g_x;
		for (new_i = g_i, y = 0, x = 0; y < SCREEN_H; new_i += 16)
		{
			memcpy(tile, bin + new_i, 16);
			write_tile(screen, tile, x, y);
			x += 8;
			if (x == g_x)
			{
				x = 0;
				y += 8;
			}
		}

		put_screen(screen);
		while (g_next != 1)
		{
			if (g_x != lastx)
			{
				init_screen(screen);
				goto load;
			}
			if (g_i != i_backup)
				goto load;
			usleep(50000);
		}
		g_i = new_i;
		g_next = 0;
	}
/*
	SCREEN_LOOP()
	{
		break;
	}
	*/
	return (NULL);
}

static void		*cpu_control_thread(void *unused)
{
	registers_t		regs = {{0}};
	cycle_count_t	cycles;
	(void)unused;

	regs.reg_pc = 0x100;
	regs.reg_sp = 0xfffe;

	CPU_LOOP()
	{
		// cycles = execute(&regs);
		// check interrupts & launch interrupts & sync
		break;
	}
	return (NULL);
}

static void		start_game(void)
{
//	pthread_t	cpu;
	pthread_t	screen;

	pthread_create(&screen, NULL, screen_control_thread, NULL);
//	pthread_create(&cpu, NULL, cpu_control_thread, NULL);
	term_noecho_mode(ON);
	joypad_control_loop();
	term_noecho_mode(OFF);
	//save_external_ram();
}

static void		restore_terminal(int sig)
{
	(void)sig;
	term_noecho_mode(OFF);
	exit(1);
}

int		main(int argc, char *argv[])
{
	if (argc != 2 && argc != 3)
	{
		fprintf(stderr, "%s \"cartridge path\"\n", argv[0]);
		return (1);
	}
	if (argc == 3)
		sscanf(argv[2], "%x", &g_i);

	signal(SIGINT, restore_terminal);

	if ((bin = get_file_contents(argv[1], &size)) == NULL)
		return (1);
	start_game();

	return (0);
}
