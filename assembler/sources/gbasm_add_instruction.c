/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_add_instruction.c                          .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/16 22:10:25 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/08 08:45:04 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "gbasm_struct.h"
#include "gbasm_struct_tools.h"
#include "gbasm_tools.h"
#include "gbasm_error.h"
#include "gbasm_get_bin_instruction.h"

/*
**	isolate operands without spaces inside. if value = "(toto + 1)", result is "(toto+1"
*/
int		set_params(char **param1, char **param2, char **s)
{
	uint8_t		parent = 0;
	uint32_t	len, i;
	char		*tmp;
	*param1 = NULL;
	*param2 = NULL;


	while (is_space(**s)) (*s)++;
	if (is_endl(**s))
		return (0);

	/*
	** isolate parameter 1
	*/
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
	printf("ISOLATE PARAM1 = \"%s\"\n", (*param1));

	if (is_endl(**s))
		return (0);
	if (**s != ',')
		goto __unexpected_char;
	(*s)++;
	parent = 0;


	/*
	** isolate parameter 2
	*/
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
	printf("ISOLATE PARAM2 = \"%s\"\n", (*param2));

	if (**s == ',') goto __too_many_arguments;
	if (!is_endl(**s)) goto __unexpected_char;
	return (0);


__too_many_arguments:
	g_error++;
	puts("TOO MANY ARGUMENTS\n");
	return (-1);

__parent_error:
	g_error++;
	puts("parent error\n");
	return (-1);

__unexpected_char:
	g_error++;
	puts("unexpected character\n");
	return (-1);
}

/*
**	exception inutile ???
*/
int		replace_macro(char **param, vector_t *macro, int exception, char **mne)	// and delete spaces
{
	char		*s = *param;
	char		*start, *end;
	char		*new_content[5] = {NULL};
	uint32_t	total_length = 0;
	char		new_operator[5][2] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
	char		*macro_name;
	int32_t		index;
	uint8_t		parent = 0;
	uint8_t		count = 0;

	if (*s == '[' || *s == '(') {
		parent = 1;
		s++;
		while (is_space(*s)) s++;
	}

	// remplacment of ld's alternative syntaxes
	if (exception)
	{
		if (parent)
		{
			int			type;
			uint32_t	len;

			puts("EXCEPTION");
			if ((type = is_numeric(s, &len)) != 0)
			{
				if (atou_type(s, NULL, type) == 0xff00u)
				{
					printf("PARAM[%u] = '%c'\n", len, (*param)[len + 1]);
					if (s[len] == '+')
					{
						/* suppression de la partie 0xff00+ */
						char	*tmp = *param + len + 2;
						printf("tmp = \"%s\"\n", tmp);
						tmp = strdup(tmp);
						free(*param);
						*param = tmp;
						s = *param;
						parent = 0;
						printf("\e[1;32mNEWPARAM = \e[0m\"%s\"\n", tmp);
					}
					else if (s[len] == '\0')
					{
						/* ld (ff00h)  -> ld (ff00h + 0) */
						free(*param);
						*param = strdup("0");
						s = *param;
					}
					else
						goto __no_exception;

					free(*mne);
					*mne = strdup("ldff");
				}
			}
		}
	}
__no_exception:

	// maybe a macro
	while (!is_endl(*s) && *s != ']' && *s != ')' && count != 4)
	{
		if (is_alpha(*s) || *s == '_')
		{
			start = s;
			s++;
			while (is_alnum(*s) || *s == '_') s++; // s = identifier's end
			end = s;

			printf("*s = '%c'\n", *s);
			/*
			**	check if next character is valid (a +/- b * c + d)
			*/
			if (!is_space(*s) && !is_endl(*s) && !is_operator(*s) && *s != ']' && *s != ')')
				goto __unexpected_char;
			if (is_operator(*s))
			{
				if ((count == 0 && *s == '*') || (count == 1 && *s != '*') || (count == 2 && *s != '+') || count == 3)
					goto __unexpected_operator;
			}
			new_operator[count][0] = *s;

			/*
			**	copy identifier's name in macro_name
			*/
			macro_name = strndup(start, end - start);

			/*
			**	search identifier into macro list
			*/
			index = vector_search(macro, (void*)&macro_name);
			if (index > -1)		// identifier founded
			{
				char		*macro_content = VEC_ELEM(macro_t, macro, index)->content;
				uint32_t	macro_argc = VEC_ELEM(macro_t, macro, index)->argc;
				uint32_t	len;

				// macro with param forbidden into operands
				if (macro_argc != 0)
					goto __macro_with_param;

				if (is_numeric(macro_content, &len))
				{
					char	*value = macro_content;

					// check if no characters after number
					while (is_space(macro_content[len])) len++;
					if (macro_content[len] != '\0')
						goto __invalid_macro;

					new_content[count] = strndup(value, len);
					total_length += len;
				}
				else
				{
					char	*value = macro_content;
					char	*end = value + strlen(value + 1);

					// check if no characters after number
					while (is_space(*value)) value++;
					while (is_space(end[-1])) end--;

					new_content[count] = strndup(value, end - value);
					total_length += end - value;
				}
				free(macro_name);
			}
			else
			{
				new_content[count] = macro_name;
				total_length += end - start;
			}

			if (is_endl(*s))
				break;
			s++;
			while (is_space(*s)) s++;
		}
		else
		{
			start = s;
			while (is_alnum(*s)) s++;
			end = s;
			if (!is_space(*s) && !is_endl(*s) && !is_operator(*s) && *s != ']' && *s != ')')
				goto __unexpected_char;
			if (is_operator(*s))
			{
				if ((count == 0 && *s == '*') || (count == 1 && *s != '*') || (count == 2 && *s != '+') || count == 3)
					goto __unexpected_operator;
			}
			new_operator[count][0] = *s;
			new_content[count] = strndup(start, end - start);
			total_length += end - start;


			if (is_endl(*s))
				break;
			while (is_space(*s)) s++;
			s++;
		}
		count++;
	}

	char	*new_operand = malloc(total_length + 1 + count + parent);
	if (parent)
	{
		new_operand[0] = '(';
		new_operand[1] = '\0';
	}
	else
		new_operand[0] = 0;
	new_operator[4][0] = '\0';

	for (uint32_t i = 0; new_content[i]; i++)
	{
		strcat(new_operand, new_content[i]);
		strcat(new_operand, new_operator[i]);
	}
	free(*param);
	printf("new_operand = \e[0;33m\"%s\"\e[0m\n", new_operand);
	*param = new_operand;

	return (0);

__unexpected_operator:
	g_error++;
	fprintf(stderr, "unexpected operator %c\n", *s);
	return (-1);
__macro_with_param:
	g_error++;
	fprintf(stderr, "ERROR: macro with_param...\n");
	return (-1);
__unexpected_char:
	g_error++;
	fprintf(stderr, "unexpected char %c\n", *s);
	return (-1);
__invalid_macro:
	fprintf(stderr, "invalid macro\n");
	free(macro_name);
	return (-1);
}

