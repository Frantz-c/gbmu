/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   disassemble_region.c                             .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/22 22:17:53 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/06/03 14:06:39 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "disassemble_table.c"

static inline const char		*left_trim(const char *s)
{
	while (*s == ' ' || *s == '\t')
		s++;
	if (*s == '0' && s[1] == 'x')
	{
		s += 2;
		while (*s == '0')
			s++;
		return (s);
	}
	return (NULL);
}

static unsigned int				get_base_value(char c)
{
	if (c >= 'a' && c <= 'f')
		return (c - ('a' - 10));
	if (c >= 'A' && c <= 'F')
		return (c - ('a' - 10));
	return (c - '0');
}

static inline unsigned int		ft_strtoi(const char *s)
{
	unsigned int	n;

	n = 0;
	while (1)
	{
		if (*s > 'f' || (*s > 'F' && *s < 'a')
				|| (*s > '9' && *s < 'A') || *s < '0')
			break ;
		n *= 16;
		n += get_base_value(*(s++));
	}
	return (n);
}

extern inline unsigned int		atoi_hexa(const char *s)
{
	if ((s = left_trim(s)) == NULL)
		return (0);
	return (ft_strtoi(s));
}


static char		*itoazx(int n, int size)
{
	static char	s[8];
	int			i = 6;
	int			negative = 0;

	if (n < 0)
	{
		negative++;
		n = 0 - n;
	}

	s[7] = '\0';
	while (n)
	{
		s[i] = ((n % 16) + '0');
		if (s[i] > '9')
			s[i] += 39;
		i--;
		n /= 16;
	}
	while (i > 4 - size)
	{
		s[i--] = '0';
	}
	s[i + 2] = 'x';
	if (negative)
	{
		s[i] = '-';
		return (s + i);
	}
	return (s + i + 1);
}

int				get_int16_from_little_endian(void *memory)
{
#if BYTE_ORDER == LITTLE_ENDIAN
	return (*(int16_t*)memory);
#else
	return ((int16_t)(*(unsigned char*)memory + *((unsigned char*)(memory + 1)) << 8));
#endif
}

unsigned int	get_uint16_from_little_endian(void *memory)
{
#if BYTE_ORDER == LITTLE_ENDIAN
	return (*(uint16_t*)memory);
#else
	return (*(unsigned char*)memory + *((unsigned char*)(memory + 1)) << 8);
#endif
}

static void		*fmt_strcpy(char *dst, char *src,
							enum e_operand_type optype, void *bin, int *i)
{
	int		value;
	char	*numeric = NULL;

	while (*src != '*' && (*(dst++) = *(src)))
		src++;
	if (*src == 0)
		return (bin);
	src++;

	if (optype == IMM8)
	{
		value = *((int8_t*)bin);
		numeric = itoazx(value, 2);
		bin += 1;
	}
	else if (optype == IMM16)
	{
		value = get_int16_from_little_endian(bin);
		numeric = itoazx(value, 4);
		bin += 2;
	}
	else if (optype == ADDR8)
	{
		value = *((uint8_t*)bin);
		numeric = itoazx(value, 2);
		bin += 1;
	}
	else if (optype == ADDR16)
	{
		value = get_uint16_from_little_endian(bin);
		numeric = itoazx(value, 4);
		bin += 2;
	}
	else
	{
		fprintf(stderr, "\e[0;31mFATAL ERROR\e[0m\n");
		exit(1);
	}
	strcpy(dst, numeric);
	dst += strlen(numeric);
	while ((*(dst++) = *(src++)));
	if (*numeric == '-')
		(*i)++;
	return (bin);
}

