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


static void		put_screen(char	*screen)
{
	write(1, "\e[2J\e[H", 7);
	for (unsigned int i = 0; i < 160; i++)
	{
		write(1, screen + (i * (145 * 13 + 80)), (145 * 13));
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
			strcpy(screen + (((i / 2) + y) * (145 * 13 + 80)) + (13 * (x + 0)), color[ ((tile[i] & 0x80) >> 7) | ((tile[i + 1] & 0x80) >> 6) ]);
			strcpy(screen + (((i / 2) + y) * (145 * 13 + 80)) + (13 * (x + 1)), color[ ((tile[i] & 0x40) >> 6) | ((tile[i + 1] & 0x40) >> 5) ]);
			strcpy(screen + (((i / 2) + y) * (145 * 13 + 80)) + (13 * (x + 2)), color[ ((tile[i] & 0x20) >> 5) | ((tile[i + 1] & 0x20) >> 4) ]);
			strcpy(screen + (((i / 2) + y) * (145 * 13 + 80)) + (13 * (x + 3)), color[ ((tile[i] & 0x10) >> 4) | ((tile[i + 1] & 0x10) >> 3) ]);
			strcpy(screen + (((i / 2) + y) * (145 * 13 + 80)) + (13 * (x + 4)), color[ ((tile[i] & 0x08) >> 3) | ((tile[i + 1] & 0x08) >> 2) ]);
			strcpy(screen + (((i / 2) + y) * (145 * 13 + 80)) + (13 * (x + 5)), color[ ((tile[i] & 0x04) >> 2) | ((tile[i + 1] & 0x04) >> 1) ]);
			strcpy(screen + (((i / 2) + y) * (145 * 13 + 80)) + (13 * (x + 6)), color[ ((tile[i] & 0x02) >> 1) | ((tile[i + 1] & 0x02) << 0) ]);
			strcpy(screen + (((i / 2) + y) * (145 * 13 + 80)) + (13 * (x + 7)), color[ ((tile[i] & 0x01) >> 0) | ((tile[i + 1] & 0x01) << 1) ]);
			i += 2;
		}
	}
	else
	{
		return ;
	}
}

int		main(int argc, char *argv[])
{
	char			screen[160 * ((145 * 13) + 80)] = {0};
	char			*tile;
	unsigned int	len;
	unsigned int	x = 0, y = 0;

	if (argc != 2)
		return (1);
	tile = get_file_contents(argv[1], &len);
	if (!tile || (len & 0x0f))
	{
		fprintf(stderr, "ERROR (len - %u)\n", len);
		return (1);
	}
	while (len && y < 160)
	{
		write_dmg_tile_in_screen(screen, tile, 64, x, y);
		len -= 16;
		tile += 16;
		x += 8;
		if (x == 144)
		{
			x = 0;
			y += 8;
		}
	}

	put_screen(screen);
}
