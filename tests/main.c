/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   main.c                                           .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/30 09:02:45 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/05/30 14:58:03 by fcordon     ###    #+. /#+    ###.fr     */
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


#define K_UP		0x415b1b
#define K_DOWN		0x425b1b
#define K_LEFT		0x445b1b
#define K_RIGHT		0x435b1b
#define K_ENTER		10
#define K_DEL		0x7e335b1b
#define K_BACKSP	127
#define K_ESC		27
#define K_TAB		9
#define K_SPACE		32

#define A			K_ENTER
#define B			K_BACKSP
#define START		K_SPACE
#define SELECT		's'

#define PIXEL		"  "

#define ON				1
#define OFF				0
#define EVENT_LOOP()	for (;;)

uint8_t			*g_get_real_read_addr[16] = {NULL};
memory_map_t	g_memmap = {NULL};

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

static void	*joypad_control_thread(void *unused)
{
	uint32_t			button = 0;
	uint32_t			arrows = 0;
	register uint32_t	tmp;
	uint64_t			key;

	(void)unused;
	term_noecho_mode(ON);

	EVENT_LOOP()
	{
		key = 0;
		if (read(0, &key, sizeof(long)) > 0)
		{
			switch (key)
			{
				case K_ESC:		goto end;
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

end:
	term_noecho_mode(OFF);
	save_external_ram();
	exit(0);
	return (NULL);
}

static int		restore_terminal(int sig)
{
	(void)sig;
	term_noecho_mode(OFF);
	exit(1);
}

static void		start_game(void)
{
	registers_t	regs = {{0}};
	pthread_t	joypad;

	regs.reg_pc = 0x100;
	regs.reg_sp = 0xfffe;

	pthread_create(&joypad, NULL, joypad_control_thread, NULL);
	pthread_join(joypad, NULL);
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
