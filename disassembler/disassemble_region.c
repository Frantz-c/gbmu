/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   disassemble_region.c                             .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/22 22:17:53 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/05/23 11:20:42 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disassemble_table.c"

static char		*itoazx(int n)
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
			s[i] += 40;
		n /= 16;
	}
	while (i > 0)
	{
		s[i--] = '0';
	}
	s[2] = 'x';
	if (negative)
	{
		s[0] = '-';
		return (s);
	}
	return (s + 1);
}

static void		*fmt_strcpy(char *dst, char *src,
							enum e_operand_type optype, void *bin)
{
	int		value;
	char	*numeric;

	while (*src != '*' && (*(dst++) = *(src++)));
	if (*src == 0)
		return (bin);

	if (optype == IMM8)
	{
		value = *((char*)bin);
		bin += 1;
	}
	else if (optype == IMM16)
	{
		value = *((unsigned char*)bin) + (*(unsigned char*)(bin + 1) << 8);
		value = (short)value;
		bin += 2;
	}
	else if (optype == ADDR8)
	{
		value = *((unsigned char*)bin);
		bin += 1;
	}
	else if (optype == ADDR16)
	{
		value = *((unsigned char*)bin) + (*(unsigned char*)(bin + 1) << 8);
		bin += 2;
	}
	numeric = itoazx(value); //special itoa 0x0001, ...
	strcpy(dst, numeric);
	dst += strlen(numeric);
	while ((*(dst++) = *(src++)));
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
								opcodes[*(unsigned char*)binary].optype, binary + 1);
			i += len;
		}
		else
		{
			free(buf);
			return (-1);
		}
	}
	*disassembled_code = buf;
	return (0);
}

int main(void)
{
	char	file[] = "\01\02\03\04\x77\xc4\x00\x2a\x0e\x2a\x1e\xff\xcb\x30\01\02\03\04";
	char	*dis;

	disassemble_region(&dis, file, 0x04, 0xe);

	puts(dis);
	free(dis);
	return (0);
}
