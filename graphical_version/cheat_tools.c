#include <stdint.h>
#include <assert.h>
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
#include <stdarg.h>
#include <time.h>
#include "memory_map.h"
#include "pkmn_green_string.h"
#include "cheat.h"

extern int		is_missingno(uint32_t n)
{
	static const uint8_t	missingno[191] = {
		1,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		1,0,1,0,0,0,1,0,0,0,
		0,1,1,1,0,0,0,1,1,1,
		0,0,0,0,0,0,0,0,0,1,
		1,1,0,0,0,0,1,1,0,0,
		0,0,0,0,1,1,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,1,0,0,0,0,
		0,1,1,0,0,0,0,1,0,0,
		0,0,0,0,1,1,0,1,0,0,
		1,0,0,0,0,0,1,0,0,0,
		0,0,0,0,0,0,1,0,0,1,
		1,1,1,0,0,0,0,0,0,0,
		0,0,1,0,1,1,0,0,0,0,
		0,1,1,1,1,0,0,0,0,0,0
	};
	if (n > 190U)
		return (1);
	return (missingno[n]);
}

extern int	set_string_green(char *input, uint8_t maxlen, uint8_t *output)
{
	static const unsigned char	boin[58] = {
		1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
		0,0,0,0,0,0,
		1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0
	};

	for (uint8_t count = 0; count != maxlen && *input; input++, count++)
	{
		if (*input >= '0')
		{
			if (*input >= 'a' && *input <= 'z')
			{
				if (*input == 't' && (input[1] == 's' || input[1] == 'S')
					&& (input[2] == 'u' || input[2] == 'U'))
				{
					input += 2;
					*(output++) = 0xc2; // TSU
				}
				else if (*input == 'n' && (input[1] == '\'' || input[1] == '\0'
					|| (!boin[input[1] - 'a'] && input[1] != 'y' && input[1] != 'Y')))
				{
					if (input[1] == '\'' || input[1] == 'y' || input[1] == 'Y')
						input++;
					*(output++) = 0xde; // N
				}
				else
				{
					if (get_hiragana(&input, &output, &count) == -1)
						goto __error;
				}
			}
			else if (*input >= 'A' && *input <= 'Z')
			{
				if (*input == 'T' && (input[1] == 's' || input[1] == 'S')
					&& (input[2] == 'u' || input[2] == 'U'))
				{
					input += 2;
					*(output++) = 0x91; // TSU
				}
				else if (*input == 'N' && (input[1] == '\'' || input[1] == '\0'
					|| (!boin[input[1] - 'A'] && input[1] != 'y' && input[1] != 'Y')))
				{
					if (input[1] == '\'' || input[1] == 'y' || input[1] == 'Y')
						input++;
					*(output++) = 0xab; // N
				}
				else
				{
					if (get_katakana(&input, &output, &count) == -1)
					{
						puts("ERROR\n");
						goto __error;
					}
				}
			}
			if (*input <= '9')
				*(output++) = (*input - '0') + 0xf6;
			else
				goto __error;
		}
		else if (*input == '-') {
			*(output++) = 0xe3; // -
		}
		else if (*input == ' ') {
			*(output++) = 0x0; // ' '
		}
		else
			goto __error;
	}
	*(output) = 0x50;

	return (0);

	__error:
	puts("error");
	return (-1);
}

extern int	set_string_red(char *input, uint8_t maxlen, int fill, uint8_t *output)
{
	uint8_t	count;

	for (count = 0; *input && count != maxlen; input++, count++)
	{
		if (*input >= 'a' && *input <= 'z')
			*(output++) = (uint8_t)((*input - 'a') + 0xa0);
		else if (*input >= 'A' && *input <= 'Z')
			*(output++) = (uint8_t)((*input - 'A') + 0x80);
		else if (*input >= '0' && *input <= '9')
			*(output++) = (uint8_t)((*input - '0') + 0xf6);
		else
		{
			switch (*input)
			{
				case ' ': *(output++) = 229U; break;
				case '(': *(output++) = 154U; break;
				case ')': *(output++) = 155U; break;
				case ':': *(output++) = 156U; break;
				case ';': *(output++) = 157U; break;
				case '[': *(output++) = 158U; break;
				case ']': *(output++) = 159U; break;
				case '?': *(output++) = 230U; break;
				case '!': *(output++) = 231U; break;
				case '.': *(output++) = 232U; break;
				default: goto __error;
			}
		}
	}
	if (fill)
	{
		while (count <= maxlen)
		{
			*(output++) = 0x50;
			count++;
		}
	}
	else
		*output = 0x50;
	return (0);

	__error:
	puts("error");
	return (-1);
}
