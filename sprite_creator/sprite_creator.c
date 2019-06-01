#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdint.h>

/*
static void		put_file_contents(const char *file, const void *content, uint32_t length)
{
	const int		fd = open(file, O_WRONLY | O_TRUNC | O_CREAT, 0664);

	if (fd == -1)
	{
		fprintf(stderr, "can't open file %s\n", file);
		exit(1);
	}
	if (write(fd, content, length) != length)
	{
		fprintf(stderr, "write failure\n");
		exit(1);
	}
	close(fd);
}
*/

static char		*get_file_contents(const char *file, uint32_t *length)
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
	else if (filestat.st_size > 0x8000)
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

char	*convert_sprite(char *sp, unsigned int len, unsigned int *size)
{
	static char		sprite[4096] = {0};
	char			_0, _1, _2, _3;
	unsigned int	i;
	unsigned int	bit;

	_0 = sp[0] & sp[1];
	_1 = sp[2] & sp[3];
	_2 = sp[4] & sp[5];
	_3 = sp[6] & sp[7];
	len -= 9;
	sp += 9;

	i = 0;
	bit	= 0x80U;
	*size = 2;
	while (len)
	{
		if (*sp == _1) {
			sprite[i] |= bit;
		}
		else if (*sp == _2) {
			sprite[i + 1] |= bit;
		}
		else if (*sp == _3) {
			sprite[i] |= bit;
			sprite[i + 1] |= bit;
		}
		else if (*sp != '\n' && *sp != _0) {
			fprintf(stderr, "ERROR\n");
			exit(1);
		}

		if (*sp == '\n')
		{
			bit = 0x80U;
			sp++;
			if (--len == 0)
				break;
			i += 2;
			*size += 2;
		}
		else
		{
			sp += 2;
			len -= 2;
			bit >>= 1U;
		}
	}
	return (sprite);
}

void	put_sprite(char *sprite, int f, unsigned int len)
{
	if (f)
	{
		write(1, sprite, len);
		return ;
	}
	printf("char	sprite[] = {\n\t0x%hhx", *sprite);
	sprite++;
	len--;
	while (len)
	{
		(((unsigned long)sprite & 0x7U)) ?
			printf(", 0x%hhx", *sprite) :
			printf(",\n\t0x%hhx", *sprite);
		sprite++;
		len--;
	}
	puts("\n};");
}

int		main(int argc, char *argv[])
{
	char			*sprite;
	char			*gb_sprite;
	unsigned int	size;
	unsigned int	len;

	if (argc != 3)
	{
		fprintf(stderr, "%s \"sprite.gbs\"\n", argv[0]);
		return (1);
	}

	if ((sprite = get_file_contents(argv[1], &size)) == NULL)
		return (1);
	gb_sprite = convert_sprite(sprite, size, &len);
	put_sprite(gb_sprite, argc == 3, len);
	return (0);
}