param_t	get_type(char *param, value_t *n)
{
	char		*s = param;
	int			parent = 0;
	int			num = 0;
	uint32_t	len;


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
		/*
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
		*/
		// (n) (nn)
		if (is_numeric(s, &len))
		{
			if (n->value > 0xff)
				return (ADDR16);
			return (ADDR8);
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
		return (SYMBOL);
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
		return (SYMBOL);
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


int		calcul_param(char *param, value_t *n, int exception)
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
	if (parent && exception && base == 0xff00 && minus == 0)
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

STATIC_DEBUG const char		*get_param_type(enum param_e p)
{
	switch (p)
	{
		case UNKNOWN: return ("UNKNOWN");
		case NONE: return ("NONE");
		case A: return ("A");
		case B: return ("B");
		case C: return ("C");
		case D: return ("D");
		case E: return ("E");
		case F: return ("F");
		case H: return ("H");
		case L: return ("L");
		case AF: return ("AF");
		case BC: return ("BC");
		case DE: return ("DE");
		case HL: return ("HL");
		case SP: return ("SP");
		case _NZ_: return ("_NZ_");
		case _Z_: return ("_Z_");
		case _NC_: return ("_NC_");
		case _C_: return ("_C_");
		case HLI: return ("HLI");
		case HLD: return ("HLD");
		case SP_ADDR: return ("SP_ADDR");
		case HL_ADDR: return ("HL_ADDR");
		case BC_ADDR: return ("BC_ADDR");
		case DE_ADDR: return ("DE_ADDR");
		case AF_ADDR: return ("AF_ADDR");
		case FF00_C: return ("FF00_C");
		case FF00_IMM8: return ("FF00_IMM8");
		case SP_IMM8: return ("SP_IMM8");
		case IMM8: return ("IMM8");
		case ADDR8: return ("ADDR8");
		case IMM16: return ("IMM16");
		case ADDR16: return ("ADDR16");
		case SYMBOL: return ("SYMBOL");
	}
	return ("PARAM TYPE ERROR");
}

char	*add_instruction(char *inst, vector_t *area, vector_t *ext_symbol, loc_sym_t *loc_symbol, vector_t *macro, char *s, data_t *data)
{
	char		*param1, *param2;
	param_t		param[2] = {NONE, NONE};
	value_t		val = {0, 0, 0};
	int			exception = 0;
//	char		*mne = inst;

	str_to_lower(inst);
	printf("MNEMONIC = \"%s\"\n", inst);
	if (*inst == 'l' && inst[1] == 'd' && inst[2] == '\0')
		exception = 1;
	if (set_params(&param1, &param2, &s) == -1)
	{
		// error checked in set_params()
		if (*s == ',')
			goto __too_many_param;
		goto __unexpected_char;
	}

	if (param1)
	{
		if (macro && replace_macro(&param1, macro, exception, &inst) == -1) //without params only
			goto __error;
		if (calcul_param(param1, &val, exception) == -1)
			goto __error;
		param[0] = get_type(param1, &val); //default SYMBOL
		printf("\e[0;33mPARAM1 after replace\e[0m = \"%s\" (%s)\n", param1, get_param_type(param[0]));

		if (param2)
		{
			value_t	tmp_val = {0, 0, 0};

			if (macro && replace_macro(&param2, macro, exception, &inst) == -1)
				goto __error;
			if (calcul_param(param2, &tmp_val, exception) == -1)
				goto __error;
			param[1] = get_type(param2, &tmp_val); //default SYMBOL
			printf("\e[0;33mPARAM2 after replace\e[0m = \"%s\" (%s)\n", param2, get_param_type(param[1]));
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
	if (error.p1 || error.p2)
	{
		g_error++;
		const char *errtable[OVERFLOW + 1] = {
			"Missing parameter for instruction\n",
			"Too many parameters for instruction\n",
			"Invalid destination for instruction\n",
			"Invalid source for instruction\n",
			"Address overflow\n"
		};
		if (error.p1)
			fprintf(stderr, errtable[error.p1]);
		else
			fprintf(stderr, errtable[error.p2]);
	}
	else
		push_instruction(VEC_ELEM(code_area_t, area, data->cur_area), bin, param, symbol, ext_symbol, loc_symbol, data);
	return (s);

__unexpected_char:
__too_many_param:
__error:
	g_error++;
	while (!is_endl(*s))
	s++;
	return (s);
}
