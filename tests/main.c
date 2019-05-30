/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   main.c                                           .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/30 09:02:45 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/05/30 19:02:47 by fcordon     ###    #+. /#+    ###.fr     */
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

// len = 13
#define COL0		"\e[48;5;255m  "
#define COL1		"\e[48;5;249m  "
#define COL2		"\e[48;5;242m  "
#define COL3		"\e[48;5;235m  "

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

uint8_t			*g_get_real_read_addr[16] = {NULL};
memory_map_t	g_memmap;


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
				case A:			button = ~0x01; break;
				case B:			button = ~0x02; break;
				case SELECT:	button = ~0x04; break;
				case START:		button = ~0x08; break;
				// ARROWS
				case K_RIGHT:	arrows = ~0x01; break;
				case K_LEFT:	arrows = ~0x02; break;
				case K_UP:		arrows = ~0x04; break;
				case K_DOWN:	arrows = ~0x08; break;
			}
		}
		if ((tmp = (g_memmap.complete_block[0xff00] & 0x30)) != 0x0)
		{
			if (tmp == 0x30)
				g_memmap.complete_block[0xff00] = 0;
			else if (tmp == 0x20)
				g_memmap.complete_block[0xff00] |= (arrows & 0x0f);
			else
				g_memmap.complete_block[0xff00] |= (button & 0x0f);
		}
	}
}

static void		puchar(char	**screen)
{
	write(1, "\e[2J", 4);
	for (unsigned int i = 0; i < 160; i++)
		write(1, screen[i], strlen(screen[i]));
}

static void		write_dmg_tile_in_screen(char **screen, char *tile, size_t tile_size,
				unsigned int x, unsigned int y)
{
	unsigned int		i = 0;
	static const char	*color[4] = {COL0, COL1, COL2, COL3};

	if (tile_size == 64)
	{
		while (i < 16)
		{
			//printf("%p???  %scolor\e[0m\n\n", screen + (((i / 2) + y) * (13 * (x + 1))), color[ ((tile[i] & 0x80) >> 7) | ((tile[i + 1] & 0x80) >> 6) ]);
			strcpy(screen + (((i / 2) + y) * 145 * 13 + 80) + (13 * (x + 0)), color[ ((tile[i] & 0x80) >> 7) | ((tile[i + 1] & 0x80) >> 6) ]);
			strcpy(screen + (((i / 2) + y) * 145 * 13 + 80) + (13 * (x + 1)), color[ ((tile[i] & 0x40) >> 6) | ((tile[i + 1] & 0x40) >> 5) ]);
			strcpy(screen + (((i / 2) + y) * 145 * 13 + 80) + (13 * (x + 2)), color[ ((tile[i] & 0x20) >> 5) | ((tile[i + 1] & 0x20) >> 4) ]);
			strcpy(screen + (((i / 2) + y) * 145 * 13 + 80) + (13 * (x + 3)), color[ ((tile[i] & 0x10) >> 4) | ((tile[i + 1] & 0x10) >> 3) ]);
			strcpy(screen + (((i / 2) + y) * 145 * 13 + 80) + (13 * (x + 4)), color[ ((tile[i] & 0x08) >> 3) | ((tile[i + 1] & 0x08) >> 2) ]);
			strcpy(screen + (((i / 2) + y) * 145 * 13 + 80) + (13 * (x + 5)), color[ ((tile[i] & 0x04) >> 2) | ((tile[i + 1] & 0x04) >> 1) ]);
			strcpy(screen + (((i / 2) + y) * 145 * 13 + 80) + (13 * (x + 6)), color[ ((tile[i] & 0x02) >> 1) | ((tile[i + 1] & 0x02) << 0) ]);
			strcpy(screen + (((i / 2) + y) * 145 * 13 + 80) + (13 * (x + 7)), color[ ((tile[i] & 0x01) >> 0) | ((tile[i + 1] & 0x01) << 1) ]);
			i += 2;
		}
	}
	else
	{
		return ;
	}
}

static void		*screen_control_thread(void *unused)
{
	char	screen[160][(145 * 13) + 80] = {{0}};
	char	*tile = "\x3c\x3c\x42\x7e\x85\xfb\x81\xff\xa1\xdf\xb1\xcf\x42\x7e\x3c\x3c";
	// screen = 160 * 144
	// screen + right debug = 160 * 224
	(void)unused;

	printf("screen = %p\n", screen);
	write_dmg_tile_in_screen(screen, tile, 64, 0, 0);
	puchar(screen);

	SCREEN_LOOP()
	{
		break;
	}
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
		//cycles = execute(&regs);
		break;
	}
	return (NULL);
}

static void		start_game(void)
{
	pthread_t	cpu;
	pthread_t	screen;

	pthread_create(&screen, NULL, screen_control_thread, NULL);
	pthread_create(&cpu, NULL, cpu_control_thread, NULL);
	term_noecho_mode(ON);
	joypad_control_loop();
	term_noecho_mode(OFF);
	save_external_ram();
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

	open_cartridge(argv[1]);
	start_game();

	return (0);
}
