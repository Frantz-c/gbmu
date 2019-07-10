/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   log_print.c                                      .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/09 14:32:05 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/09 15:59:30 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "memory_map.h"
#include "../disassembler/disassemble_table.c"

static char	*get_bin(unsigned char n)
{
	static char					buf[16][9] = {{0}};
	static uint32_t				cur = 0;
	unsigned char				curs;
	unsigned int				i = 0;

	if (cur == 16)
		cur = 0;
	curs = 1UL << 7;
	while (curs)
	{
		buf[cur][i++] = (curs & n) ? '1' : '0';
		curs >>= 1;
	}
	buf[cur++][i] = 0;
	return (buf[cur - 1]);
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
#if __BYTE_ORDER == __ORDER_LITTLE_ENDIAN
	return (*(int16_t*)memory);
#else
	return ((int16_t)(*(unsigned char*)memory + *((unsigned char*)(memory + 1)) << 8));
#endif
}

unsigned int	get_uint16_from_little_endian(void *memory)
{
#if __BYTE_ORDER == __ORDER_LITTLE_ENDIAN
	return (*(uint16_t*)memory);
#else
	return (*(unsigned char*)memory + *((unsigned char*)(memory + 1)) << 8);
#endif
}

static char __attribute__((unused))	*fmt_strcpy(char *src,
							enum e_operand_type optype, void *bin)
{
	static char buf[512];
	char		*dst = buf;
	int			value;
	char		*numeric = NULL;

	while (*src != '*' && (*(dst++) = *(src)))
		src++;
	if (*src == 0)
		return (buf);
	src++;

	if (optype == IMM8)
	{
		value = *((int8_t*)bin);
		numeric = itoazx(value, 2);
	}
	else if (optype == IMM16)
	{
		value = get_int16_from_little_endian(bin);
		numeric = itoazx(value, 4);
	}
	else if (optype == ADDR8)
	{
		value = *((uint8_t*)bin);
		numeric = itoazx(value, 2);
	}
	else if (optype == ADDR16)
	{
		value = get_uint16_from_little_endian(bin);
		numeric = itoazx(value, 4);
	}
	else
	{
		fprintf(stderr, "\e[0;31mFATAL ERROR\e[0m\n");
		exit(1);
	}
	strcpy(dst, numeric);
	dst += strlen(numeric);
	while ((*(dst++) = *(src++)));
	return (buf);
}

int main(int argc, char *argv[])
{
	unsigned char	data[16], data2[16];//, backup[16];
	int				fd, fd2;
	unsigned int	l1, l2;
	int				first = 1;
	int				_exit = 0;
	
	if (argc != 3 && argc != 4)
		return(1);
	if (argc == 4)
		_exit = 1;
	if ((fd = open(argv[1], O_RDONLY)) == -1)
		return(1);
	if ((fd2 = open(argv[2], O_RDONLY)) == -1)
		return(1);


	while ((l1 = read(fd, data, 16)) == 16 && (l2 = read(fd2, data2, 16)) == 16)
	{
		if (memcmp(data, data2, 15))
		{
			if (data[8] != data2[8] || data[9] != data2[9])
				puts("~~PC_DIFF~~");
			printf
			(
				"===DIFF===\n\n%s:\n\nA = %hhx, F = %hhx(%.4s), B = %hhx, C = %hhx,\nD = %hhx, E = %hhx, H = %hhx, L = %hhx,\nPC = %hx, SP = %hx\ninstruction = %s\n\n",
				argv[1], data[0], data[1], get_bin(data[1]), data[2], data[3], data[4], data[5], data[6], data[7], (unsigned short)((data[8] << 8) | data[9]), (unsigned short)((data[10] << 8) | data[11]),
				data[12] == 0xcb ? cb_opcodes[data[13]].inst : fmt_strcpy(opcodes[data[12]].inst, opcodes[data[12]].optype, data + 13)
			);
			printf
			(
				"%s:\n\nA = %hhx, F = %hhx(%.4s), B = %hhx, C = %hhx,\nD = %hhx, E = %hhx, H = %hhx, L = %hhx,\nPC = %hx, SP = %hx\ninstruction = %s\n\n",
				argv[2], data2[0], data2[1], get_bin(data2[1]), data2[2], data2[3], data2[4], data2[5], data2[6], data2[7], (unsigned short)((data2[8] << 8) | data2[9]), (unsigned short)((data2[10] << 8) | data2[11]),
				data2[12] == 0xcb ? cb_opcodes[data2[13]].inst : fmt_strcpy(opcodes[data2[12]].inst, opcodes[data2[12]].optype, data2 + 13)
			);
			while (data[8] != data2[8] || data[9] != data2[9])
			{
				if (read(fd2, data2, 16) != 16)
					return (1);
			}
			if (_exit)
				return (1);
		}
		else
		{
			printf
			(
				"%s:\n\nA = %hhx, F = %hhx(%.4s), B = %hhx, C = %hhx,\nD = %hhx, E = %hhx, H = %hhx, L = %hhx,\nPC = %hx, SP = %hx\ninstruction = %s\n\n",
				argv[1], data[0], data[1], get_bin(data[1]), data[2], data[3], data[4], data[5], data[6], data[7], (unsigned short)((data[8] << 8) | data[9]), (unsigned short)((data[10] << 8) | data[11]),
				data[12] == 0xcb ? cb_opcodes[data[13]].inst : fmt_strcpy(opcodes[data[12]].inst, opcodes[data[12]].optype, data + 13)
			);
		}
	//	memcpy(backup, data, 16);
	}
	close(fd);
	close(fd2);
}
