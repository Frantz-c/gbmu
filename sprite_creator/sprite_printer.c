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
#include <malloc/malloc.h>


#define COL0		"\e[48;5;255m  "
#define COL1		"\e[48;5;249m  "
#define COL2		"\e[48;5;242m  "
#define COL3		"\e[48;5;235m  "


#define REAL_W		(64 * 13 + 5)
#define SCREEN_W	(64)
#define SCREEN_H	(144)
#define SCREEN_SIZE	(SCREEN_H * REAL_W)

// 18 * 20
#define TILE_OFFSET(x, y)	\
	(((y) * REAL_W * 8) + (13 * 8 * (x)))

// 144 * 160
#define SCREEN_OFFSET(x, y)	\
	(((y) * REAL_W) + (13 * (x)))


static char		*get_file_contents(const char *file, uint32_t *length)
{
	char			*content;
	const int		fd = open(file, O_RDONLY);
	struct stat		filestat;

	*length = 0;
	if (fd == -1)
	{
		fprintf(stderr, "can't open file %s\n", file);
		return (NULL);
	}
	lstat(file, &filestat);
	if (filestat.st_size == 0)
	{
		close(fd);
		fprintf(stderr, "empty file\n");
		return (NULL);
	}
	else if (filestat.st_size > 0x8000)
	{
		close(fd);
		fprintf(stderr, "too heavy file\n");
		return (NULL);
	}
	if ((content = valloc(filestat.st_size)) == NULL)
	{
		close(fd);
		fprintf(stderr, "malloc fatal error\n");
		return (NULL);
	}
	*length = read(fd, content, filestat.st_size);
	close(fd);
	return (content);
}

static void		put_screen(char	*screen, unsigned int y)
{
	//write(1, "\e[2J\e[H", 7);
	for (unsigned int i = 0; i < y; i++)
	{
		memcpy(screen + (REAL_W - 5 + i * REAL_W), "\e[0m", 4);
		screen[i * REAL_W + REAL_W - 1] = '\n';
	}
	write(1, screen, SCREEN_SIZE - ((SCREEN_H - y) * REAL_W));
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

static void		write_tile(char *screen, char *tile, unsigned int x, unsigned int y)
{
	unsigned int		i = 0;
#if BYTE_ORDER == LITTLE_ENDIAN
	static const uint16_t	color2[4] = {'5' | ('5' << 8), '4' | ('9' << 8), '4' | ('2' << 8), '3' | ('5' << 8)};
#else
	static const uint16_t	color2[4] = {'5' | ('5' << 8), '9' | ('4' << 8), '2' | ('4' << 8), '5' | ('3' << 8)};
#endif

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
		i += 2;
	}
}


int		main(int argc, char *argv[])
{
	char			screen[SCREEN_SIZE];
	char			*tile;
	unsigned int	len;
	unsigned int	x = 0, y = 0;

	if (argc != 2)
		return (1);
	tile = get_file_contents(argv[1], &len);
	if (!tile || (len & 0x0f))
	{
		fprintf(stderr, "ERROR (len = %u Bytes)\n", len);
		return (1);
	}
	
	init_screen(screen);

	while (len && y < SCREEN_H)
	{
		write_tile(screen, tile, x, y);
		len -= 16;
		tile += 16;
		x += 8;
		if (x == SCREEN_W)
		{
			x = 0;
			y += 8;
		}
	}

	put_screen(screen, y + 8);
	return (0);
}
