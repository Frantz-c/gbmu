#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

const uint32_t	ascii[256] = {
	0x40,0,0,0,0,0,0,0,0,2, //0
	0x40,0,0,0,0,0,0,0,0,0, //10
	0,0,0,0,0,0,0,0,0,0, //20
	0,0,2,0,0,0,0,4,4,0, //30
	0x80,0x80,4,4,0,4,0,4,1,1, //40
	1,1,1,1,1,1,1,1,0,0, //50
	4,0,4,0,0,0x18,0x18,0x18,0x18,0x18, //60
	0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18, //70
	0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18, //80
	0x18,0x80,0,0x80,4,0,0,0x28,0x28,0x28, //90
	0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28, //100
	0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28, //110
	0x28,0x28,0x28,0,4,0,0,0,0,0 //120
};

const uint8_t	to_lower_char[128] = {
	0,1,2,3,4,5,6,7,8,9,
	10,11,12,13,14,15,16,17,18,19,
	20,21,22,23,24,25,26,27,28,29,
	30,31,32,33,34,35,36,37,38,39,
	40,41,42,43,44,45,46,47,48,49,
	50,51,52,53,54,55,56,57,58,59,
	60,61,62,63,64,97,98,99,100,101,
	102,103,104,105,106,107,108,109,110,111,
	112,113,114,115,116,117,118,119,120,121,
	122,91,92,93,94,95,96,97,98,99,
	100,101,102,103,104,105,106,107,108,109,
	110,111,112,113,114,115,116,117,118,119,
	120,121,122,123,124,125,126,127
};

# define is_digit(c)		(ascii[(uint8_t)c] & 0x01)
# define is_alpha(c)		(ascii[(uint8_t)c] & 0x08)
# define is_lower_alpha(c)	(ascii[(uint8_t)c] & 0x20)
# define is_upper_alpha(c)	(ascii[(uint8_t)c] & 0x10)
# define is_space(c)		(ascii[(uint8_t)c] & 0x02)
# define is_alnum(c)		(ascii[(uint8_t)c] & 0x09)
# define is_endl(c)			(ascii[(uint8_t)c] & 0x40)
# define is_operator(c)		(ascii[(uint8_t)c] & 0x04)	// + - / % * ^ | & <(<) >(>)
# define is_parent(c)		(ascii[(uint8_t)c] & 0x80)
# define is_comment(c)		(c == ';' || c == '#')

# define LOWER(x)			to_lower_char[((x) & 0x7f)]


typedef struct	calc_elem_s
{
	int32_t		val;		// value or operator
	int32_t		lvl;
}
calc_elem_t;


uint32_t	atou_inc(const char **s)
{
	const char	*p;
	uint32_t	n;

	if (**s == '0' && LOWER((*s)[1]) == 'x')
	{
		p = *s + 2;
		if (is_digit(*p))
			n = *p - '0';
		else if (LOWER(*p) >= 'a' && LOWER(*p) <= 'f')
			n = LOWER(*p) - 87;
		else
			goto __ret_0;
		while (1)
		{
			p++;
			if (is_digit(*p))
				n = (n << 4) | (*p - '0');
			else if (LOWER(*p) >= 'a' && LOWER(*p) <= 'f')
				n = (n << 4) | (LOWER(*p) - 87) ;
			else
				break;
		}
	}
	else if (**s == '0' && LOWER((*s)[1]) == 'b')
	{
		p = *s + 2;
		if (*p == '1')
			n = 1;
		else if (*p == '0')
			n = 0;
		else
			goto __ret_0;
		while (1)
		{
			p++;
			if (*p == '0')
				n <<= 1;
			else if (*p == '1')
				n = (n << 1) | 1;
			else
				break;
		}
	}
	else if (**s == '0')
	{
		if (**s >= '0' && **s <= '7')
			n = **s - '0';
		else
			goto __ret_0;
		p = *s + 1;
		while (1)
		{
			if (*p >= '0' && *p <= '7')
				n = (n << 3) | (*p - '0');
			else
				break;
			p++;
		}
	}
	else if (is_digit(**s))
	{
		if (is_digit(**s))
			n = **s - '0';
		else
			goto __ret_0;
		p = *s + 1;
		while (1)
		{
			if (is_digit(*p))
				n = (n * 10) + (*p - '0');
			else
				break;
			p++;
		}
	}
	else
		goto __ret_0;

	*s = p;
	return (n);

__ret_0:
	return (0);
}