// peut potentiellement depasser la fin d'1 ou 2 octets
int				disassemble_region(char **disassembled_code, void *binary,
									unsigned int start, unsigned int end)
{
	char		*binstart = (char *)binary;
	binary	+= start;
	end		-= start;
	const void	*ptr_end = binary + end;
	char		*buf = malloc(1024 + 1);
	int			buflen = 1024;
	int			i = 0;
	int			len;

	*disassembled_code = NULL;
	while (binary < ptr_end)
	{
		if (i + 16 >= buflen)
		{
			buflen += 1024;
			buf = realloc(buf, buflen);
			if (!buf) {
				fprintf(stderr, "realloc fatal error\n");
				exit(1);
			}
		}
		i += sprintf(buf + i, "0x%x:  ", (unsigned int)((char *)binary - binstart));
		if (*(unsigned char*)binary == 0xcbU)
		{
			len = strlen(cb_opcodes[((unsigned char*)binary)[1]].inst);
			if (i + len >= buflen)
			{
				buflen += 1024;
				buf = realloc(buf, buflen);
				if (!buf) {
					fprintf(stderr, "realloc fatal error\n");
					exit(1);
				}
			}
			strcpy(buf + i, cb_opcodes[((unsigned char*)binary)[1]].inst);
			i += len;
			binary += 2;
		}
		else if (opcodes[*(unsigned char*)binary].inst)
		{
			len = strlen(opcodes[*(unsigned char*)binary].inst);
			if (opcodes[*(unsigned char*)binary].optype)
			{
				switch (opcodes[*(unsigned char*)binary].optype)
				{
					case IMM8:
					case ADDR8: len += 3; break;
					case IMM16:
					case ADDR16: len += 5;
					default: break;
				}
			}
			if (i + len >= buflen)
			{
				buflen += 1024;
				buf = realloc(buf, buflen);
				if (!buf) {
					fprintf(stderr, "realloc fatal error\n");
					exit(1);
				}
			}
			binary = fmt_strcpy(buf + i, opcodes[*(unsigned char*)binary].inst,
								opcodes[*(unsigned char*)binary].optype, binary + 1, &i);
			i += len;
		}
		else
		{
			if (i + 40 >= buflen)
			{
				buflen += 41;
				buf = realloc(buf, buflen);
				if (!buf) {
					fprintf(stderr, "realloc fatal error\n");
					exit(1);
				}
			}
			i += sprintf(buf + i, "\e[1;31mILLEGAL INSTRUCTION: (0x%hhx)\e[0m\n", *(uint8_t*)binary);
			binary++;
			//break;
		}
	}
	*disassembled_code = buf;
	return (0);
}

char	*get_file_contents(const char *file, int *length)
{
	char		*content;
	const int	fd = open(file, O_RDONLY);
	struct stat	filestat;

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
	if ((content = malloc(filestat.st_size + 1)) != NULL)
	{
		*length = read(fd, content, filestat.st_size);
		close(fd);
	}
	return (content);
}

void	print_data(const char *filename, int len, int readl)
{
	printf("\e[1mfile  : \e[0;33m%s\e[0m\n\e[1mlength: \e[0;33m%d octets\e[0m\n\e[1mdisassembled size: \e[0;33m%d\e[0m\n\n", filename, len, readl);
}

int main(int ac, char *av[])
{
	char	*dis;
	char	*file;
	int		len;
	int		start, end;

	if (ac < 2 || ac > 4)
	{
		fprintf(stderr, "%s \"file\" [start offset] [end offset]\n", av[0]);
		return (1);
	}
	if ((file = get_file_contents(av[1], &len)) == NULL)
		return (1);
	start = 0;
	end = len;
	if (ac > 2)
	{
		if (av[2][0] == '0' && av[2][1] == 'x')
			start = atoi_hexa(av[2]);
		else
			start = atoi(av[2]);
	}
	if (ac > 3)
	{
		if (av[3][0] == '0' && av[3][1] == 'x')
			end = atoi_hexa(av[3]);
		else
			end = atoi(av[3]);
	}
	if (len < end)
	{
		fprintf(stderr, "end offset (%d) > file end (%d)\n", end, len);
		return (1);
	}
	if (start > end)
	{
		fprintf(stderr, "start offset (%d) > end offset (%d)\n", start, end);
		return (1);
	}
	if (start < 0 || end < 0)
		return (1);

	print_data(av[1], len, end - start);
	disassemble_region(&dis, file, start, end);

	if (dis)
	{
		puts(dis);
		free(dis);
	}
	return (0);
}
