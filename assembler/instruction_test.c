# include <string.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <ctype.h>
# include "vector.h"
#include "includes/gbasm_struct.h"
#include "sources/gbasm_tools.c"

typedef struct	value_s
{
	uint16_t	is_signed;
	uint16_t	value;
}
value_t;

char	*get_type_str(param_t t)
{
	char	*type[] = {
		"UNKNOWN","NONE","A","B","C","D","E","F","H","L","AF","BC","DE","HL","SP","_NZ_","_Z_","_NC_","_C_","HLI","HLD","SP_ADDR","HL_ADDR","BC_ADDR","DE_ADDR","AF_ADDR","FF00_C","FF00_IMM8","IMM8","ADDR8","IMM16","ADDR16","SYMBOL"
	};

	return (type[t]);
}

param_t	get_type(char *param1, value_t *n)
{
	char		*s = param1;
	int			parent = 0;
	int			num = 0;
	int			error;
	uint32_t	len;
	uint32_t	tmp;


	// addr param
	if (*s == '(')
	{
		parent++;
		s++;
		
		// (C)
		if (LOWER(*s) == 'c' && s[1] == '\0')
			return (FF00_C);
		if (*s == '0' && LOWER(s[1]) == 'x')
		{
			num = 1;
			s += 2;
		}
		// (ff00+x)
		if (LOWER(*s) == 'f')
		{
			if (LOWER(s[1]) == 'f' && s[2] == '0' && s[3] == '0')
			{
				if ((!num && LOWER(s[4]) == 'h' && s[5] == '+') || (num && s[4] == '+'))
				{
					while (*s != '+') s++;
					s++;
					if (LOWER(*s) == 'c' && s[1] == '\0')
						return (FF00_C);
					return (FF00_IMM8);
				}
			}
		}
		if (num)
			return (UNKNOWN);
		// (rr)
		if (LOWER(*s) == 'a' && LOWER(s[1]) == 'f' && s[2] == '\0')
			return (AF_ADDR);
		if (LOWER(*s) == 'b' && LOWER(s[1]) == 'c' && s[2] == '\0')
			return (BC_ADDR);
		if (LOWER(*s) == 'd' && LOWER(s[1]) == 'e' && s[2] == '\0')
			return (DE_ADDR);
		if (LOWER(*s) == 'h' && LOWER(s[1]) == 'l' && s[2] == '\0')
			return (HL_ADDR);
		if (LOWER(*s) == 's' && LOWER(s[1]) == 'p' && s[2] == '\0')
			return (SP_ADDR);
		// (n) (nn)
		if (is_numeric(s, &len))
		{
			if (n->value > 0xff)
				return (ADDR16);
			return (ADDR8);
		}
		// (HL++?) (HLI) (HL--?) (HLD)
		if (LOWER(*s) == 'h' && LOWER(s[1]) == 'l')
		{
			if (s[2] == '+' && (s[3] == '\0' || (s[3] == '+' && s[4] == '\0')))
				return (HLI);
			if (LOWER(s[2]) == 'i' && s[3] == '\0')
				return (HLI);
			if (s[2] == '-' && (s[3] == '\0' || (s[3] == '-' && s[4] == '\0')))
				return (HLD);
			if (LOWER(s[2]) == 'd' && s[3] == '\0')
				return (HLD);
		}
	}
	else
	{
		if ((LOWER(*s) >= 'a' && LOWER(*s) <= 'f') && s[1] == '\0')
			return (A + (LOWER(*s) - 'a'));
		if (LOWER(*s) == 'h' && s[1] == '\0')
			return (H);
		if (LOWER(*s) == 'l' && s[1] == '\0')
			return (L);
		if (LOWER(*s) == 'a' && LOWER(s[1]) == 'f' && s[2] == '\0')
			return (AF);
		if (LOWER(*s) == 'b' && LOWER(s[1]) == 'c' && s[2] == '\0')
			return (BC);
		if (LOWER(*s) == 'd' && LOWER(s[1]) == 'e' && s[2] == '\0')
			return (DE);
		if (LOWER(*s) == 'h' && LOWER(s[1]) == 'l' && s[2] == '\0')
			return (HL);
		if (LOWER(*s) == 's' && LOWER(s[1]) == 'p' && s[2] == '\0')
			return (SP);
		if (n->is_signed && *s == '-' && is_numeric(s + 1, &len))
		{
			if ((int16_t)n->value > 0x7f || (int16_t)n->value < 0xff)
				return (IMM16);
			return (IMM8);
		}
		else if (is_numeric(s, &len))
		{
			if (n->value > 0xff)
				return (IMM16);
			return (IMM8);
		}
	}
	return (SYMBOL);
}