#define	PARENT_VALUE	2	//must be greater than 1 and do not contain BIT0

calc_elem_t	*get_calc(const char *s, uint32_t *n_elem)
{
	calc_elem_t	*calc = NULL;
	uint32_t	i = 0;
	int32_t		n;
	int32_t		lvl = 0;
	uint8_t		minus;
	
	*n_elem = 0;
	while (*s == ' ' || *s == '\t') s++;

	if (*s == 0)
		return (NULL);

	while (1)
	{
// is opened parent ?
		while (*s == '(') {
			s++;
			while (*s == ' ' || *s == '\t') s++;
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
		if (!is_digit(*s))
			goto __error_not_digit;

		n = atou_inc(&s);
/*		tmp = *(s++) - 48;
		while (isdigit(*s)) {
			tmp *= 10;
			tmp += *(s++) - 48;
		}*/
		if (minus)
			n = (int32_t)(~n + 1);

// add number
		if ((i & 0x7) == 0)
			calc = realloc(calc, sizeof(calc_elem_t) * (i + 8));
		calc[i].val = n;
		calc[i++].lvl = lvl;
		
		while (*s == ' ' || *s == '\t') s++;

// is closed parent ?
		int32_t	prev_lvl = lvl;
		while (*s == ')') {
			s++;
			while (*s == ' ' || *s == '\t') s++;
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
		while (*s == ' ' || *s == '\t') s++;
		if (*s == 0)
			break;
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
				//printf("(1)calculate(%d %c %d);\n", calc[start].val, calc[start+1].val, calc[start+2].val);
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
				//printf("(2)calculate(%d %c %d);\n", calc[start-1].val, calc[start].val, calc[start+1].val);
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


const char	*binary(int32_t n)
{
	uint32_t		space = 0x01010100u;
	uint32_t		b = 0x80000000u;
	uint8_t			is_neg = (n < 0);
	static char		string[48] = {0};
	uint8_t			i = 0;
	
	if (is_neg)
		n = (~n + 1);

	if ((b & n) == 0)
	{
		b >>= 1;
		while (b && (b & n) == 0)
			b >>= 1;
	}
	if (is_neg)
	{
		uint32_t	c = b;

		if (b < 0x80)
			c = 0x80u;
		else if (b < 0x8000)
			c = 0x8000u;
		else if (b < 0x800000)
			c = 0x800000u;
		else
			c = 0x80000000u;

		while (c != b)
		{
			string[i++] = '1';
			if (space & c)
				string[i++] = ' ';
			c >>= 1;
		}
	}

	if (b == 0)
	{
		string[i++] = '0';
	}
	else
	{
		while (b)
		{
			string[i++] = (b & n) ? '1' : '0';
			if (space & b)
				string[i++] = ' ';
			b >>= 1;
		}
	}

	return ((const char *)string);
}

int		main(int argc, char *argv[])
{
	uint32_t	n_elem;
	if (argc != 2)
	{
		fprintf(stderr, "%s [operations]\n\n"
						"  operators:\n"
						"    + - / * %% >> << | & ^\n"
						"  others:\n"
						"    ( ) ' ' '\\t'\n\n", argv[0]);
		return (1);
	}

	calc_elem_t	*calc = get_calc(argv[1], &n_elem);

	int	result = execute_calcul(calc, n_elem);
	printf(
			"\e[1;33mhexa\e[0m=[\e[1m0x%X\e[0m], "
			"\e[1;33mdecimal\e[0m=[\e[1m%d\e[0m], "
			"\e[1;33moctal\e[0m=[\e[1m0%o\e[0m], "
			"\e[1;33mbinary\e[0m=[\e[1m%s\e[0m]\n",
			result, result, result, binary(result)
	);

	return (0);
}
