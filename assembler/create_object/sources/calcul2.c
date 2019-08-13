#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

typedef struct	calc_elem_s
{
	int32_t		val;		// value or operator
	int32_t		lvl;
}
calc_elem_t;

/*
	couper encore une fois en tableau 2D (le niveau depend uniquement du signe la 2e fois)

	exemple:

		5*(3+6*2+3*5)*(5+3*(5+5)) = 5 * 30 * 35 = 

		==>	1) faire les calculs possible dans chaque ligne  (on ote les parentheses)

		5 *
		3 +
		6 * 2
		+
		3 * 5
		*
		5 +
		3 *
		5 + 5

		==> 2) faire les calculs de niveau prioritaire (signe precedent < signe courant && signe suivant < signe courant)

		// tableau 2D ?
		5 *(1)
		3 +(4) 12 +(4)
		15 *(0)
		5 +(4)
		3 *(5)
		10 0(0)

		==>

		5 *(1)
		30 *(0)
		5 +(4)
		30

		==>

		150 *(0)
		35

		==>

		...

		A priori ce qui suit est nul...
		Alternative: couper directement dans la chaine [ avec '(' = '\0' && ')' = '\0' ]

		"5*"
		"3+6*2+3*5"
		"*"
		"5+3*"
		"5+5"

		==> reprises des etapes normales (voir au dessus)

		// tableau 2D ?
		5 *(1)
		3 +(4) 12 +(4)
		15 *(0)
		5 +(4)
		...

*/

#define	PARENT_VALUE	2	//must be greater than 1 and do not contain BIT0

calc_elem_t	*get_calc(const char *s, uint32_t *n_elem)
{
	calc_elem_t	*calc = NULL;
	uint32_t	i = 0;
	int32_t		n;
	int32_t		lvl = 0;
	uint8_t		minus;
	
	*n_elem = 0;

	if (*s == 0)
		return (NULL);

	while (1)
	{
// is opened parent ?
		while (*s == '(') {
			s++;
			lvl &= ~0x1u;	// cplt = 0;
			lvl += PARENT_VALUE;
		}


// get number
		if (*s == '-')
		{
			minus = 1;
			s++;
		}
		else
			minus = 0;
		if (!isdigit(*s))
			goto __error_not_digit;

		register uint32_t	tmp = 0;
		tmp = *(s++) - 48;
		while (isdigit(*s)) {
			tmp *= 10;
			tmp += *(s++) - 48;
		}
		if (minus)
			n = (int32_t)(~tmp + 1);
		else
			n = (int32_t)tmp;

// add number
		if ((i & 0x7) == 0)
			calc = realloc(calc, sizeof(calc_elem_t) * (i + 8));
		calc[i].val = n;
		calc[i++].lvl = lvl;
		

// is closed parent ?
		int32_t	prev_lvl = lvl;
		while (*s == ')') {
			s++;
			lvl &= ~0x1u; // cplt = 0
			prev_lvl = lvl;
			lvl -= PARENT_VALUE;
			if (lvl < 0)
				goto __parent_error;
		}


// get operator
		switch (*s)
		{
			case '*': case '/': case '%': case '|': case '&': case '^':
			{
				if ((lvl & 0x1u) == 0 && lvl >= prev_lvl)
				{
					calc[i-1].lvl++;
					lvl++;
				}
				break;
			}
			case '>': case '<':
			{
				if (s[1] != s[0])
					exit(1);
				if ((lvl & 0x1u) == 0)
				{
					calc[i-1].lvl++;
					lvl++;
				}
				s++;
				break;
			}
			case '+':
			case '-':
				lvl &= ~0x1u;
			case '\0':
				break;
			default:
				fprintf(stderr, "ope error (%c::%d) \"%s\"\n", *s, *s, s); exit(1);
		}

// add operator
		calc[i].val = *s;
		calc[i++].lvl = lvl;

		if (*s == 0)
			break;
		s++;
	}
	
	if ((lvl &= ~0x1u) != 0)
		goto __parent_error;
	*n_elem = i;
	calc[i-1].lvl = 0;
	return (calc);

	__error_not_digit:
		fprintf(stderr, "%c is not a number\n", *s);
		exit(1);
	__parent_error:
		fprintf(stderr, "parent error\n");
		exit(1);
}

int32_t	calculate(calc_elem_t calc[])
{
	switch (calc[1].val)
	{
		case '*': return (calc[0].val * calc[2].val);
		case '/': return (calc[0].val / calc[2].val);	// check if r_value == 0 (error)
		case '%': return (calc[0].val % calc[2].val);	// check if r_value == 0 (error)
		case '|': return (calc[0].val | calc[2].val);
		case '&': return (calc[0].val & calc[2].val);
		case '^': return (calc[0].val ^ calc[2].val);
		case '>': return (calc[0].val >> calc[2].val);	// check if r_value < 16 && > -1
		case '<': return (calc[0].val << calc[2].val);	// check if r_value < 16 && > -1
		case '+': return (calc[0].val + calc[2].val);
		case '-': return (calc[0].val - calc[2].val);
		case '\0': return (calc[0].val);
		default : fprintf(stderr, "OPE ERROR"); exit(1);
	}
	return (0);
}

int32_t	execute_calcul(calc_elem_t *calc, uint32_t n_elem)
{
	uint32_t	start, end;

	for (start = 0; start < n_elem; )
	{
		for (end = start + 1; end < n_elem && calc[start].lvl == calc[end].lvl; end++);

		if (end - start >= 3)
		{
			register uint32_t	len = end - start;

			while (len >= 3)
			{
				printf("(1)calculate(%d %c %d);\n", calc[start].val, calc[start+1].val, calc[start+2].val);
				calc[start].val = calculate(calc + start);
				end -= 2;
				len = end - start;
				memmove(calc + start + 1, calc + start + 3, (n_elem - (start + 2)) * sizeof(calc_elem_t));
				n_elem -= 2;
			}
		}
		else
			start += 2;
	}

	while (n_elem > 2)
	{
		for (start = 1; start < n_elem - 1; )
		{
			if ((start < 2 || calc[start-2].lvl < calc[start].lvl) && calc[start+2].lvl <= calc[start].lvl)
			{
				printf("(2)calculate(%d %c %d);\n", calc[start-1].val, calc[start].val, calc[start+1].val);
				calc[start-1].val = calculate(calc + start - 1);
				memmove(calc + start, calc + start + 2, (n_elem - (start + 1)) * sizeof(calc_elem_t));
				n_elem -= 2;
			}
			else
				start += 2;
		}
	}
	return (calc[0].val);
}


int		main(int argc, char *argv[])
{
	uint32_t	n_elem;
	if (argc != 2)
	{
		fprintf(stderr, "error\n");
		return (1);
	}

	calc_elem_t	*calc = get_calc(argv[1], &n_elem);

	printf("result = %d\n", execute_calcul(calc, n_elem));

	return (0);
}

/*
	printf("n_elem = %u\n", n_elem);
	for (uint32_t j = 0; ; j += 2)
	{
		printf("%d\e[0;31m(%d)\e[0m %c\e[0;31m(%d)\e[0m ", calc[j].val, calc[j].lvl, calc[j+1].val, calc[j+1].lvl);
		if (calc[j+1].val == 0 && calc[j+1].lvl == 0)
			break;
	}
	puts("\n");
*/

