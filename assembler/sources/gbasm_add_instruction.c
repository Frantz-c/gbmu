/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_add_instruction.c                          .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/16 22:10:25 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/25 10:16:35 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "gbasm_struct.h"
#include "gbasm_struct_tools.h"
#include "gbasm_tools.h"
#include "gbasm_error.h"
#include "gbasm_get_bin_instruction.h"

int		set_params(char **param1, char **param2, char **s)
{
//	char		*param[2] = {NULL};
//	value_t		val[2] = {{0}};
	uint8_t		parent = 0;
	uint32_t	len, i;
	char		*tmp;
	*param1 = NULL;
	*param2 = NULL;
//	int			is_ld = 0;
//	param_t		t1 = NONE, t2 = NONE;

/*
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
*/

	while (is_space(**s)) (*s)++;
	if (is_endl(**s))
		return (0);

	tmp = *s;
	if (**s == '(')
		parent = ')';
	else if (**s == '[')
		parent = ']';
	
	for (len = 0; **s != ',' && !is_endl(**s); (*s)++)
	{
		if (!is_space(**s)) len++;
	}
	*param1 = malloc(len + 1);
	for (*s = tmp, i = 0; **s != ',' && !is_endl(**s) && **s != ')' && **s != ']'; (*s)++)
	{
		if (!is_space(**s)) (*param1)[i++] = **s;
	}
	(*param1)[i] = '\0';
	if (parent) {
		if (**s != parent)
			goto __parent_error;
		(*param1)[0] = '(';
		(*s)++;
		while (is_space(**s)) (*s)++;
	}

	if (is_endl(**s))
		return (0);
	if (**s != ',')
		goto __unexpected_char;
	(*s)++;
	parent = 0;

	while (is_space(**s)) (*s)++;
	tmp = *s;
	if (**s == '(')
		parent = ')';
	else if (**s == '[')
		parent = ']';
	
	for (len = 0; **s != ',' && !is_endl(**s); (*s)++)
	{
		if (!is_space(**s)) len++;
	}
	(*param2) = malloc(len + 1);
	for (*s = tmp, i = 0; **s != ',' && !is_endl(**s) && **s != ')' && **s != ']'; (*s)++)
	{
		if (!is_space(**s)) (*param2)[i++] = **s;
	}
	(*param2)[i] = '\0';
	if (parent) {
		if (**s != parent)
			goto __parent_error;
		(*param2)[0] = '(';
		(*s)++;
	}
	printf("**s = %d\n", **s);
	if (**s == ',') goto __too_many_arguments;
	if (!is_endl(**s)) goto __unexpected_char;
	return (0);


__too_many_arguments:
	puts("TOO MANY ARGUMENTS\n");
	return (-1);

__parent_error:
	puts("parent error\n");
	return (-1);

__unexpected_char:
	puts("unexpected character\n");
	return (-1);
}

int		replace_macro(char **param, vector_t *macro)
{
	char		*s = *param;
	char		*start, *end;
	char		*new = NULL;
	uint32_t	newl = 0;
	char		*macro_name;
	int32_t		index;
	uint8_t		parent = 0;
	uint8_t		count = 0;

	if (*s == '[' || *s == '(') {
		parent = 1;
		s++;
		while (is_space(*s)) s++;
	}

	// maybe a macro
	while (!is_endl(*s) && count != 4)
	{
		if (is_alpha(*s) || *s == '_')
		{
			start = s;
			s++;
			while (is_alnum(*s) || *s == '_') s++;
			if (!is_space(*s) && !is_endl(*s) && !is_operator(*s) && *s != ']' && *s != ')')
			{
				goto __unexpected_char;
			}
			if (is_operator(*s))
			{
				if ((count == 0 && *s == '*') || (count == 1 && *s != '*') || (count == 2 && *s != '+') || count == 3)
					goto __unexpected_operator;
			}
			macro_name = strndup(start, s - start);
			printf("macro_name = \"%s\"\n", macro_name);
			index = vector_search(macro, (void*)&macro_name);
			if (index > -1)
			{
				char		*macro_content = VEC_ELEM(macro_t, macro, index)->content;
				uint32_t	macro_argc = VEC_ELEM(macro_t, macro, index)->argc;
				uint32_t	len;

				if (macro_argc != 0)
					goto __macro_with_param;

				if (is_numeric(macro_content, &len))
				{
					char	*value = macro_content;
					macro_content += len;
					while (is_space(macro_content[len])) len++;
					if (macro_content[len] != '\0')
						goto __invalid_macro;
					if (new == NULL)
					{
						newl = (start - *param) + (strlen(value));
						new = malloc(newl + 1);
						strncpy(new, *param, start - *param);
						strcpy(new + (start - *param), value);
					}
					else
					{
						newl += (start - end) + (strlen(value));
						new = realloc(new, newl + 1);
						strncat(new, end, start - end);
						strcat(new, value);
					}
				}
				end = s;
			}
			while (is_space(*s)) s++;
			if (is_operator(*s))
			{
				if ((count == 0 && *s == '*') || (count == 1 && *s != '*') || (count == 2 && *s != '+') || count == 3)
					goto __unexpected_operator;
				while (is_space(*s)) s++;
			}
			if (*s == ']' || *s == ')') {
				if (parent == 0)
					goto __unexpected_char;
				parent--;
				s++;
			}
		}
		count++;
	}

	if (new)
	{
		strcat(new, end);
		free(*param);
		*param = new;
	}
	return (0);

__unexpected_operator:
__macro_with_param:
__unexpected_char:
	return (-1);
__invalid_macro:
	free(macro_name);
	return (-1);
}

