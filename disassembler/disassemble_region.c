/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   disassemble_region.c                             .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/22 22:17:53 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/17 18:56:59 by fcordon     ###    #+. /#+    ###.fr     */
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

typedef struct	cartridge_s
{
	uint8_t		_0x00c3[2];
	uint8_t		program_start[2];
	uint8_t		logo[48];
	uint8_t		game_title[11];
	uint8_t		game_code[4];
	uint8_t		cgb_support;
	uint8_t		maker_code[2];
	uint8_t		sgb_support;
	uint8_t		cart_type;
	uint8_t		rom_size;
	uint8_t		ram_size;
	uint8_t		destination;
	uint8_t		_0x33;	//0x33
	uint8_t		version;
	uint8_t		complement_check;
	uint8_t		check_sum[2];
}
cartridge_t;


const char	*get_cgb_support(uint8_t c)
{
	switch (c)
	{
		case 0x00: return ("incompatible");
		case 0x80: return ("compatible");
		case 0xc0: return ("exclusive");
	}
	return ("ERROR");
}

const char	*get_sgb_support(uint8_t c)
{
	switch (c)
	{
		case 0x00: return ("supported");
		case 0x03: return ("not supported");
	}
	return ("ERROR");
}

const char	*get_cart_type(uint8_t c)
{
	switch (c)
	{
		case 0x00: return ("ROM");
		case 0x01: return ("MBC-1");
		case 0x02: return ("MBC-1 SRAM");
		case 0x03: return ("MBC-1 SRAM BATTERY");
		case 0x04: return ("0x04");
		case 0x05: return ("MBC-2");
		case 0x06: return ("MBC-2 BATTERY");
		case 0x07: return ("0x07");
		case 0x08: return ("SRAM");
		case 0x09: return ("SRAM BATTERY");
		case 0x0b: return ("MMM01");
		case 0x0c: return ("MMM01 SRAM");
		case 0x0d: return ("MMM01 SRAM BATTERY");
		case 0x0f: return ("MBC-3 RTC BATTERY");
		case 0x10: return ("MBC-3 RTC SRAM BATTERY");
		case 0x11: return ("MBC-3");
		case 0x12: return ("MBC-3 SRAM");
		case 0x13: return ("MBC-3 SRAM BATTERY");
		case 0x19: return ("MBC-5");
		case 0x1a: return ("MBC-5 SRAM");
		case 0x1b: return ("MBC-5 SRAM BATTERY");
		case 0x1c: return ("MBC-5 Rumble");
		case 0x1d: return ("MBC-5 Rumble SRAM");
		case 0x1e: return ("MBC-5 Rumble SRAM BATTERY");
	}
	return ("ERROR");
}
const char	*get_rom_size(uint8_t c)
{
	switch (c)
	{
		case 0x00: return ("0 banks");
		case 0x01: return ("4 banks");
		case 0x02: return ("8 banks");
		case 0x03: return ("16 banks");
		case 0x04: return ("32 banks");
		case 0x05: return ("64 banks");
		case 0x06: return ("128 banks");
		case 0x07: return ("256 banks");
		case 0x08: return ("512 banks");
	}
	return ("ERROR");
}
/*
00h - None
01h - 2 KBytes ??
02h - 8 Kbytes
03h - 32 KBytes (4 banks of 8KBytes each)
04h - 128 KBytes (16 banks of 8KBytes each)
05h - 64 KBytes (8 banks of 8KBytes each)
*/
const char	*get_ram_size(uint8_t c)
{
	switch (c)
	{
		case 0x00: return ("0 banks");
		//case 0x01: return ("16 KBits (2 Kbyte)");
		case 0x02: return ("2 banks"); // ??
		case 0x03: return ("8 banks");
		case 0x04: return ("32 banks");
		case 0x05: return ("16 banks");
	}
	return ("ERROR");
}

const char	*get_destination(uint8_t c)
{
	switch (c)
	{
		case 0x00: return ("Japan");
		case 0x01: return ("Other than Japan");
	}
	return ("ERROR");
}



void		print_header_info(cartridge_t *cart)
{
	printf(
		"## header\n"
		".program_start	0x%hhx%hhx\n"
		".game_title	\"%.11s\"\n"
		".game_code		\"%.4s\"\n"
		".maker_code	\"%.2s\"\n"
		".cgb_support	\"%s\"\n"
		".sgb_support	\"%s\"\n"
		".cart_type		\"%s\"\n"
		".rom_size		\"%s\"\n"
		".ram_size		\"%s\"\n"
		".destination	\"%s\"\n"
		".version		%u\n\n",
		cart->program_start[1], cart->program_start[0],
		cart->game_title,
		cart->game_code,
		cart->maker_code,
		get_cgb_support(cart->cgb_support),
		get_sgb_support(cart->sgb_support),
		get_cart_type(cart->cart_type),
		get_rom_size(cart->rom_size),
		get_ram_size(cart->ram_size),
		get_destination(cart->destination),
		cart->version
	);
}