int		calcul_param(char *param, value_t *n, int is_ld)
{
	int			parent = 0;
	uint32_t	len;
	uint32_t	base = 0;
	uint32_t	result = 0;
	uint8_t		minus = 0;
	uint8_t		type, _signed = 0;
	char		*first_operator = NULL;

	if (*param == '(') {
		parent = 1;
		param++;
	}

// A + b * c + d
// ^
	if (*param == '-') {
		n->is_signed = 1;
		param++;
	}
	if ((type = is_numeric(param, NULL)))
	{
		base = atou_type(param, &len, type);
		param += len;
	}
	else if (n->is_signed == 1)
		goto __label_syntax;
	else if (!is_alpha(*param) && *param != '_')
		goto __label_syntax;
	while (!is_operator(*param))
	{
		if (*param == '\0')
			goto __set_n_return;
		param++;
	}
	if (n->is_signed)
		goto __signed_error;
	if (*param != '+' && *param != '-')
		goto __operator_error;
	if (*param == '-')
		minus = 1;
	first_operator = param;
	param++;

// a + B * c + d
//     ^
	if ((type = is_numeric(param, NULL)))
	{
		result = atou_type(param, &len, type);
		param += len;
	}
	else
	{
		printf("param = \"%s\"\n", param);
		if (to_lower_char[(uint8_t)*param] == 'c' && param[1] == '\0') {
			n->value = 0;
			return (0);
		}
		else
			goto __error;
	}
	while (!is_operator(*param))
	{
		if (*param == '\0')
			goto __set_n_return;
		param++;
	}
	if (*param != '*')
		goto __operator_error;
	param++;

// a + b * C + d
//         ^
	if ((type = is_numeric(param, NULL)))
	{
		result *= atou_type(param, &len, type);
		param += len;
	}
	else
		goto __error;
	while (!is_operator(*param))
	{
		if (*param == '\0')
			goto __set_n_return;
		param++;
	}
	if (*param != '+')
		goto __operator_error;
	param++;

// a + b * c + D
//             ^
	if ((type = is_numeric(param, NULL)))
	{
		result += atou_type(param, &len, type);
		param += len;
	}
	if (*param != '\0')
		goto __error;


__set_n_return:
	// ld (ff80h+x) exception
	if (parent && is_ld && base == 0xff00 && minus == 0)
	{
		base = 0;
		if (result > 0xff)
		{
			fprintf(stderr, "overflow\n");
			exit(1);
		}
		n->value = result;
		return (0);
	}
	if (n->is_signed)
	{
		n->value = (uint16_t)(0 - base);
		if ((0 - base) < (int16_t)0x8000)
			fprintf(stderr, "value is to small\n");
		return (0);
	}
	if (minus)
	{
		if (result > base)
		{
			fprintf(stderr, "underflow\n");
			exit(1);
		}
		result = base - result;
	}
	else
	{
		result = base + result;
		if (result > 0xffff)
		{
			fprintf(stderr, "overflow\n");
			exit(1);
		}
	}
	n->value = (result & 0xffff);
	if (first_operator) *first_operator = '\0';
	return (0);

__error:
	n->value = 0;
	fprintf(stderr, "ERROR CALCUL\n");
	return (-1);

__label_syntax:
	n->value = 0;
	fprintf(stderr, "label must begin with [a-zA-Z_]\n");
	return (-1);

__operator_error:
	n->value = 0;
	fprintf(stderr, "bad operator\n");
	return (-1);

__signed_error:
	n->value = 0;
	fprintf(stderr, "signed error\n");
	return (-1);
}


void	parse_instruction(char *s)
{
	char		*inst, *param[2] = {NULL};
	value_t		val[2] = {{0}};
	uint8_t		parent = 0;
	uint32_t	len, i;
	char		*tmp;
	int			is_ld = 0;
	param_t		t1 = NONE, t2 = NONE;

// PARAM 1
	inst = s;
	while (!is_space(*s))
	{
		if (is_endl(*s))
			goto __no_param;
		s++;
	}
	inst = strndup(inst, s - inst);

	if (strcmp(inst, "ld") == 0)
		is_ld = 1;


	while (is_space(*s)) s++;
	tmp = s;
	if (*s == '(')
		parent = ')';
	else if (*s == '[')
		parent = ']';
	
	for (len = 0; *s != ',' && !is_endl(*s); s++)
	{
		if (!is_space(*s)) len++;
	}
	param[0] = malloc(len + 1);
	for (s = tmp, i = 0; *s != ',' && !is_endl(*s) && *s != parent; s++)
	{
		if (!is_space(*s)) param[0][i++] = *s;
	}
	param[0][i] = '\0';
	if (parent) {
		if (*s != parent)
			goto __parent_error;
		param[0][0] = '(';
		s++;
		while (is_space(*s)) s++;
	}

	if (is_endl(*s))
		goto __add_inst;
	if (*s != ',')
		goto __unexpected_char;
	s++;
	parent = 0;

	while (is_space(*s)) s++;
	tmp = s;
	if (*s == '(')
		parent = ')';
	else if (*s == '[')
		parent = ']';
	
	for (len = 0; *s != ',' && !is_endl(*s); s++)
	{
		if (!is_space(*s)) len++;
	}
	param[1] = malloc(len + 1);
	for (s = tmp, i = 0; *s != ',' && !is_endl(*s) && *s != parent; s++)
	{
		if (!is_space(*s)) param[1][i++] = *s;
	}
	param[1][i] = '\0';
	if (parent) {
		if (*s != parent)
			goto __parent_error;
		param[1][0] = '(';
		s++;
	}
	
	if (*s == ',') goto __too_many_arguments;
	if (*s != '\0') goto __unexpected_char;



__add_inst:
	if (param[0])
	{
		if (calcul_param(param[0], val, is_ld) == -1)
			exit(1);
		t1 = get_type(param[0], val);
	}
	if (param[1])
	{
		if (calcul_param(param[1], val + 1, is_ld) == -1)
			exit(1);
		t2 = get_type(param[1], val + 1);
	}
	
__print_inst:
	printf("%s %s(%u)::%s  %s(%u)::%s\n", inst, param[0], val[0].value, get_type_str(t1), param[1], val[1].value, get_type_str(t2));
	return;

__no_param:
	inst = strndup(inst, s - inst);
	goto __print_inst;

__too_many_arguments:
	puts("TOO MANY ARGUMENTS\n");
	return;

__parent_error:
	puts("parent error\n");
	return;

__unexpected_char:
	puts("unexpected character\n");
}

int		main(int argc, char *argv[])
{
	if (argc != 2)
		return (1);

	parse_instruction(argv[1]);
}