param_t	get_type(char *param, value_t *n)
{
	char		*s = param;
	int			parent = 0;
	int			num = 0;
//	int			error;
	uint32_t	len;
//	uint32_t	tmp;


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
			if ((int32_t)n->value > 0x7f || (int32_t)n->value < -0x80)
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
	if (is_alpha(*param) || *param == '_')
	{
		while (is_alnum(*param) || *param == '_')
			param++;
	}
	if (*param != '\0')
		return (UNKNOWN);
	return (SYMBOL);
}


int		calcul_param(char *param, value_t *n, int is_ld)
{
	int			parent = 0;
	uint32_t	len;
	uint32_t	base = 0;
	uint32_t	result = 0;
	uint8_t		minus = 0;
	uint8_t		type;
//	uint8_t		_signed = 0;
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
	if (n->is_signed == 0 && LOWER(param[-1]) == 'l' && LOWER(param[-2]) == 'h'
			&& (param[1] == '\0' || (param[1] == *param && param[2] == '\0')))
	{
		n->value = 0;
		return (0);
	}
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
		n->value = (uint32_t)(0 - base);
		if ((0 - base) < (int32_t)0x8000)
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


char	*add_instruction(char *inst, vector_t *area, vector_t *ext_symbol, loc_sym_t *loc_symbol, vector_t *macro, char *s, data_t *data)
{
	char		*param1, *param2;
	param_t		param[2] = {NONE, NONE};
	value_t		val = {0, 0, 0};
	int			is_ld = 0;

	str_to_lower(inst);
	if (*inst == 'l' && inst[1] == 'd' && inst[2] == '\0')
		is_ld = 1;
	if (set_params(&param1, &param2, &s) == -1)
	{
		if (*s == ',')
			goto __too_many_param;
		goto __unexpected_char;
	}
	printf("inst = \"%s\", param1 = \"%s\", param2 = \"%s\"\n", inst, param1, param2);

	if (param1)
	{
		if (macro && replace_macro(&param1, macro) == -1) //without params only
			goto __error;
		if (calcul_param(param1, &val, is_ld) == -1)
			goto __error;
		printf("replace_param1 = \"%s\"\n", param1);
		param[0] = get_type(param1, &val); //default SYMBOL

		if (param2)
		{
			value_t	tmp_val = {0, 0, 0};

			if (macro && replace_macro(&param2, macro) == -1)
				goto __error;
			if (calcul_param(param2, &tmp_val, is_ld) == -1)
				goto __error;
			printf("replace_param2 = \"%s\"\n", param2);
			param[1] = get_type(param2, &tmp_val); //default SYMBOL
			if (param[0] < FF00_IMM8 && param[1] >= FF00_IMM8)
				val = tmp_val;
		}
	}

	char	*symbol = param2;

	if (param[0] == SYMBOL)
	{
		symbol = param1;
		free(param2);
	}
	else
		free(param1);


	param_error_t	error;
	uint8_t		bin[4] = {0, 0, 0, val.sign};

	error = get_bin_instruction(inst, param, &val, bin);
	push_instruction(VEC_ELEM(code_area_t, area, data->cur_area), bin, param, symbol, ext_symbol, loc_symbol, data);
	return (s);

__unexpected_char:
__too_many_param:
__error:
	while (!is_endl(*s))
	s++;
	return (s);
}
