#include "std_includes.h"
#include "gbasm_struct.h"
#include "time.h"

uint8_t char_value[58];
/*
static uint8_t	char_value[58] = {
	  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,
	 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
	255,255,255,255,255,255,
	  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,
     13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25
};
*/
static char	*instructions[] = {
	"ld",
	"ldd",
	"ldi",
	"ldhl",
	"push",
	"pop",
	"add",
	"adc",
	"sub",
	"sbc",
	"and",
	"or",
	"xor",
	"cp",
	"inc",
	"dec",
	"swap",
	"daa", //??? decimal adjust A
	"cpl", //??? complement A register
	"ccf",
	"scf",
	"nop",
	"halt",
	"stop",
	"di",
	"ei",
	"rlca",
	"rla",
	"rrca",
	"rra",
	"rlc",
	"rl",
	"rrc",
	"rr",
	"sla",
	"sra",
	"srl",
	"bit",
	"set",
	"res",
	"jp",
	"jr",
	"call",
	"rst",
	"ret",
	"reti",
	// customs
	"jpnz",
	"jpz",
	"jpnc",
	"jpc",
	"jrnz",
	"jrz",
	"jrnc",
	"jrc",
	"cmp",
	"mov",
	"callnz",
	"callz",
	"callnc",
	"callc",
	"retnz",
	"retz",
	"retc",
	"retnc",
	"testb",
	NULL
};

int				hash(const char *s)
{
	int		hash = 0;
	int		mask = 0x44;
	int		n = 4;

	while (*s)
	{
		hash += char_value[*s - 'A'];
		hash *= n;
		hash ^= mask;
		s++;
		if (n > 1)
			n--;
		mask <<= 1;
	}
	/*
		if (*s)
		{
			hash += char_value[*s - 'A'];
			hash /= 2;
		}
		else
			break;
		s++;
	}*/
	hash = hash % 133;
	return (hash);
}
int			print_collisions(int tab[], int len)
{
	int i = 0;
	int	col = 0;

	while (i < len - 1)
	{
		int j = i + 1;
		while (j < len)
		{
			if (tab[i] == tab[j])
			{
				col++;
//				printf("col ==> %s, %s\n", instructions[i], instructions[j]);
			}
			j++;
		}
		i++;
	}
	return (col);
}

void	set_char_value(void)
{
	int i = 0;


	char_value[i++] = (rand() % 51) / 2;
	while (i < 26)
	{
	__rand:
		char_value[i] = (rand() % 51) / 2;

		int j = 0;
		while (j < i)
		{
			if (char_value[i] == char_value[j])
				goto __rand;
			j++;
		}
		i++;
	}
	memcpy(char_value + 31, char_value, 26);
}

void		print_tab(void)
{
	int i = 0;
	while (i < 26)
	{
		printf("[%c] %d, ", i + 'A', char_value[i]);
		i++;
	}
	printf("\n");
}

extern char		*add_instruction_or_label(zones_t **zon, zones_t **curzon,
								defines_t *def[], char *s, error_t *err)
{
	int		hash_tab[256];
	int		i = 0, j = 0;
	int		col = 11;

	srand(time(NULL));

	while (col != 0)
	{
		i = 0;
		j = 0;
		set_char_value();
		while (instructions[j])
		{
			hash_tab[i] = hash(instructions[j]);
	//		printf("%s -> %d\n", instructions[j], hash_tab[i]);
			i++;
			j++;
		}

		col = print_collisions(hash_tab, i);
		if (col < 3)
		{
			printf("\n\e[0;36m%d collisions\e[0m\n", col);
			print_tab();
		}
	}
	//print_tab();
	while (*s && *s != '\n') s++;
	return (s);
}