int				disassemble_region(char **disassembled_code, void *binary,
									unsigned int start, unsigned int end, unsigned int dontpaddr)
{
	char			*binstart = (char *)binary;
	binary	+= start;
	end		-= start;
	const void		*ptr_end = binary + end;
	char			*buf = malloc(1024 + 1);
	int				buflen = 1024;
	int				i = 0;
	int				len;
	unsigned int	n_nop = 0;

	*disassembled_code = NULL;
	while (binary < ptr_end)
	{
		if (i + 56 >= buflen)
		{
			buflen += 1024;
			buf = realloc(buf, buflen);
			if (!buf) {
				fprintf(stderr, "realloc fatal error\n");
				exit(1);
			}
		}

		if (dontpaddr == 0)
			i += sprintf(buf + i, "0x%x:  ", (unsigned int)((char *)binary - binstart));
		else if (((unsigned int)((char *)binary - binstart)) % 0x4000 == 0)
		{
			i += sprintf(buf + i, "\n\n## address = 0x%X\n.bank	%u\n\n",
					((unsigned int)((char *)binary - binstart)),
					((unsigned int)((char *)binary - binstart)) / 0x4000);
		}
		else if ((unsigned int)((char *)binary - binstart) == 0x100)
		{
			if (n_nop)
			{
				if (n_nop < 9)
				{
					while (n_nop)
					{

						i += sprintf(buf + i, "nop\n");
						n_nop--;
					}
				}
				else
				{
					i += sprintf(
						buf + i,
						"\n.bank	%u,	0x%x\n\n", 
						(unsigned int)((char *)binary - binstart) / 0x4000,
						(unsigned int)((char *)binary - binstart) % 0x4000
					);
					n_nop = 0;
				}
			}

			if (ptr_end - binary > 0x4F)
			{
				print_header_info(binary);
				binary += 0x50;
				i += sprintf(buf + i, "\n.bank	0x150\n\n");
				continue;
			}
		}

		if (*(unsigned char*)binary == 0)
		{
			binary++;
			n_nop++;
			continue;
		}
		else if (n_nop)
		{
			if (n_nop < 9)
			{
				while (n_nop)
				{

					i += sprintf(buf + i, "nop\n");
					n_nop--;
				}
			}
			else
			{
				i += sprintf(
					buf + i,
					"\n.bank	%u,	0x%x\n\n", 
					(unsigned int)((char *)binary - binstart) / 0x4000,
					(unsigned int)((char *)binary - binstart) % 0x4000
				);
				n_nop = 0;
			}
		}

		if (*(unsigned char*)binary == 0xcbU)
		{
			if (binary + 1 < ptr_end)
				break;
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
			unsigned char	opsiz = 0;

			if (opcodes[*(unsigned char*)binary].optype)
			{
				if (opcodes[*(unsigned char*)binary].optype == IMM8)
				{
					if (binary + 1 > ptr_end)
						break;
					opsiz = 1;
				}
				else if (opcodes[*(unsigned char*)binary].optype == IMM16)
				{
					if (binary + 2 > ptr_end)
						break;
					opsiz = 2;
				}
			}



			len = strlen(opcodes[*(unsigned char*)binary].inst);
			if (i + len >= buflen)
			{
				buflen += 1024;
				buf = realloc(buf, buflen);
				if (!buf) {
					fprintf(stderr, "realloc fatal error\n");
					exit(1);
				}
			}

			i += sprintf(buf + i, opcodes[*(unsigned char*)binary].inst, *(int*)(binary + 1));
			binary += opsiz;
			binary++;
		}
		else
		{
			if (i + 12 >= buflen)
			{
				buflen += 1024;
				buf = realloc(buf, buflen);
				if (!buf) {
					fprintf(stderr, "realloc fatal error\n");
					exit(1);
				}
			}
			i += sprintf(buf + i, ".byte	0x%0.2hhx\n", *(uint8_t*)binary);
			binary++;
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
	char				*dis;
	char				*file;
	int					len;
	int					start, end;
	unsigned char		i = 0;

	if (ac < 2 || ac > 5)
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
		if (av[2][0] == '-' && av[2][1] == 'c' && av[2][2] == 0)
			i++;
	}

	if (ac > 2 + i)
	{
		if (av[2 + i][0] == '0' && av[2 + i][1] == 'x')
			start = atoi_hexa(av[2 + i]);
		else
			start = atoi(av[2 + i]);
	}
	if (ac > 3 + i)
	{
		if (av[3 + i][0] == '0' && av[3 + i][1] == 'x')
			end = atoi_hexa(av[3 + i]);
		else
			end = atoi(av[3 + i]);
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
	disassemble_region(&dis, file, start, end, i);

	if (dis)
	{
		puts(dis);
		free(dis);
	}
	return (0);
}
