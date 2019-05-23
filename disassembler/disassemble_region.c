/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   disassemble_region.c                             .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/22 22:17:53 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/05/23 16:48:22 by fcordon     ###    #+. /#+    ###.fr     */
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
		puts("ERROR\n");
		exit (1);
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
			if (i + 45 >= buflen)
			{
				buflen += 46;
				buf = realloc(buf, buflen);
				if (!buf) {
					fprintf(stderr, "realloc fatal error\n");
					exit(1);
				}
			}
			sprintf(buf + i, "\e[1;31mILLEGAL INSTRUCTION: \e[0mstop (0x%hhx)\e[0m\n", *(uint8_t*)binary);
			break;
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
//	char	file[] = "\01\02\03\04\x77\xc4\x2a\x00\x0e\x2a\x1e\xff\xcb\x30\01\02\03\04";
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
		start = atoi(av[2]);
	if (ac > 3)
		end = atoi(av[3]);
	if (len < end)
	{
		fprintf(stderr, "end offset (%d) > file end (%d)\n", end, len);
		return (1);
	}
	if (start > end)
	{
		fprintf(stderr, "start offset (%d) > file end (%d)\n", start, len);
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
