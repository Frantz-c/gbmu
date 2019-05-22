/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   disassemble_region.c                             .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/22 22:17:53 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/05/22 22:58:09 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "disassemble_table.c"

static char		*itoazx(int n)
{
	char	s[7];
	int		i = 5;

	s[6] = '\0';
	while (n)
	{
		s[i] = ((n % 16) + '0');
		if (s[i] > '9')
			s[i] += 40;
		n /= 16;
	}
	while (i)
	{
		s[i--] = '0';
	}
	s[1] = 'x';
	return (s);
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
		value = *((char*)bin) + (*(char*)(bin + 1) << 8);
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
		if (*binary == 0xcb)
		{
			len = strlen(cb_opcodes[binary[1]].inst);
			if (i + len >= buflen)
			{
				buflen += 1024;
				buf = realloc(buf, buflen);
				if (!buf) {
					fprintf(stderr, "realloc fatal error\n");
					exit(1)
				}
			}
			strcpy(buf + i, cb_opcodes[binary[1]].inst);
			i += len;
			binary += 2;
		}
		else if (opcodes[*binary].inst)
		{
			len = strlen(opcodes[*binary].inst);
			if (opcodes[*binary].optype)
			{
				switch (opcodes[*binary].optype)
				{
					case IMM8:
					case ADDR8: len += 3; break;
					case IMM16:
					case ADDR16: len += 5;
				}
			}
			if (i + len >= buflen)
			{
				buflen += 1024;
				buf = realloc(buf, buflen);
				if (!buf) {
					fprintf(stderr, "realloc fatal error\n");
					exit(1)
				}
			}
			binary = fmt_strcpy(buf + i, opcodes[*binary].inst,
								opcodes[*binary].optype, binary + 1);
			i += len;
		}
		else
		{
			free(buf);
			return (-1);
		}
	}
	disassembled_code = buf;
	return (0);
}
