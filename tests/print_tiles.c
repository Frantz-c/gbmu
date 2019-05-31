/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   main.c                                           .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/30 09:02:45 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/05/31 12:09:24 by fcordon     ###    #+. /#+    ###.fr     */
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

unsigned int	g_offset = 0;
unsigned int	g_next = 0;

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

uint8_t			*g_get_real_addr[16] = {NULL};
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
				case A:			button = ~0x01; g_next = 1; break;
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

static void		put_screen(char	*screen)
{
	register unsigned int	len;
	write(1, "\e[2J\e[H", 7);
	for (unsigned int i = 0; i < 120; i++)
	{
		write(1, screen + (i * (145 * 13 + 80)), (145 * 13 + 80));
		write(1, "\e[0m\n", 5);
	}
}

static void		write_dmg_tile_in_screen(char *screen, char *tile, size_t tile_size,
				unsigned int x, unsigned int y)
{
	unsigned int		i = 0;
	static const char	*color[4] = {COL0, COL1, COL2, COL3};

	if (tile_size == 64)
	{
		while (i < 16)
		{
			//printf("%p???  %scolor\e[0m\n\n", screen + (((i / 2) + y) * (13 * (x + 1))), color[ ((tile[i] & 0x80) >> 7) | ((tile[i + 1] & 0x80) >> 6) ]);
			strcpy(screen + (((i / 2) + y) * (145 * 13 + 80)) + (13 * (x + 0)), color[ ((tile[i] & 0x80) >> 7) | ((tile[i + 1] & 0x80) >> 6) ]);
			strcpy(screen + (((i / 2) + y) * (145 * 13 + 80)) + (13 * (x + 1)), color[ ((tile[i] & 0x40) >> 6) | ((tile[i + 1] & 0x40) >> 5) ]);
			strcpy(screen + (((i / 2) + y) * (145 * 13 + 80)) + (13 * (x + 2)), color[ ((tile[i] & 0x20) >> 5) | ((tile[i + 1] & 0x20) >> 4) ]);
			strcpy(screen + (((i / 2) + y) * (145 * 13 + 80)) + (13 * (x + 3)), color[ ((tile[i] & 0x10) >> 4) | ((tile[i + 1] & 0x10) >> 3) ]);
			strcpy(screen + (((i / 2) + y) * (145 * 13 + 80)) + (13 * (x + 4)), color[ ((tile[i] & 0x08) >> 3) | ((tile[i + 1] & 0x08) >> 2) ]);
			strcpy(screen + (((i / 2) + y) * (145 * 13 + 80)) + (13 * (x + 5)), color[ ((tile[i] & 0x04) >> 2) | ((tile[i + 1] & 0x04) >> 1) ]);
			strcpy(screen + (((i / 2) + y) * (145 * 13 + 80)) + (13 * (x + 6)), color[ ((tile[i] & 0x02) >> 1) | ((tile[i + 1] & 0x02) << 0) ]);
			strcpy(screen + (((i / 2) + y) * (145 * 13 + 80)) + (13 * (x + 7)), color[ ((tile[i] & 0x01) >> 0) | ((tile[i + 1] & 0x01) << 1) ]);
			//a supprimer
			//strcpy(screen + (((i / 2) + y) * (145 * 13 + 80)) + (13 * (x + 8)), color[3]);
			//strcpy(screen + (((i / 2) + y) * (145 * 13 + 80)) + (13 * (x + 9)), color[3]);
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
	char	screen[160 * ((145 * 13) + 80)] = {0};
	char	*tile = "\x3c\x3c\x42\x7e\x85\xfb\x81\xff\xa1\xdf\xb1\xcf\x42\x7e\x3c\x3c";
	char	tile1[16];
	char	tile2[16];
	char	tile3[16];
	char	tile4[16];
	char	tile5[16];
	char	tile6[16];
	char	tile7[16];
	char	tile8[16];
	char	tile9[16];
	char	tilea[16];
	char	tileb[16];
	char	tilec[16];
	char	tiled[16];
	char	tilee[16];

	char	tile11[16];
	char	tile12[16];
	char	tile13[16];
	char	tile14[16];
	char	tile15[16];
	char	tile16[16];
	char	tile17[16];
	char	tile18[16];
	char	tile19[16];
	char	tile1a[16];
	char	tile1b[16];
	char	tile1c[16];
	char	tile1d[16];
	char	tile1e[16];

	char	tile21[16];
	char	tile22[16];
	char	tile23[16];
	char	tile24[16];
	char	tile25[16];
	char	tile26[16];
	char	tile27[16];
	char	tile28[16];
	char	tile29[16];
	char	tile2a[16];
	char	tile2b[16];
	char	tile2c[16];
	char	tile2d[16];
	char	tile2e[16];

	char	tile31[16];
	char	tile32[16];
	char	tile33[16];
	char	tile34[16];
	char	tile35[16];
	char	tile36[16];
	char	tile37[16];
	char	tile38[16];
	char	tile39[16];
	char	tile3a[16];
	char	tile3b[16];
	char	tile3c[16];
	char	tile3d[16];
	char	tile3e[16];

	char	tile41[16];
	char	tile42[16];
	char	tile43[16];
	char	tile44[16];
	char	tile45[16];
	char	tile46[16];
	char	tile47[16];
	char	tile48[16];
	char	tile49[16];
	char	tile4a[16];
	char	tile4b[16];
	char	tile4c[16];
	char	tile4d[16];
	char	tile4e[16];

	char	tile51[16];
	char	tile52[16];
	char	tile53[16];
	char	tile54[16];
	char	tile55[16];
	char	tile56[16];
	char	tile57[16];
	char	tile58[16];
	char	tile59[16];
	char	tile5a[16];
	char	tile5b[16];
	char	tile5c[16];
	char	tile5d[16];
	char	tile5e[16];

	char	tile61[16];
	char	tile62[16];
	char	tile63[16];
	char	tile64[16];
	char	tile65[16];
	char	tile66[16];
	char	tile67[16];
	char	tile68[16];
	char	tile69[16];
	char	tile6a[16];
	char	tile6b[16];
	char	tile6c[16];
	char	tile6d[16];
	char	tile6e[16];

	char	tile71[16];
	char	tile72[16];
	char	tile73[16];
	char	tile74[16];
	char	tile75[16];
	char	tile76[16];
	char	tile77[16];
	char	tile78[16];
	char	tile79[16];
	char	tile7a[16];
	char	tile7b[16];
	char	tile7c[16];
	char	tile7d[16];
	char	tile7e[16];

	char	tile81[16];
	char	tile82[16];
	char	tile83[16];
	char	tile84[16];
	char	tile85[16];
	char	tile86[16];
	char	tile87[16];
	char	tile88[16];
	char	tile89[16];
	char	tile8a[16];
	char	tile8b[16];
	char	tile8c[16];
	char	tile8d[16];
	char	tile8e[16];

	char	tile91[16];
	char	tile92[16];
	char	tile93[16];
	char	tile94[16];
	char	tile95[16];
	char	tile96[16];
	char	tile97[16];
	char	tile98[16];
	char	tile99[16];
	char	tile9a[16];
	char	tile9b[16];
	char	tile9c[16];
	char	tile9d[16];
	char	tile9e[16];

	char	tilea1[16];
	char	tilea2[16];
	char	tilea3[16];
	char	tilea4[16];
	char	tilea5[16];
	char	tilea6[16];
	char	tilea7[16];
	char	tilea8[16];
	char	tilea9[16];
	char	tileaa[16];
	char	tileab[16];
	char	tileac[16];
	char	tilead[16];
	char	tileae[16];

	// screen = 160 * 144
	// screen + right debug = 160 * 224
	(void)unused;
	char	buf[40];
unsigned int i;
for (i = g_offset; i < malloc_size(g_memmap.fixed_rom) - (140 * 16 + 8); i += (140 * 16 - 8))
{
/*	for (unsigned int j = 0; j < 8; i++, j++)
	{*/
		sprintf(buf, "    offset = 0x%X", i);
		strcpy(screen + 145 * 13, buf);
		memcpy(tile1, g_memmap.fixed_rom + i, 16);
		memcpy(tile2, g_memmap.fixed_rom + i + 16, 16);
		memcpy(tile3, g_memmap.fixed_rom + i + 32, 16);
		memcpy(tile4, g_memmap.fixed_rom + i + 48, 16);
		memcpy(tile5, g_memmap.fixed_rom + i + 64, 16);
		memcpy(tile6, g_memmap.fixed_rom + i + 80, 16);
		memcpy(tile7, g_memmap.fixed_rom + i + 96, 16);
		memcpy(tile8, g_memmap.fixed_rom + i + 112, 16);
		memcpy(tile9, g_memmap.fixed_rom + i + 128, 16);
		memcpy(tilea, g_memmap.fixed_rom + i + 144, 16);
		memcpy(tileb, g_memmap.fixed_rom + i + 160, 16);
		memcpy(tilec, g_memmap.fixed_rom + i + 176, 16);
		memcpy(tiled, g_memmap.fixed_rom + i + 192, 16);
		memcpy(tilee, g_memmap.fixed_rom + i + 208, 16);

		memcpy(tile11, g_memmap.fixed_rom + i + 224, 16);
		memcpy(tile12, g_memmap.fixed_rom + i + 240, 16);
		memcpy(tile13, g_memmap.fixed_rom + i + 256, 16);
		memcpy(tile14, g_memmap.fixed_rom + i + 272, 16);
		memcpy(tile15, g_memmap.fixed_rom + i + 288, 16);
		memcpy(tile16, g_memmap.fixed_rom + i + 304, 16);
		memcpy(tile17, g_memmap.fixed_rom + i + 320, 16);
		memcpy(tile18, g_memmap.fixed_rom + i + 336, 16);
		memcpy(tile19, g_memmap.fixed_rom + i + 352, 16);
		memcpy(tile1a, g_memmap.fixed_rom + i + 368, 16);
		memcpy(tile1b, g_memmap.fixed_rom + i + 384, 16);
		memcpy(tile1c, g_memmap.fixed_rom + i + 400, 16);
		memcpy(tile1d, g_memmap.fixed_rom + i + 416, 16);
		memcpy(tile1e, g_memmap.fixed_rom + i + 432, 16);

		memcpy(tile21, g_memmap.fixed_rom + i + 448, 16);
		memcpy(tile22, g_memmap.fixed_rom + i + 464, 16);
		memcpy(tile23, g_memmap.fixed_rom + i + 480, 16);
		memcpy(tile24, g_memmap.fixed_rom + i + 496, 16);
		memcpy(tile25, g_memmap.fixed_rom + i + 512, 16);
		memcpy(tile26, g_memmap.fixed_rom + i + 528, 16);
		memcpy(tile27, g_memmap.fixed_rom + i + 544, 16);
		memcpy(tile28, g_memmap.fixed_rom + i + 560, 16);
		memcpy(tile29, g_memmap.fixed_rom + i + 576, 16);
		memcpy(tile2a, g_memmap.fixed_rom + i + 592, 16);
		memcpy(tile2b, g_memmap.fixed_rom + i + 608, 16);
		memcpy(tile2c, g_memmap.fixed_rom + i + 624, 16);
		memcpy(tile2d, g_memmap.fixed_rom + i + 640, 16);
		memcpy(tile2e, g_memmap.fixed_rom + i + 656, 16);

		memcpy(tile31, g_memmap.fixed_rom + i + 672, 16);
		memcpy(tile32, g_memmap.fixed_rom + i + 688, 16);
		memcpy(tile33, g_memmap.fixed_rom + i + 704, 16);
		memcpy(tile34, g_memmap.fixed_rom + i + 720, 16);
		memcpy(tile35, g_memmap.fixed_rom + i + 736, 16);
		memcpy(tile36, g_memmap.fixed_rom + i + 752, 16);
		memcpy(tile37, g_memmap.fixed_rom + i + 768, 16);
		memcpy(tile38, g_memmap.fixed_rom + i + 784, 16);
		memcpy(tile39, g_memmap.fixed_rom + i + 800, 16);
		memcpy(tile3a, g_memmap.fixed_rom + i + 816, 16);
		memcpy(tile3b, g_memmap.fixed_rom + i + 832, 16);
		memcpy(tile3c, g_memmap.fixed_rom + i + 848, 16);
		memcpy(tile3d, g_memmap.fixed_rom + i + 864, 16);
		memcpy(tile3e, g_memmap.fixed_rom + i + 880, 16);

		memcpy(tile41, g_memmap.fixed_rom + i + 56 * 16, 16);
		memcpy(tile42, g_memmap.fixed_rom + i + 57 * 16, 16);
		memcpy(tile43, g_memmap.fixed_rom + i + 58 * 16, 16);
		memcpy(tile44, g_memmap.fixed_rom + i + 59 * 16, 16);
		memcpy(tile45, g_memmap.fixed_rom + i + 60 * 16, 16);
		memcpy(tile46, g_memmap.fixed_rom + i + 61 * 16, 16);
		memcpy(tile47, g_memmap.fixed_rom + i + 62 * 16, 16);
		memcpy(tile48, g_memmap.fixed_rom + i + 63 * 16, 16);
		memcpy(tile49, g_memmap.fixed_rom + i + 64 * 16, 16);
		memcpy(tile4a, g_memmap.fixed_rom + i + 65 * 16, 16);
		memcpy(tile4b, g_memmap.fixed_rom + i + 66 * 16, 16);
		memcpy(tile4c, g_memmap.fixed_rom + i + 67 * 16, 16);
		memcpy(tile4d, g_memmap.fixed_rom + i + 68 * 16, 16);
		memcpy(tile4e, g_memmap.fixed_rom + i + 69 * 16, 16);

		memcpy(tile51, g_memmap.fixed_rom + i + 70 * 16, 16);
		memcpy(tile52, g_memmap.fixed_rom + i + 71 * 16, 16);
		memcpy(tile53, g_memmap.fixed_rom + i + 72 * 16, 16);
		memcpy(tile54, g_memmap.fixed_rom + i + 73 * 16, 16);
		memcpy(tile55, g_memmap.fixed_rom + i + 74 * 16, 16);
		memcpy(tile56, g_memmap.fixed_rom + i + 75 * 16, 16);
		memcpy(tile57, g_memmap.fixed_rom + i + 76 * 16, 16);
		memcpy(tile58, g_memmap.fixed_rom + i + 77 * 16, 16);
		memcpy(tile59, g_memmap.fixed_rom + i + 78 * 16, 16);
		memcpy(tile5a, g_memmap.fixed_rom + i + 79 * 16, 16);
		memcpy(tile5b, g_memmap.fixed_rom + i + 80 * 16, 16);
		memcpy(tile5c, g_memmap.fixed_rom + i + 81 * 16, 16);
		memcpy(tile5d, g_memmap.fixed_rom + i + 82 * 16, 16);
		memcpy(tile5e, g_memmap.fixed_rom + i + 83 * 16, 16);

		memcpy(tile61, g_memmap.fixed_rom + i + 84 * 16, 16);
		memcpy(tile62, g_memmap.fixed_rom + i + 85 * 16, 16);
		memcpy(tile63, g_memmap.fixed_rom + i + 86 * 16, 16);
		memcpy(tile64, g_memmap.fixed_rom + i + 87 * 16, 16);
		memcpy(tile65, g_memmap.fixed_rom + i + 88 * 16, 16);
		memcpy(tile66, g_memmap.fixed_rom + i + 89 * 16, 16);
		memcpy(tile67, g_memmap.fixed_rom + i + 90 * 16, 16);
		memcpy(tile68, g_memmap.fixed_rom + i + 91 * 16, 16);
		memcpy(tile69, g_memmap.fixed_rom + i + 92 * 16, 16);
		memcpy(tile6a, g_memmap.fixed_rom + i + 93 * 16, 16);
		memcpy(tile6b, g_memmap.fixed_rom + i + 94 * 16, 16);
		memcpy(tile6c, g_memmap.fixed_rom + i + 95 * 16, 16);
		memcpy(tile6d, g_memmap.fixed_rom + i + 96 * 16, 16);
		memcpy(tile6e, g_memmap.fixed_rom + i + 97 * 16, 16);

		memcpy(tile71, g_memmap.fixed_rom + i + 98 * 16, 16);
		memcpy(tile72, g_memmap.fixed_rom + i + 99 * 16, 16);
		memcpy(tile73, g_memmap.fixed_rom + i + 101 * 16, 16);
		memcpy(tile74, g_memmap.fixed_rom + i + 102 * 16, 16);
		memcpy(tile75, g_memmap.fixed_rom + i + 103 * 16, 16);
		memcpy(tile76, g_memmap.fixed_rom + i + 104 * 16, 16);
		memcpy(tile77, g_memmap.fixed_rom + i + 105 * 16, 16);
		memcpy(tile78, g_memmap.fixed_rom + i + 106 * 16, 16);
		memcpy(tile79, g_memmap.fixed_rom + i + 107 * 16, 16);
		memcpy(tile7a, g_memmap.fixed_rom + i + 108 * 16, 16);
		memcpy(tile7b, g_memmap.fixed_rom + i + 109 * 16, 16);
		memcpy(tile7c, g_memmap.fixed_rom + i + 110 * 16, 16);
		memcpy(tile7d, g_memmap.fixed_rom + i + 111 * 16, 16);
		memcpy(tile7e, g_memmap.fixed_rom + i + 112 * 16, 16);

		memcpy(tile81, g_memmap.fixed_rom + i + 113 * 16, 16);
		memcpy(tile82, g_memmap.fixed_rom + i + 114 * 16, 16);
		memcpy(tile83, g_memmap.fixed_rom + i + 115 * 16, 16);
		memcpy(tile84, g_memmap.fixed_rom + i + 116 * 16, 16);
		memcpy(tile85, g_memmap.fixed_rom + i + 117 * 16, 16);
		memcpy(tile86, g_memmap.fixed_rom + i + 118 * 16, 16);
		memcpy(tile87, g_memmap.fixed_rom + i + 119 * 16, 16);
		memcpy(tile88, g_memmap.fixed_rom + i + 120 * 16, 16);
		memcpy(tile89, g_memmap.fixed_rom + i + 121 * 16, 16);
		memcpy(tile8a, g_memmap.fixed_rom + i + 122 * 16, 16);
		memcpy(tile8b, g_memmap.fixed_rom + i + 123 * 16, 16);
		memcpy(tile8c, g_memmap.fixed_rom + i + 124 * 16, 16);
		memcpy(tile8d, g_memmap.fixed_rom + i + 125 * 16, 16);
		memcpy(tile8e, g_memmap.fixed_rom + i + 126 * 16, 16);

		memcpy(tile91, g_memmap.fixed_rom + i + 127 * 16, 16);
		memcpy(tile92, g_memmap.fixed_rom + i + 128 * 16, 16);
		memcpy(tile93, g_memmap.fixed_rom + i + 129 * 16, 16);
		memcpy(tile94, g_memmap.fixed_rom + i + 130 * 16, 16);
		memcpy(tile95, g_memmap.fixed_rom + i + 131 * 16, 16);
		memcpy(tile96, g_memmap.fixed_rom + i + 132 * 16, 16);
		memcpy(tile97, g_memmap.fixed_rom + i + 133 * 16, 16);
		memcpy(tile98, g_memmap.fixed_rom + i + 134 * 16, 16);
		memcpy(tile99, g_memmap.fixed_rom + i + 135 * 16, 16);
		memcpy(tile9a, g_memmap.fixed_rom + i + 136 * 16, 16);
		memcpy(tile9b, g_memmap.fixed_rom + i + 137 * 16, 16);
		memcpy(tile9c, g_memmap.fixed_rom + i + 138 * 16, 16);
		memcpy(tile9d, g_memmap.fixed_rom + i + 139 * 16, 16);
		memcpy(tile9e, g_memmap.fixed_rom + i + 140 * 16, 16);

		memcpy(tilea1, g_memmap.fixed_rom + i + 141 * 16, 16);
		memcpy(tilea2, g_memmap.fixed_rom + i + 142 * 16, 16);
		memcpy(tilea3, g_memmap.fixed_rom + i + 143 * 16, 16);
		memcpy(tilea4, g_memmap.fixed_rom + i + 144 * 16, 16);
		memcpy(tilea5, g_memmap.fixed_rom + i + 145 * 16, 16);
		memcpy(tilea6, g_memmap.fixed_rom + i + 146 * 16, 16);
		memcpy(tilea7, g_memmap.fixed_rom + i + 147 * 16, 16);
		memcpy(tilea8, g_memmap.fixed_rom + i + 148 * 16, 16);
		memcpy(tilea9, g_memmap.fixed_rom + i + 149 * 16, 16);
		memcpy(tileaa, g_memmap.fixed_rom + i + 150 * 16, 16);
		memcpy(tileab, g_memmap.fixed_rom + i + 151 * 16, 16);
		memcpy(tileac, g_memmap.fixed_rom + i + 152 * 16, 16);
		memcpy(tilead, g_memmap.fixed_rom + i + 153 * 16, 16);
		memcpy(tileae, g_memmap.fixed_rom + i + 154 * 16, 16);

		write_dmg_tile_in_screen(screen, tile1, 64, 0, 0);
		write_dmg_tile_in_screen(screen, tile2, 64, 8, 0);
		write_dmg_tile_in_screen(screen, tile3, 64, 16, 0);
		write_dmg_tile_in_screen(screen, tile4, 64, 24, 0);
		write_dmg_tile_in_screen(screen, tile5, 64, 32, 0);
		write_dmg_tile_in_screen(screen, tile6, 64, 40, 0);
		write_dmg_tile_in_screen(screen, tile7, 64, 48, 0);
		write_dmg_tile_in_screen(screen, tile8, 64, 56, 0);
		write_dmg_tile_in_screen(screen, tile9, 64, 64, 0);
		write_dmg_tile_in_screen(screen, tilea, 64, 72, 0);
		write_dmg_tile_in_screen(screen, tileb, 64, 80, 0);
		write_dmg_tile_in_screen(screen, tilec, 64, 88, 0);
		write_dmg_tile_in_screen(screen, tiled, 64, 96, 0);
		write_dmg_tile_in_screen(screen, tilee, 64, 104, 0);

		write_dmg_tile_in_screen(screen, tile11, 64, 0, 8);
		write_dmg_tile_in_screen(screen, tile12, 64, 8, 8);
		write_dmg_tile_in_screen(screen, tile13, 64, 16, 8);
		write_dmg_tile_in_screen(screen, tile14, 64, 24, 8);
		write_dmg_tile_in_screen(screen, tile15, 64, 32, 8);
		write_dmg_tile_in_screen(screen, tile16, 64, 40, 8);
		write_dmg_tile_in_screen(screen, tile17, 64, 48, 8);
		write_dmg_tile_in_screen(screen, tile18, 64, 56, 8);
		write_dmg_tile_in_screen(screen, tile19, 64, 64, 8);
		write_dmg_tile_in_screen(screen, tile1a, 64, 72, 8);
		write_dmg_tile_in_screen(screen, tile1b, 64, 80, 8);
		write_dmg_tile_in_screen(screen, tile1c, 64, 88, 8);
		write_dmg_tile_in_screen(screen, tile1d, 64, 96, 8);
		write_dmg_tile_in_screen(screen, tile1e, 64, 104, 8);

		write_dmg_tile_in_screen(screen, tile21, 64, 0, 16);
		write_dmg_tile_in_screen(screen, tile22, 64, 8, 16);
		write_dmg_tile_in_screen(screen, tile23, 64, 16, 16);
		write_dmg_tile_in_screen(screen, tile24, 64, 24, 16);
		write_dmg_tile_in_screen(screen, tile25, 64, 32, 16);
		write_dmg_tile_in_screen(screen, tile26, 64, 40, 16);
		write_dmg_tile_in_screen(screen, tile27, 64, 48, 16);
		write_dmg_tile_in_screen(screen, tile28, 64, 56, 16);
		write_dmg_tile_in_screen(screen, tile29, 64, 64, 16);
		write_dmg_tile_in_screen(screen, tile2a, 64, 72, 16);
		write_dmg_tile_in_screen(screen, tile2b, 64, 80, 16);
		write_dmg_tile_in_screen(screen, tile2c, 64, 88, 16);
		write_dmg_tile_in_screen(screen, tile2d, 64, 96, 16);
		write_dmg_tile_in_screen(screen, tile2e, 64, 104, 16);

		write_dmg_tile_in_screen(screen, tile31, 64, 0, 24);
		write_dmg_tile_in_screen(screen, tile32, 64, 8, 24);
		write_dmg_tile_in_screen(screen, tile33, 64, 16, 24);
		write_dmg_tile_in_screen(screen, tile34, 64, 24, 24);
		write_dmg_tile_in_screen(screen, tile35, 64, 32, 24);
		write_dmg_tile_in_screen(screen, tile36, 64, 40, 24);
		write_dmg_tile_in_screen(screen, tile37, 64, 48, 24);
		write_dmg_tile_in_screen(screen, tile38, 64, 56, 24);
		write_dmg_tile_in_screen(screen, tile39, 64, 64, 24);
		write_dmg_tile_in_screen(screen, tile3a, 64, 72, 24);
		write_dmg_tile_in_screen(screen, tile3b, 64, 80, 24);
		write_dmg_tile_in_screen(screen, tile3c, 64, 88, 24);
		write_dmg_tile_in_screen(screen, tile3d, 64, 96, 24);
		write_dmg_tile_in_screen(screen, tile3e, 64, 104, 24);

		write_dmg_tile_in_screen(screen, tile41, 64, 0, 32);
		write_dmg_tile_in_screen(screen, tile42, 64, 8, 32);
		write_dmg_tile_in_screen(screen, tile43, 64, 16, 32);
		write_dmg_tile_in_screen(screen, tile44, 64, 24, 32);
		write_dmg_tile_in_screen(screen, tile45, 64, 32, 32);
		write_dmg_tile_in_screen(screen, tile46, 64, 40, 32);
		write_dmg_tile_in_screen(screen, tile47, 64, 48, 32);
		write_dmg_tile_in_screen(screen, tile48, 64, 56, 32);
		write_dmg_tile_in_screen(screen, tile49, 64, 64, 32);
		write_dmg_tile_in_screen(screen, tile4a, 64, 72, 32);
		write_dmg_tile_in_screen(screen, tile4b, 64, 80, 32);
		write_dmg_tile_in_screen(screen, tile4c, 64, 88, 32);
		write_dmg_tile_in_screen(screen, tile4d, 64, 96, 32);
		write_dmg_tile_in_screen(screen, tile4e, 64, 104, 32);

		write_dmg_tile_in_screen(screen, tile51, 64, 0, 40);
		write_dmg_tile_in_screen(screen, tile52, 64, 8, 40);
		write_dmg_tile_in_screen(screen, tile53, 64, 16, 40);
		write_dmg_tile_in_screen(screen, tile54, 64, 24, 40);
		write_dmg_tile_in_screen(screen, tile55, 64, 32, 40);
		write_dmg_tile_in_screen(screen, tile56, 64, 40, 40);
		write_dmg_tile_in_screen(screen, tile57, 64, 48, 40);
		write_dmg_tile_in_screen(screen, tile58, 64, 56, 40);
		write_dmg_tile_in_screen(screen, tile59, 64, 64, 40);
		write_dmg_tile_in_screen(screen, tile5a, 64, 72, 40);
		write_dmg_tile_in_screen(screen, tile5b, 64, 80, 40);
		write_dmg_tile_in_screen(screen, tile5c, 64, 88, 40);
		write_dmg_tile_in_screen(screen, tile5d, 64, 96, 40);
		write_dmg_tile_in_screen(screen, tile5e, 64, 104, 40);

		write_dmg_tile_in_screen(screen, tile61, 64, 0, 48);
		write_dmg_tile_in_screen(screen, tile62, 64, 8, 48);
		write_dmg_tile_in_screen(screen, tile63, 64, 16, 48);
		write_dmg_tile_in_screen(screen, tile64, 64, 24, 48);
		write_dmg_tile_in_screen(screen, tile65, 64, 32, 48);
		write_dmg_tile_in_screen(screen, tile66, 64, 40, 48);
		write_dmg_tile_in_screen(screen, tile67, 64, 48, 48);
		write_dmg_tile_in_screen(screen, tile68, 64, 56, 48);
		write_dmg_tile_in_screen(screen, tile69, 64, 64, 48);
		write_dmg_tile_in_screen(screen, tile6a, 64, 72, 48);
		write_dmg_tile_in_screen(screen, tile6b, 64, 80, 48);
		write_dmg_tile_in_screen(screen, tile6c, 64, 88, 48);
		write_dmg_tile_in_screen(screen, tile6d, 64, 96, 48);
		write_dmg_tile_in_screen(screen, tile6e, 64, 104, 48);

		write_dmg_tile_in_screen(screen, tile71, 64, 0, 56);
		write_dmg_tile_in_screen(screen, tile72, 64, 8, 56);
		write_dmg_tile_in_screen(screen, tile73, 64, 16, 56);
		write_dmg_tile_in_screen(screen, tile74, 64, 24, 56);
		write_dmg_tile_in_screen(screen, tile75, 64, 32, 56);
		write_dmg_tile_in_screen(screen, tile76, 64, 40, 56);
		write_dmg_tile_in_screen(screen, tile77, 64, 48, 56);
		write_dmg_tile_in_screen(screen, tile78, 64, 56, 56);
		write_dmg_tile_in_screen(screen, tile79, 64, 64, 56);
		write_dmg_tile_in_screen(screen, tile7a, 64, 72, 56);
		write_dmg_tile_in_screen(screen, tile7b, 64, 80, 56);
		write_dmg_tile_in_screen(screen, tile7c, 64, 88, 56);
		write_dmg_tile_in_screen(screen, tile7d, 64, 96, 56);
		write_dmg_tile_in_screen(screen, tile7e, 64, 104, 56);

		write_dmg_tile_in_screen(screen, tile81, 64, 0, 64);
		write_dmg_tile_in_screen(screen, tile82, 64, 8, 64);
		write_dmg_tile_in_screen(screen, tile83, 64, 16, 64);
		write_dmg_tile_in_screen(screen, tile84, 64, 24, 64);
		write_dmg_tile_in_screen(screen, tile85, 64, 32, 64);
		write_dmg_tile_in_screen(screen, tile86, 64, 40, 64);
		write_dmg_tile_in_screen(screen, tile87, 64, 48, 64);
		write_dmg_tile_in_screen(screen, tile88, 64, 56, 64);
		write_dmg_tile_in_screen(screen, tile89, 64, 64, 64);
		write_dmg_tile_in_screen(screen, tile8a, 64, 72, 64);
		write_dmg_tile_in_screen(screen, tile8b, 64, 80, 64);
		write_dmg_tile_in_screen(screen, tile8c, 64, 88, 64);
		write_dmg_tile_in_screen(screen, tile8d, 64, 96, 64);
		write_dmg_tile_in_screen(screen, tile8e, 64, 104, 64);

		write_dmg_tile_in_screen(screen, tile91, 64, 0, 72);
		write_dmg_tile_in_screen(screen, tile92, 64, 8, 72);
		write_dmg_tile_in_screen(screen, tile93, 64, 16, 72);
		write_dmg_tile_in_screen(screen, tile94, 64, 24, 72);
		write_dmg_tile_in_screen(screen, tile95, 64, 32, 72);
		write_dmg_tile_in_screen(screen, tile96, 64, 40, 72);
		write_dmg_tile_in_screen(screen, tile97, 64, 48, 72);
		write_dmg_tile_in_screen(screen, tile98, 64, 56, 72);
		write_dmg_tile_in_screen(screen, tile99, 64, 64, 72);
		write_dmg_tile_in_screen(screen, tile9a, 64, 72, 72);
		write_dmg_tile_in_screen(screen, tile9b, 64, 80, 72);
		write_dmg_tile_in_screen(screen, tile9c, 64, 88, 72);
		write_dmg_tile_in_screen(screen, tile9d, 64, 96, 72);
		write_dmg_tile_in_screen(screen, tile9e, 64, 104, 72);

		write_dmg_tile_in_screen(screen, tilea1, 64, 0, 80);
		write_dmg_tile_in_screen(screen, tilea2, 64, 8, 80);
		write_dmg_tile_in_screen(screen, tilea3, 64, 16, 80);
		write_dmg_tile_in_screen(screen, tilea4, 64, 24, 80);
		write_dmg_tile_in_screen(screen, tilea5, 64, 32, 80);
		write_dmg_tile_in_screen(screen, tilea6, 64, 40, 80);
		write_dmg_tile_in_screen(screen, tilea7, 64, 48, 80);
		write_dmg_tile_in_screen(screen, tilea8, 64, 56, 80);
		write_dmg_tile_in_screen(screen, tilea9, 64, 64, 80);
		write_dmg_tile_in_screen(screen, tileaa, 64, 72, 80);
		write_dmg_tile_in_screen(screen, tileab, 64, 80, 80);
		write_dmg_tile_in_screen(screen, tileac, 64, 88, 80);
		write_dmg_tile_in_screen(screen, tilead, 64, 96, 80);
		write_dmg_tile_in_screen(screen, tileae, 64, 104, 80);

		put_screen(screen);
/*		usleep(300000);
	}*/
		while (g_next != 1) usleep(50000);
		g_next = 0;
}

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
		// cycles = execute(&regs);
		// check interrupts & launch interrupts & sync
		break;
	}
	return (NULL);
}

static void		start_game(void)
{
	pthread_t	cpu;
	pthread_t	screen;

	puts("ok\n\n");
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
	if (argc != 2 && argc != 3)
	{
		fprintf(stderr, "%s \"cartridge path\"\n", argv[0]);
		return (1);
	}
	if (argc == 3)
		sscanf(argv[2], "%x", &g_offset);

	signal(SIGINT, restore_terminal);

	open_cartridge(argv[1]);
	start_game();

	return (0);
}
