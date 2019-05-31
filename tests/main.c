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
#include "tab.c"

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

static void		put_screen(char	*screen)
{
	register unsigned int	len;
	write(1, "\e[2J", 4);
	for (unsigned int i = 0; i < 160; i++)
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
			strcpy(screen + (((i / 2) + y) * (145 * 13 + 80)) + (13 * (x + 8)), color[3]);
			strcpy(screen + (((i / 2) + y) * (145 * 13 + 80)) + (13 * (x + 9)), color[3]);
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
for (i = 0; i < 0x1000 - 140; i += (140 * 16 - 8))
{
	for (unsigned int j = 0; j < 8; i++, j++)
	{
		sprintf(buf, "    offset = 0x%X", i);
		strcpy(screen + 145 * 13, buf);
		memcpy(tile1, test + i, 16);
		memcpy(tile2, test + i + 16, 16);
		memcpy(tile3, test + i + 32, 16);
		memcpy(tile4, test + i + 48, 16);
		memcpy(tile5, test + i + 64, 16);
		memcpy(tile6, test + i + 80, 16);
		memcpy(tile7, test + i + 96, 16);
		memcpy(tile8, test + i + 112, 16);
		memcpy(tile9, test + i + 128, 16);
		memcpy(tilea, test + i + 144, 16);
		memcpy(tileb, test + i + 160, 16);
		memcpy(tilec, test + i + 176, 16);
		memcpy(tiled, test + i + 192, 16);
		memcpy(tilee, test + i + 208, 16);

		memcpy(tile11, test + i + 224, 16);
		memcpy(tile12, test + i + 240, 16);
		memcpy(tile13, test + i + 256, 16);
		memcpy(tile14, test + i + 272, 16);
		memcpy(tile15, test + i + 288, 16);
		memcpy(tile16, test + i + 304, 16);
		memcpy(tile17, test + i + 320, 16);
		memcpy(tile18, test + i + 336, 16);
		memcpy(tile19, test + i + 352, 16);
		memcpy(tile1a, test + i + 368, 16);
		memcpy(tile1b, test + i + 384, 16);
		memcpy(tile1c, test + i + 400, 16);
		memcpy(tile1d, test + i + 416, 16);
		memcpy(tile1e, test + i + 432, 16);

		memcpy(tile21, test + i + 448, 16);
		memcpy(tile22, test + i + 464, 16);
		memcpy(tile23, test + i + 480, 16);
		memcpy(tile24, test + i + 496, 16);
		memcpy(tile25, test + i + 512, 16);
		memcpy(tile26, test + i + 528, 16);
		memcpy(tile27, test + i + 544, 16);
		memcpy(tile28, test + i + 560, 16);
		memcpy(tile29, test + i + 576, 16);
		memcpy(tile2a, test + i + 592, 16);
		memcpy(tile2b, test + i + 608, 16);
		memcpy(tile2c, test + i + 624, 16);
		memcpy(tile2d, test + i + 640, 16);
		memcpy(tile2e, test + i + 656, 16);

		memcpy(tile31, test + i + 672, 16);
		memcpy(tile32, test + i + 688, 16);
		memcpy(tile33, test + i + 704, 16);
		memcpy(tile34, test + i + 720, 16);
		memcpy(tile35, test + i + 736, 16);
		memcpy(tile36, test + i + 752, 16);
		memcpy(tile37, test + i + 768, 16);
		memcpy(tile38, test + i + 784, 16);
		memcpy(tile39, test + i + 800, 16);
		memcpy(tile3a, test + i + 816, 16);
		memcpy(tile3b, test + i + 832, 16);
		memcpy(tile3c, test + i + 848, 16);
		memcpy(tile3d, test + i + 864, 16);
		memcpy(tile3e, test + i + 880, 16);

		memcpy(tile41, test + i + 56 * 16, 16);
		memcpy(tile42, test + i + 57 * 16, 16);
		memcpy(tile43, test + i + 58 * 16, 16);
		memcpy(tile44, test + i + 59 * 16, 16);
		memcpy(tile45, test + i + 60 * 16, 16);
		memcpy(tile46, test + i + 61 * 16, 16);
		memcpy(tile47, test + i + 62 * 16, 16);
		memcpy(tile48, test + i + 63 * 16, 16);
		memcpy(tile49, test + i + 64 * 16, 16);
		memcpy(tile4a, test + i + 65 * 16, 16);
		memcpy(tile4b, test + i + 66 * 16, 16);
		memcpy(tile4c, test + i + 67 * 16, 16);
		memcpy(tile4d, test + i + 68 * 16, 16);
		memcpy(tile4e, test + i + 69 * 16, 16);

		memcpy(tile51, test + i + 70 * 16, 16);
		memcpy(tile52, test + i + 71 * 16, 16);
		memcpy(tile53, test + i + 72 * 16, 16);
		memcpy(tile54, test + i + 73 * 16, 16);
		memcpy(tile55, test + i + 74 * 16, 16);
		memcpy(tile56, test + i + 75 * 16, 16);
		memcpy(tile57, test + i + 76 * 16, 16);
		memcpy(tile58, test + i + 77 * 16, 16);
		memcpy(tile59, test + i + 78 * 16, 16);
		memcpy(tile5a, test + i + 79 * 16, 16);
		memcpy(tile5b, test + i + 80 * 16, 16);
		memcpy(tile5c, test + i + 81 * 16, 16);
		memcpy(tile5d, test + i + 82 * 16, 16);
		memcpy(tile5e, test + i + 83 * 16, 16);

		memcpy(tile61, test + i + 84 * 16, 16);
		memcpy(tile62, test + i + 85 * 16, 16);
		memcpy(tile63, test + i + 86 * 16, 16);
		memcpy(tile64, test + i + 87 * 16, 16);
		memcpy(tile65, test + i + 88 * 16, 16);
		memcpy(tile66, test + i + 89 * 16, 16);
		memcpy(tile67, test + i + 90 * 16, 16);
		memcpy(tile68, test + i + 91 * 16, 16);
		memcpy(tile69, test + i + 92 * 16, 16);
		memcpy(tile6a, test + i + 93 * 16, 16);
		memcpy(tile6b, test + i + 94 * 16, 16);
		memcpy(tile6c, test + i + 95 * 16, 16);
		memcpy(tile6d, test + i + 96 * 16, 16);
		memcpy(tile6e, test + i + 97 * 16, 16);

		memcpy(tile71, test + i + 98 * 16, 16);
		memcpy(tile72, test + i + 99 * 16, 16);
		memcpy(tile73, test + i + 101 * 16, 16);
		memcpy(tile74, test + i + 102 * 16, 16);
		memcpy(tile75, test + i + 103 * 16, 16);
		memcpy(tile76, test + i + 104 * 16, 16);
		memcpy(tile77, test + i + 105 * 16, 16);
		memcpy(tile78, test + i + 106 * 16, 16);
		memcpy(tile79, test + i + 107 * 16, 16);
		memcpy(tile7a, test + i + 108 * 16, 16);
		memcpy(tile7b, test + i + 109 * 16, 16);
		memcpy(tile7c, test + i + 110 * 16, 16);
		memcpy(tile7d, test + i + 111 * 16, 16);
		memcpy(tile7e, test + i + 112 * 16, 16);

		memcpy(tile81, test + i + 113 * 16, 16);
		memcpy(tile82, test + i + 114 * 16, 16);
		memcpy(tile83, test + i + 115 * 16, 16);
		memcpy(tile84, test + i + 116 * 16, 16);
		memcpy(tile85, test + i + 117 * 16, 16);
		memcpy(tile86, test + i + 118 * 16, 16);
		memcpy(tile87, test + i + 119 * 16, 16);
		memcpy(tile88, test + i + 120 * 16, 16);
		memcpy(tile89, test + i + 121 * 16, 16);
		memcpy(tile8a, test + i + 122 * 16, 16);
		memcpy(tile8b, test + i + 123 * 16, 16);
		memcpy(tile8c, test + i + 124 * 16, 16);
		memcpy(tile8d, test + i + 125 * 16, 16);
		memcpy(tile8e, test + i + 126 * 16, 16);

		memcpy(tile91, test + i + 127 * 16, 16);
		memcpy(tile92, test + i + 128 * 16, 16);
		memcpy(tile93, test + i + 129 * 16, 16);
		memcpy(tile94, test + i + 130 * 16, 16);
		memcpy(tile95, test + i + 131 * 16, 16);
		memcpy(tile96, test + i + 132 * 16, 16);
		memcpy(tile97, test + i + 133 * 16, 16);
		memcpy(tile98, test + i + 134 * 16, 16);
		memcpy(tile99, test + i + 135 * 16, 16);
		memcpy(tile9a, test + i + 136 * 16, 16);
		memcpy(tile9b, test + i + 137 * 16, 16);
		memcpy(tile9c, test + i + 138 * 16, 16);
		memcpy(tile9d, test + i + 139 * 16, 16);
		memcpy(tile9e, test + i + 140 * 16, 16);

		memcpy(tilea1, test + i + 141 * 16, 16);
		memcpy(tilea2, test + i + 142 * 16, 16);
		memcpy(tilea3, test + i + 143 * 16, 16);
		memcpy(tilea4, test + i + 144 * 16, 16);
		memcpy(tilea5, test + i + 145 * 16, 16);
		memcpy(tilea6, test + i + 146 * 16, 16);
		memcpy(tilea7, test + i + 147 * 16, 16);
		memcpy(tilea8, test + i + 148 * 16, 16);
		memcpy(tilea9, test + i + 149 * 16, 16);
		memcpy(tileaa, test + i + 150 * 16, 16);
		memcpy(tileab, test + i + 151 * 16, 16);
		memcpy(tileac, test + i + 152 * 16, 16);
		memcpy(tilead, test + i + 153 * 16, 16);
		memcpy(tileae, test + i + 154 * 16, 16);

		write_dmg_tile_in_screen(screen, tile1, 64, 0, 0);
		write_dmg_tile_in_screen(screen, tile2, 64, 10, 0);
		write_dmg_tile_in_screen(screen, tile3, 64, 20, 0);
		write_dmg_tile_in_screen(screen, tile4, 64, 30, 0);
		write_dmg_tile_in_screen(screen, tile5, 64, 40, 0);
		write_dmg_tile_in_screen(screen, tile6, 64, 50, 0);
		write_dmg_tile_in_screen(screen, tile7, 64, 60, 0);
		write_dmg_tile_in_screen(screen, tile8, 64, 70, 0);
		write_dmg_tile_in_screen(screen, tile9, 64, 80, 0);
		write_dmg_tile_in_screen(screen, tilea, 64, 90, 0);
		write_dmg_tile_in_screen(screen, tileb, 64, 100, 0);
		write_dmg_tile_in_screen(screen, tilec, 64, 110, 0);
		write_dmg_tile_in_screen(screen, tiled, 64, 120, 0);
		write_dmg_tile_in_screen(screen, tilee, 64, 130, 0);

		write_dmg_tile_in_screen(screen, tile11, 64, 0, 10);
		write_dmg_tile_in_screen(screen, tile12, 64, 10, 10);
		write_dmg_tile_in_screen(screen, tile13, 64, 20, 10);
		write_dmg_tile_in_screen(screen, tile14, 64, 30, 10);
		write_dmg_tile_in_screen(screen, tile15, 64, 40, 10);
		write_dmg_tile_in_screen(screen, tile16, 64, 50, 10);
		write_dmg_tile_in_screen(screen, tile17, 64, 60, 10);
		write_dmg_tile_in_screen(screen, tile18, 64, 70, 10);
		write_dmg_tile_in_screen(screen, tile19, 64, 80, 10);
		write_dmg_tile_in_screen(screen, tile1a, 64, 90, 10);
		write_dmg_tile_in_screen(screen, tile1b, 64, 100, 10);
		write_dmg_tile_in_screen(screen, tile1c, 64, 110, 10);
		write_dmg_tile_in_screen(screen, tile1d, 64, 120, 10);
		write_dmg_tile_in_screen(screen, tile1e, 64, 130, 10);

		write_dmg_tile_in_screen(screen, tile21, 64, 0, 20);
		write_dmg_tile_in_screen(screen, tile22, 64, 10, 20);
		write_dmg_tile_in_screen(screen, tile23, 64, 20, 20);
		write_dmg_tile_in_screen(screen, tile24, 64, 30, 20);
		write_dmg_tile_in_screen(screen, tile25, 64, 40, 20);
		write_dmg_tile_in_screen(screen, tile26, 64, 50, 20);
		write_dmg_tile_in_screen(screen, tile27, 64, 60, 20);
		write_dmg_tile_in_screen(screen, tile28, 64, 70, 20);
		write_dmg_tile_in_screen(screen, tile29, 64, 80, 20);
		write_dmg_tile_in_screen(screen, tile2a, 64, 90, 20);
		write_dmg_tile_in_screen(screen, tile2b, 64, 100, 20);
		write_dmg_tile_in_screen(screen, tile2c, 64, 110, 20);
		write_dmg_tile_in_screen(screen, tile2d, 64, 120, 20);
		write_dmg_tile_in_screen(screen, tile2e, 64, 130, 20);

		write_dmg_tile_in_screen(screen, tile31, 64, 0, 30);
		write_dmg_tile_in_screen(screen, tile32, 64, 10, 30);
		write_dmg_tile_in_screen(screen, tile33, 64, 20, 30);
		write_dmg_tile_in_screen(screen, tile34, 64, 30, 30);
		write_dmg_tile_in_screen(screen, tile35, 64, 40, 30);
		write_dmg_tile_in_screen(screen, tile36, 64, 50, 30);
		write_dmg_tile_in_screen(screen, tile37, 64, 60, 30);
		write_dmg_tile_in_screen(screen, tile38, 64, 70, 30);
		write_dmg_tile_in_screen(screen, tile39, 64, 80, 30);
		write_dmg_tile_in_screen(screen, tile3a, 64, 90, 30);
		write_dmg_tile_in_screen(screen, tile3b, 64, 100, 30);
		write_dmg_tile_in_screen(screen, tile3c, 64, 110, 30);
		write_dmg_tile_in_screen(screen, tile3d, 64, 120, 30);
		write_dmg_tile_in_screen(screen, tile3e, 64, 130, 30);

		write_dmg_tile_in_screen(screen, tile41, 64, 0, 40);
		write_dmg_tile_in_screen(screen, tile42, 64, 10, 40);
		write_dmg_tile_in_screen(screen, tile43, 64, 20, 40);
		write_dmg_tile_in_screen(screen, tile44, 64, 30, 40);
		write_dmg_tile_in_screen(screen, tile45, 64, 40, 40);
		write_dmg_tile_in_screen(screen, tile46, 64, 50, 40);
		write_dmg_tile_in_screen(screen, tile47, 64, 60, 40);
		write_dmg_tile_in_screen(screen, tile48, 64, 70, 40);
		write_dmg_tile_in_screen(screen, tile49, 64, 80, 40);
		write_dmg_tile_in_screen(screen, tile4a, 64, 90, 40);
		write_dmg_tile_in_screen(screen, tile4b, 64, 100, 40);
		write_dmg_tile_in_screen(screen, tile4c, 64, 110, 40);
		write_dmg_tile_in_screen(screen, tile4d, 64, 120, 40);
		write_dmg_tile_in_screen(screen, tile4e, 64, 130, 40);

		write_dmg_tile_in_screen(screen, tile51, 64, 0, 50);
		write_dmg_tile_in_screen(screen, tile52, 64, 10, 50);
		write_dmg_tile_in_screen(screen, tile53, 64, 20, 50);
		write_dmg_tile_in_screen(screen, tile54, 64, 30, 50);
		write_dmg_tile_in_screen(screen, tile55, 64, 40, 50);
		write_dmg_tile_in_screen(screen, tile56, 64, 50, 50);
		write_dmg_tile_in_screen(screen, tile57, 64, 60, 50);
		write_dmg_tile_in_screen(screen, tile58, 64, 70, 50);
		write_dmg_tile_in_screen(screen, tile59, 64, 80, 50);
		write_dmg_tile_in_screen(screen, tile5a, 64, 90, 50);
		write_dmg_tile_in_screen(screen, tile5b, 64, 100, 50);
		write_dmg_tile_in_screen(screen, tile5c, 64, 110, 50);
		write_dmg_tile_in_screen(screen, tile5d, 64, 120, 50);
		write_dmg_tile_in_screen(screen, tile5e, 64, 130, 50);

		write_dmg_tile_in_screen(screen, tile61, 64, 0, 60);
		write_dmg_tile_in_screen(screen, tile62, 64, 10, 60);
		write_dmg_tile_in_screen(screen, tile63, 64, 20, 60);
		write_dmg_tile_in_screen(screen, tile64, 64, 30, 60);
		write_dmg_tile_in_screen(screen, tile65, 64, 40, 60);
		write_dmg_tile_in_screen(screen, tile66, 64, 50, 60);
		write_dmg_tile_in_screen(screen, tile67, 64, 60, 60);
		write_dmg_tile_in_screen(screen, tile68, 64, 70, 60);
		write_dmg_tile_in_screen(screen, tile69, 64, 80, 60);
		write_dmg_tile_in_screen(screen, tile6a, 64, 90, 60);
		write_dmg_tile_in_screen(screen, tile6b, 64, 100, 60);
		write_dmg_tile_in_screen(screen, tile6c, 64, 110, 60);
		write_dmg_tile_in_screen(screen, tile6d, 64, 120, 60);
		write_dmg_tile_in_screen(screen, tile6e, 64, 130, 60);

		write_dmg_tile_in_screen(screen, tile71, 64, 0, 70);
		write_dmg_tile_in_screen(screen, tile72, 64, 10, 70);
		write_dmg_tile_in_screen(screen, tile73, 64, 20, 70);
		write_dmg_tile_in_screen(screen, tile74, 64, 30, 70);
		write_dmg_tile_in_screen(screen, tile75, 64, 40, 70);
		write_dmg_tile_in_screen(screen, tile76, 64, 50, 70);
		write_dmg_tile_in_screen(screen, tile77, 64, 60, 70);
		write_dmg_tile_in_screen(screen, tile78, 64, 70, 70);
		write_dmg_tile_in_screen(screen, tile79, 64, 80, 70);
		write_dmg_tile_in_screen(screen, tile7a, 64, 90, 70);
		write_dmg_tile_in_screen(screen, tile7b, 64, 100, 70);
		write_dmg_tile_in_screen(screen, tile7c, 64, 110, 70);
		write_dmg_tile_in_screen(screen, tile7d, 64, 120, 70);
		write_dmg_tile_in_screen(screen, tile7e, 64, 130, 70);

		write_dmg_tile_in_screen(screen, tile81, 64, 0, 80);
		write_dmg_tile_in_screen(screen, tile82, 64, 10, 80);
		write_dmg_tile_in_screen(screen, tile83, 64, 20, 80);
		write_dmg_tile_in_screen(screen, tile84, 64, 30, 80);
		write_dmg_tile_in_screen(screen, tile85, 64, 40, 80);
		write_dmg_tile_in_screen(screen, tile86, 64, 50, 80);
		write_dmg_tile_in_screen(screen, tile87, 64, 60, 80);
		write_dmg_tile_in_screen(screen, tile88, 64, 70, 80);
		write_dmg_tile_in_screen(screen, tile89, 64, 80, 80);
		write_dmg_tile_in_screen(screen, tile8a, 64, 90, 80);
		write_dmg_tile_in_screen(screen, tile8b, 64, 100, 80);
		write_dmg_tile_in_screen(screen, tile8c, 64, 110, 80);
		write_dmg_tile_in_screen(screen, tile8d, 64, 120, 80);
		write_dmg_tile_in_screen(screen, tile8e, 64, 130, 80);

		write_dmg_tile_in_screen(screen, tile91, 64, 0, 90);
		write_dmg_tile_in_screen(screen, tile92, 64, 10, 90);
		write_dmg_tile_in_screen(screen, tile93, 64, 20, 90);
		write_dmg_tile_in_screen(screen, tile94, 64, 30, 90);
		write_dmg_tile_in_screen(screen, tile95, 64, 40, 90);
		write_dmg_tile_in_screen(screen, tile96, 64, 50, 90);
		write_dmg_tile_in_screen(screen, tile97, 64, 60, 90);
		write_dmg_tile_in_screen(screen, tile98, 64, 70, 90);
		write_dmg_tile_in_screen(screen, tile99, 64, 80, 90);
		write_dmg_tile_in_screen(screen, tile9a, 64, 90, 90);
		write_dmg_tile_in_screen(screen, tile9b, 64, 100, 90);
		write_dmg_tile_in_screen(screen, tile9c, 64, 110, 90);
		write_dmg_tile_in_screen(screen, tile9d, 64, 120, 90);
		write_dmg_tile_in_screen(screen, tile9e, 64, 130, 90);

		write_dmg_tile_in_screen(screen, tilea1, 64, 0, 100);
		write_dmg_tile_in_screen(screen, tilea2, 64, 10, 100);
		write_dmg_tile_in_screen(screen, tilea3, 64, 20, 100);
		write_dmg_tile_in_screen(screen, tilea4, 64, 30, 100);
		write_dmg_tile_in_screen(screen, tilea5, 64, 40, 100);
		write_dmg_tile_in_screen(screen, tilea6, 64, 50, 100);
		write_dmg_tile_in_screen(screen, tilea7, 64, 60, 100);
		write_dmg_tile_in_screen(screen, tilea8, 64, 70, 100);
		write_dmg_tile_in_screen(screen, tilea9, 64, 80, 100);
		write_dmg_tile_in_screen(screen, tileaa, 64, 90, 100);
		write_dmg_tile_in_screen(screen, tileab, 64, 100, 100);
		write_dmg_tile_in_screen(screen, tileac, 64, 110, 100);
		write_dmg_tile_in_screen(screen, tilead, 64, 120, 100);
		write_dmg_tile_in_screen(screen, tileae, 64, 130, 100);

		put_screen(screen);
		usleep(1000000);
	}
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
