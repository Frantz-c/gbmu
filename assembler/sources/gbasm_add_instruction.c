/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_add_instruction.c                          .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/16 22:10:25 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/19 16:49:34 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "gbasm_struct.h"
#include "gbasm_tools.h"
#include "gbasm_error.h"

/*
	etapes:
	
	1)	"ld (  FF80 + 12  ), A" -> is_macro("ld") ?
	2)	to_lower() -> "ld (  ff80 + 12  ), a"
	3)	param1 = "(ff80+12)", param2 = "a"
	4)	is_macro_without_param("(ff80+12)") ? is_macro_with_param("a") ?
	5)	calcul_param() -> param1 = "(ff80", val1 = 12; param2 = "a", val2 = 0

*/


typedef struct	instruction_s
{
	const char			*name;
	const void *const	addr;
}
instruction_t;

ssize_t		instruction_search(instruction_t inst[71], char *tofind)
{
	uint32_t	left, right, middle;
	int			side;

	left = 0;
	right = 71;
	while (left < right)
	{
		middle = (left + right) / 2;
		side = strcmp(inst[middle].name, tofind);
		if (side < 0)
			left = middle + 1;
		else if (side > 0)
			right = middle - 1;
		else
			return (middle);
	}
	return (0xffffffffu);
}

int		set_params(char **param1, char **param2, char **s)
{
//	char		*param[2] = {NULL};
	value_t		val[2] = {{0}};
	uint8_t		parent = 0;
	uint32_t	len, i;
	char		*tmp;
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
	{
		param1 = NULL;
		param2 = NULL;
		return (0);
	}

	tmp = *s;
	if (**s == '(')
		parent = ')';
	else if (**s == '[')
		parent = ']';
	
	for (len = 0; **s != ',' && !is_endl(**s); (*s)++)
	{
		if (!is_space(**s)) len++;
	}
	param1 = malloc(len + 1);
	for (s = tmp, i = 0; **s != ',' && !is_endl(**s) && **s != parent; (*s)++)
	{
		if (!is_space(**s)) param1[i++] = **s;
	}
	param1[i] = '\0';
	if (parent) {
		if (**s != parent)
			goto __parent_error;
		param1[0] = '(';
		(*s)++;
		while (is_space(**s)) (*s)++;
	}

	if (is_endl(**s))
		goto __add_inst;
	if (**s != ',')
		goto __unexpected_char;
	s++;
	parent = 0;

	while (is_space(**s)) s++;
	tmp = s;
	if (**s == '(')
		parent = ')';
	else if (**s == '[')
		parent = ']';
	
	for (len = 0; **s != ',' && !is_endl(**s); (*s)++)
	{
		if (!is_space(**s)) len++;
	}
	param2 = malloc(len + 1);
	for (*s = tmp, i = 0; **s != ',' && !is_endl(**s) && **s != parent; (*s)++)
	{
		if (!is_space(**s)) param2[i++] = **s;
	}
	param2[i] = '\0';
	if (parent) {
		if (**s != parent)
			goto __parent_error;
		param2[0] = '(';
		s++;
	}
	
	if (**s == ',') goto __too_many_arguments;
	if (**s != '\0') goto __unexpected_char;


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


/*
int		set_params(char **p1, char **p2, char **s)
{
	uint8_t	parent = 0;

	while (is_space(**s)) (*s)++;
	if (is_endl(**s))
	{
		*p1 = NULL;
		return (0);
	}

	*p1 = *s;

	//param1
	if (**s == '(') {
		parent += **s;
		(*s)++;
		while (is_space(**s)) (*s)++;
	}
	if (**s == '[') {
		parent += **s;
		(*s)++;
		while (is_space(**s)) (*s)++;
	}
	while (is_alnum(**s) || **s == '_' || is_operator(**s)) (*s)++;
	*p1 = strndup(*p1, *s - *p1);
	if (parent) **p1 = '(';
	if (**s == ')' || **s == ']') {
		parent -= **s;
		(*s)++;
	}
	if (parent)
		return (-1);

	while (is_space(**s)) (*s)++;
	if (is_endl(**s))
	{
		*p2 = NULL;
		return (0);
	}
	if (**s != ',')
		return (-1);
	while (is_space(**s)) (*s)++;

	//param2
	parent = 0;
	*p2 = *s;
	if (**s == '(' || **s == '[') {
		parent += **s;
		(*s)++;
		while (is_space(**s)) (*s)++;
	}
	while (is_alnum(**s) || **s == '_' || is_operator(**s)) (*s)++;
	*p2 = strndup(*p2, *s - *p2);
	while (is_space(**s)) (*s)++;
	if (**s == ')' || **s == ']') {
		parent -= **s;
		(*s)++;
	}
	if (parent)
		return (-1);

	while (is_space(**s)) (*s)++;
	if (is_endl(**s))
		return (0);
	return (-1);
}
*/
//"truc + bidule * machin"

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
			index = vector_search(macro, macro_name);
			if (index > -1)
			{
				char		*macro_content = VEC_ELEM(macro_t, macro, index)->content;
				uint32_t	macro_argc = VEC_ELEM(macro_t, macro, index)->argc;
				uint32_t	len;

				if (macro_argc != 0)
					goto __macro_with_param;

				if (is_numeric(macro_content, &len))
				{
					macro_content += len;
					while (is_space(macro_content[len])) len++;
					if (macro_content[len] != '\0')
						goto __invalid_macro;
					if (new == NULL)
					{
						newl = (start - *param) + (strlen(macro_content));
						new = malloc(newl + 1);
						strncpy(new, *param, start - *param);
						strcpy(new + (start - *param), macro_content);
					}
					else
					{
						newl += (start - end) + (strlen(macro_content));
						new = realloc(new, newl + 1);
						strncat(new, end, start - end);
						strcat(new, macro_content);
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
/*
int		replace_macro(char **param, vector_t *macro)
{
	char		*s = *param;
	char		*start, *end;
	char		*new = NULL;
	uint32_t	newl = 0;
	char		*macro_name;
	int32_t		index;
	uint8_t		parent = 0;

	if (*s == '[' || *s == '(') {
		parent += *s;
		s++;
		while (is_space(*s)) s++;
	}

	// maybe a macro
	while (is_alpha(*s) || *s == '_')
	{
		start = s;
		s++;
		while (is_alnum(*s) || *s == '_') s++;
		if (!is_space(*s) && !is_endl(*s)
				&& (parent == 0 || (new == NULL && *s != '+') || (new && *s != '*') || *s != ']' || *s != ')'))
		{
			goto __unexpected_char;
		}
		macro_name = strndup(start, s - start);
		index = vector_search(macro, macro_name);
		if (index > -1)
		{
			char		*macro_content = VEC_ELEM(macro_t, macro, index)->content;
			uint32_t	macro_argc = VEC_ELEM(macro_t, macro, index)->argc;
			uint32_t	len;

			if (macro_argc != 0)
				goto __macro_with_param;

			if (is_numeric(macro_content, &len))
			{
				macro_content += len;
				while (is_space(macro_content[len])) len++;
				if (macro_length[len] != '\0')
					goto __invalid_macro;
				if (new == NULL)
				{
					newl = (start - *param) + (strlen(macro_content));
					new = malloc(newl + 1);
					strncpy(new, *param, start - *param);
					strcpy(new + (start - *param), macro_content);
				}
				else
				{
					newl += (start - end) + (strlen(macro_content));
					new = realloc(new, newl + 1);
					strncat(new, end, start - end);
					strcat(new, macro_content);
				}
			}
			end = s;
		}
		if (*s == ']' || *s == ')') {
			parent -= *s;
			s++;
			while (is_space(*s)) s++;
		}
		if (*s == '\0')
			break;
	}

	if (new)
	{
		strcat(new, end);
		free(*param);
		*param = new;
	}
	return (0);

__macro_with_param:
__unexpected_char:
	return (-1);
__invalid_macro:
	free(macro_name);
	return (-1);
}
*/
/*
param_t	get_type(char *param1, uint16_t *n)
{
	char		*s = param1;
	int			parent = 0;
	int			num = 0;
	int			error;
	uint32_t	len;
	uint32_t	tmp;

	// addr param
	if (*s == '[' || *s == '(')
	{
		parent = (*s == '[') ? ']' : ')';
		s++;
		
		// (C)
		if (*s == 'c' && s[1] == parent && s[2] == '\0')
			return (FF00_C);
		if (*s == '0' && s[1] == 'x')
		{
			num = 1;
			s += 2;
		}
		// (ff00+x)
		if (*s == 'f')
		{
			if (s[1] == 'f' && s[2] == '0' && s[3] == '0')
			{
				if ((!num && s[4] == 'h' && s[5] == '+') || (num && s[4] == '+'))
				{
					while (*s != '+') s++;
					s++;
					if (*s == 'c' && s[1] == parent)
						return (FF00_C);
					if (is_numeric(s, &len))
					{
						tmp = atou_all(s, &error);
						if (error)
							return (UNKNOWN);
						if (tmp > 0xff)
							goto __overflow_error;
						*n = tmp;
						return (FF00_IMM8);
					}
				}
			}
		}
		if (num)
			return (UNKNOWN);
		// (rr)
		if (*s == 'a' && s[1] == 'f' && *s == parent)
			return (AF_ADDR);
		if (*s == 'b' && s[1] == 'c' && *s == parent)
			return (BC_ADDR);
		if (*s == 'd' && s[1] == 'e' && *s == parent)
			return (DE_ADDR);
		if (*s == 'h' && s[1] == 'l' && *s == parent)
			return (HL_ADDR);
		if (*s == 's' && s[1] == 'p' && *s == parent)
			return (SP_ADDR);
		// (n) (nn)
		if (is_numeric(s, &len))
		{
			if (s[len] != parent)
				return (UNKNOWN);
			tmp = atou_all(s, &error);
			if (error)
				return (UNKNOWN);
			if (tmp > 0xffff)
				goto __overflow_error;
			if (tmp > 0xff) {
				*n = tmp & 0xffff;
				return (ADDR16);
			}
			return (ADDR8);
		}
		// (HL++?) (HLI) (HL--?) (HLD)
		if (*s == 'h' && s[1] == 'l')
		{
			if (s[2] == '+' && (s[3] == parent || (s[3] == '+' && s[4] == parent)))
				return (HLI);
			if (s[2] == 'i' && s[3] == parent)
				return (HLI);
			if (s[2] == '-' && (s[3] == parent || (s[3] == '-' && s[4] == parent)))
				return (HLD);
			if (s[2] == 'd' && s[3] == parent)
				return (HLD);
		}
	}
	else
	{
		if	(
				(*s == 'a' || *s == 'b' || *s == 'c' || *s == 'd' || *s == 'e')
				&& *s == '\0'
			)
			return (A + (*s - 'a'));
		if	(
				(*s == 'h' || *s == 'l')
				&& *s == '\0'
			)
			return (H + (*s == 'l'));
		if (*s == 'a' && s[1] == 'f' && *s == '\0')
			return (AF);
		if (*s == 'b' && s[1] == 'c' && *s == '\0')
			return (BC);
		if (*s == 'd' && s[1] == 'e' && *s == '\0')
			return (DE);
		if (*s == 'h' && s[1] == 'l' && *s == '\0')
			return (HL);
		if (*s == 's' && s[1] == 'p' && *s == '\0')
			return (SP);
		if (is_numeric(s, &len))
		{
			if (s[len] != '\0')
				return (UNKNOWN);
			tmp = atou_all(s, &error);
			if (error)
				return (UNKNOWN);
			if (tmp > 0xffff)
				goto __overflow_error;
			if (tmp > 0xff) {
				*n = tmp & 0xffff;
				return (IMM16);
			}
			return (IMM8);
		}
	}
	return (SYMBOL);

__overflow_error:
	return (UNKNOWN);
}
*/
/*
"x"		// a,b,c,d,e,f,h,l
"xx"	// af,bc,de,hl,sp
"(c)"
"(ff00h+c)"
"(0xff00+c)"
"(ff00+c)"
"(xx)"	// af,bc,de,hl
"(n)"	// number (unsigned / signed) hexa/decimal/octal/binary : 0111b OU 0b0111, 0777, 12, 0xcb OU 0xCB OU cbh OU CBh OU cbH...
"(nn)"
"n"		// number "
"nn"
"(hl+)"
"(hl++)"
"(hli)"
"(hl-)"
"(hl--)"
"(hld)"
*/
/*
calculs -> addr only
var		-> addr, 16bit op only
*/


/*
	calculs -> a + b * c + d
			-> a - b * c + d
			-> a + b * c
			-> a - b * c
			-> a + b
			-> a - b

	a		= number or symbol
	b, c, d	= numbers

	val[x] = valeur du paramX ou
			si symbol, offset.
	ex:	"(toto + 2 * 3 + 1)"	-> val = 7
		"55 + 34"				-> val = 89
*/

param_t	get_type(char *param, value_t *n)
{
	char		*s = param;
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
	return (SYMBOL);
}

/*
uint16_t	calcul_param(char *param, uint16_t *n, int is_ld)
{
	int			parent = 0;
	uint32_t	len;
	uint32_t	base = 0;
	uint32_t	result = 0;
	uint8_t		neg = 0;

	if (*param == '[' || *param == '(') {
		parent = 1;
		*param = '(';
		param++;
	}

// A + b * c + d
// ^
	if (is_numeric(param, &len))
	{
		base = atou_all(param, NULL);
		param += len;
	}
	while (*param != '+' && *param != '-')
	{
		if (*param == '\0')
			goto __set_n_return;
		param++;
	}
	if (*param == '-')
		neg = 1;
	while (is_space(*param)) param++;

// a + B * c + d
//     ^
	if (is_numeric(param, &len))
	{
		result = atou_all(param, NULL);
		param += len;
	}
	while (*param != '*')
	{
		if (*param == '\0')
			goto __set_n_return;
		param++;
	}
	while (is_space(*param)) param++;

// a + b * C + d
//         ^
	if (is_numeric(param, &len))
	{
		result *= atou_all(param, NULL);
		param += len;
	}
	while (*param != '+')
	{
		if (*param == '\0')
			goto __set_n_return;
		param++;
	}
	while (is_space(*param)) param++;

// a + b * c + D
//             ^
	if (is_numeric(param, &len))
	{
		result += atou_all(param, NULL);
		param += len;
	}
	while (is_space(*param)) param++;
	if (parent && (*param == ']' || *param == ')'))
		param++;
	if (*param != '\0')
		goto __error;


__set_n_return:
	if (parent && is_ld && base == 0xff00)
	{
		
	}
	result = (neg) ? base - result : base + result;
	if (result > 0xffff)
	{
		// overflow error
	}
	*n = (result & 0xffff);
	while (is_space(param[-1])) param--;
	if (parent)
		param[0] = ')'
	return (0);

__error:
	*n = 0;
	return (-1);
}
*/


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


char	*add_instruction(char *inst, vector_t *area, vector_t *label, vector_t *macro, char *s, data_t *data)
{
	char		*param1, *param2;
	param_t		p1 = NONE, p2 = NONE;
	value_t		val[2] = {{0}};

	str_to_lower(inst);
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
		str_to_lower(param1);
		if (calcul_param(param1, val, is_ld) == -1)
			goto __error;
		printf("replace_param1 = \"%s\"\n", param1);
		p1 = get_type(param1, val); //default SYMBOL
		if (param2)
		{
			if (macro && replace_macro(&param2, macro) == -1)
				goto __error;
			str_to_lower(param2);
			if (calcul_param(param2, val + 1, is_ld) == -1)
				goto __error;
			printf("replace_param2 = \"%s\"\n", param2);
			p2 = get_type(param2, val + 1); //default SYMBOL
		}
	}
/*
	char	*symbol = param2;

	if (p1 == SYMBOL)
		symbol = param1;

	add_bin_instruction(inst, p1, p2, area, symbol, val);
	return (s);
*/
__unexpected_char:
__too_many_param:
__error:
	while (!is_endl(*s))
	s++;
	return (s);
}

/*
 *	si le type recu est imm8, c'est peut-etre une imm16.
 *	penser a changer le type si necessaire. (idem pour addr8)
 */

uint32_t	add_bin_instruction(char *inst, param_t param1, param_t param2, vector_t *area, char *symbol, uint16_t val[2])
{
	static const instructions_t	inst[71] = {
		{"adc", &&__adc},		{"add", &&__add},		{"and", &&__and},		{"bit", &&__bit},
		{"call", &&__call},		{"callc", &&__callc},	{"callnc", &&__callnc}, {"callnz", &&__callnz}
		{"callz", &&__callz},	{"ccf", &&__ccf},		{"cmp", &&__cmp},		{"cp", &&__cp},
		{"cpl", &&__cpl},		{"daa", &&__daa},		{"dec", &&__dec},		{"di", &&__di},
		{"ei", &&__ei},			{"halt", &&__halt},		{"inc", &&__inc},		{"jp", &&__jp},
		{"jpc", &&__jpc},		{"jpnc", &&__jpnc},		{"jpnz", &&__jpnz},		{"jpz", &&__jpz},
		{"jr", &&__jr},			{"jrc", &&__jrc},		{"jrnc", &&__jrnc},		{"jrnz", &&__jrnz},
		{"jrz", &&__jrz},		{"ld", &&__ld},			{"ldd", &&__ldd},		{"ldhl", &&__ldhl},
		{"ldi", &&__ldi},		{"mov", &&__mov},		{"nop", &&__nop},		{"not", &&__not},
		{"or", &&__or},			{"pop", &&__pop},		{"push", &&__push},		{"res", &&__res},
		{"reset", &&__reset},	{"ret", &&__ret},		{"retc", &&__retc},		{"reti", &&__reti},
		{"retnc", &&__retnc},	{"retnz", &&__retnz},	{"retz", &&__retz},		{"rl", &&__rl},
		{"rla", &&__rla},		{"rlc", &&__rlc},		{"rlca", &&__rlca},		{"rr", &&__rr},
		{"rra", &&__rra},		{"rrc", &&__rrc},		{"rrca", &&__rrca},		{"rst", &&__rst},
		{"sar", &&__sar},		{"sbb", &&__sbb},		{"sbc", &&__sbc},		{"scf", &&__scf},
		{"set", &&__set},		{"shl", &&__shl},		{"shr", &&__shr},		{"sla", &&__sla},
		{"sra", &&__sra},		{"srl", &&__srl},		{"stop", &&__stop},		{"sub", &&__sub},
		{"swap", &&__swap},		{"testb", &&__testb},	{"xor", &&__xor}
	};
	uint32_t	index;
	uint8_t		arg1[3];
	uint8_t		arg2[3];
	uint32_t	error1 = 0;
	uint32_t	error2 = 0;

	index = instruction_search(inst, name);
	if (index == 0xffffffffu)
		return (UNKNOWN_INST);
	
//	new_instruction( VEC_ELEM(code_area_t, area, data->cur_area) );
	goto *(inst[index].addr);

__adc:
	return (error1 | (error2 << 16));
__add:
__and:
__bit:
__call:
__callc:
__callnc:
__callnz:
__callz:
__ccf:
__cmp:
__cp:
__cpl:
__daa:
__dec:
__di:
__ei:
__halt:
__inc:
__jp:
__jpc:
__jpnc:
__jpnz:
__jpz:
__jr:
__jrc:
__jrnc:
__jrnz:
__jrz:
__ld:
__ldd:
__ldhl:
__ldi:
__mov:
__nop:
__not:
__or:
__pop:
__push:
__res:
__reset:
__ret:
__retc:
__reti:
__retnc:
__retnz:
__retz:
__rl:
__rla:
__rlc:
__rlca:
__rr:
__rra:
__rrc:
__rrca:
__rst:
__sar:
__sbb:
__sbc:
__scf:
__set:
__shl:
__shr:
__sla:
__sra:
__srl:
__stop:
__sub:
__swap:
__testb:
__xor:

}
/:;;*
char	*add_instruction(char *name, vector_t *area, vector_t *macro, vector_t *label, char *s, data_t *data)
{
	static const instructions_t	inst[71] = {
		{"adc", &&__adc},		{"add", &&__add},		{"and", &&__and},		{"bit", &&__bit},
		{"call", &&__call},		{"callc", &&__callc},	{"callnc", &&__callnc}, {"callnz", &&__callnz}
		{"callz", &&__callz},	{"ccf", &&__ccf},		{"cmp", &&__cmp},		{"cp", &&__cp},
		{"cpl", &&__cpl},		{"daa", &&__daa},		{"dec", &&__dec},		{"di", &&__di},
		{"ei", &&__ei},			{"halt", &&__halt},		{"inc", &&__inc},		{"jp", &&__jp},
		{"jpc", &&__jpc},		{"jpnc", &&__jpnc},		{"jpnz", &&__jpnz},		{"jpz", &&__jpz},
		{"jr", &&__jr},			{"jrc", &&__jrc},		{"jrnc", &&__jrnc},		{"jrnz", &&__jrnz},
		{"jrz", &&__jrz},		{"ld", &&__ld},			{"ldd", &&__ldd},		{"ldhl", &&__ldhl},
		{"ldi", &&__ldi},		{"mov", &&__mov},		{"nop", &&__nop},		{"not", &&__not},
		{"or", &&__or},			{"pop", &&__pop},		{"push", &&__push},		{"res", &&__res},
		{"reset", &&__reset},	{"ret", &&__ret},		{"retc", &&__retc},		{"reti", &&__reti},
		{"retnc", &&__retnc},	{"retnz", &&__retnz},	{"retz", &&__retz},		{"rl", &&__rl},
		{"rla", &&__rla},		{"rlc", &&__rlc},		{"rlca", &&__rlca},		{"rr", &&__rr},
		{"rra", &&__rra},		{"rrc", &&__rrc},		{"rrca", &&__rrca},		{"rst", &&__rst},
		{"sar", &&__sar},		{"sbb", &&__sbb},		{"sbc", &&__sbc},		{"scf", &&__scf},
		{"set", &&__set},		{"shl", &&__shl},		{"shr", &&__shr},		{"sla", &&__sla},
		{"sra", &&__sra},		{"srl", &&__srl},		{"stop", &&__stop},		{"sub", &&__sub},
		{"swap", &&__swap},		{"testb", &&__testb},	{"xor", &&__xor}
	};
	uint32_t	index;
	uint8_t		is_macro = 0;
	char		*macro_base_ptr;	// ptr sur le debut de la macro pour la libérer à la fin

__start:
	index = instruction_search(inst, name);
	if (index == 0xffffffffu)
	{
		if (is_macro) {
			name = get_keyword(name);
			free(macro_base_ptr);
			goto __unknown_instruction;
		}
		uint32_t	macro_index;
		char		*param[10];
		uint32_t	nparam = (s[-1] == '(');

		if ((macro_index = (uint32_t)vector_search(macro, (void*)&name)) != 0xffffffffu)
		{
			uint8_t		has_param = (VEC_ELEM(macro_t, macro, macro_index)->argc != 0);

			is_macro = 1;
			free(name);
			name = NULL;

			if (nparam != has_param)
				goto __unknown_instruction;
			if (nparam)
			{
				nparam = get_called_macro_params(&s, param);
				name = set_macro_content_with_param(param, nparam, VEC_ELEM(macro_t, macro, macro_index)->content);
				macro_base_ptr = name;
			}
			else
			{
				name = strdup(VEC_ELEM(macro_t, macro, macro_index)->content);
			}
			goto __start;
		}
		goto __unknown_instruction;
	}
	if (!is_macro)
	{
		free(name);
		name = s;
	}

	while (is_space(*name)) name++;
	new_instruction( VEC_ELEM(code_area_t, area, data->cur_area) );
	goto *(inst[index].addr);

__adc:
	if (is_8bits_register(name))


__end:
	if (is_macro)
	{
		while (!is_endl(*name)) name++;
		if (*name == '\n')
		{
			name++;
			while (is_space(*name)) name++;
			if (!is_endl(*name))
				goto __unexpected_char;
		}
	}
	free(name);


__unknown_instruction:
	sprintf(data->buf, "unknown instruction `%s`", name);
	goto __print_error;
__unexpected_char:
	sprintf(data->buf, "unexpected character `%c`", *name);
	if (macro_base_ptr)
		free(macro_base_ptr);
__print_Error:
	print_error(data->filename, data->lineno, data->line, data->buf);
	while (!is_endl(*s)) s++;
	return (s);
}
*/
void	add_label(char *name, vector_t *area, vector_t *label, data_t *data)
{
	label_t	new = {name, VEC_ELEM(code_area_t, area, 0)->addr};
	vector_push(label, (void*)&new);
}
