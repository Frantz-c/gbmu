/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   sort_instructions_name.c                         .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/10 17:13:13 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/10 17:33:32 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int		cmpfunc(const void *a, const void *b)
{
	return (strcmp(*((char**)a), *((char**)b)));
}

size_t	tablength(char *t[])
{
	size_t ret = 0;

	while (t[ret]) ret++;

	return (ret);
}

int main(int argc, char *argv[])
{
	unsigned int	i = 0;
	char	*instructions[] = {
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
		"daa",
		"cpl",
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
		"reset",
		"sbb",
		"not",
		"shl",
		"shr",
		"sar",
		NULL
	};

	printf("tablength = %zu\n", tablength(instructions));
	qsort(instructions, tablength(instructions), sizeof(char*), &cmpfunc);
	printf("tablength = %zu\n", tablength(instructions));

	if (argc == 2)
		printf("char *%s[] = {\n", argv[1]);
	else
		printf("char *inst[] = {\n");
	while (instructions[i])
	{
		printf("\t\"%s\",\n", instructions[i]);
		i++;
	}
	printf("\tNULL\n};\n");

	return (0);
}
