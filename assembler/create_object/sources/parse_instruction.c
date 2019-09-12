/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   parse_instruction.c                              .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/08/13 14:05:50 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/12 13:13:25 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "instruction_or_label.h"
#include "get_bin_instruction.h"
#include "calcul_param.h"

/*
STATIC_DEBUG const char		*get_param_type_str(enum param_e p)
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
*/

/* a recoder !!!! en 2 fonctions */
static void	instruction_replace(char **inst, char **param1, char **param2)	// "ld (ff00h)" -> ldff, "ld HL, SP" -> ldhl, "(HL++?)" -> (HLI, "(HL--?)" -> (HLD
{
	uint8_t		parent[2] = {0, 0};
	char		*p1 = *param1;
	char		*p2 = *param2;

	if (*param1 && *p1 == '[') {
		parent[0]++;
		p1++;
	}
	if (*param2 && *p2 == '[') {
		parent[1]++;
		p2++;
	}

	if (*param1 && *param2)
	{
		// remplacement of ld's alternative syntax
		if (((*inst)[0] == 'l' && (*inst)[1] == 'd' && (*inst)[2] == '\0')
			|| ((*inst)[0] == 'm' && (*inst)[1] == 'o' && (*inst)[2] == 'v' && (*inst)[3] == '\0'))
		{
			if (parent[0] == 1)
			{
				uint32_t	len = 0;

				if (LOWER(p1[0]) == 'f' && LOWER(p1[1]) == 'f' && p1[2] == '0' && p1[3] == '0')
				{
					len = 4;
					if (p1[len] == 'h')
						len++;
				}
				if (len || atou_len(p1, &len) == 0xff00u)
				{
					while (is_space(p1[len])) len++;
					if (p1[len] == '+')
					{
						len++;
						while (is_space(p1[len])) len++;
						/* suppression de la partie 0xff00+ */
						p1 = strdup(p1 + len);
						free(*param1);
						*param1 = p1;
					}
					else if (p1[len] == '\0')
					{
						/* ld (ff00h)  -> ld (ff00h + 0) */
						free(*param1);
						*param1 = strdup("0");
					}
					else
						goto __end;

					free(*inst);
					*inst = strdup("ldff");
					goto __end;
				}
			}
			if (parent[1] == 1)
			{
				uint32_t	len = 0;

				if (LOWER(p2[0]) == 'f' && LOWER(p2[1]) == 'f' && p2[2] == '0' && p2[3] == '0')
				{
					len = 4;
					if (p1[len] == 'h')
						len++;
				}
				if (atou_len(p2, &len) == 0xff00u)
				{
					while (is_space(p2[len])) len++;
					if (p2[len] == '+')
					{
						len++;
						while (is_space(p2[len])) len++;
						/* suppression de la partie 0xff00+ */
						p2 = strdup(p2 + len);
						free(*param2);
						*param2 = p2;
					}
					else if (p2[len] == '\0')
					{
						/* ld (ff00h)  -> ld (ff00h + 0) */
						free(*param2);
						*param2 = strdup("0");
					}
					else
						goto __end;

					free(*inst);
					*inst = strdup("ldff");
					goto __end;
				}
			}


			if (LOWER((*param1)[0]) == 'h' && LOWER((*param1)[1]) == 'l' && LOWER((*param1)[2]) == '\0' 
					&& LOWER((*param2)[0]) == 's' && LOWER((*param2)[1]) == 'p')
			{
				if ((*param2)[2] == '+')
				{
					memmove(*param2, *param2 + 3, strlen(*param2 + 2));
				}
				else if ((*param2)[2] == '\0')
				{
					free(*param2);
					*param2 = strdup("0");
				}
				else
					goto __end;

				free(*inst);
				*inst = strdup("ldhl");
				free(*param1);
				*param1 = strdup("SP");
			}
		}
	}
__end:
	return;
}

#define IS_LABEL	0xffu

char	*replace_macro_without_param(char *s, const uint32_t len, vector_t *macro, data_t *data, uint8_t *is_string)
{
	const char	char_backup = s[len];

	s[len] = '\0';
	ssize_t	index = vector_search(macro, (void*)&s);
	s[len] = char_backup;

	if (index == -1)
		return (NULL);



	register macro_t	*m = VEC_ELEM(macro_t, macro, index);
	if (m->argc != 0)
		goto __error_macro_with_param;


/*
	register char	*p = m->content;

	// verify macro content validity
	if (is_alpha(*p) || *p == '_')
	{
		p++;
		while (is_alnum(*p) || *p == '_')
			p++;
		while (is_spce(*p)) p++;
		if (is_operator(*p))
		{
			if (*p == '<' || *p == '>')
			{
				p++;
				if (p[-1] != *p)
					goto __unexpected_char;
			}
			p++;
		}
		*is_string = 1;
	}
	else if (is_digit(*p))
	{
		uint32_t	len;

		if (is_numeric_len(p, &len) == 0)
			goto __error_syntax_digit;
		p += len;
		*is_string = 0;
	}
	else
		goto __unexpected_char;

	if (*p != '\0')
		goto __unexpected_char;
*/
	return (m->content);


	register const char	*error_msg;
__error_macro_with_param:
	error_msg = "can't use macro with parameters as argument";
	goto __print_error;
__error_syntax_digit:
	error_msg = "error digit";
	goto __print_error;
//__unexpected_char:
//	sprintf(data->buf, "in macro `%s` content, unexpected character `%c`", m->name, *p);
__print_error_fmt:
	error_msg = (const char *)data->buf;
__print_error:
	print_error(data->filename, data->lineno, data->line, error_msg);
	return (NULL);
}


int	__attribute__((always_inline))
set_mnemonic_and_params(char **s, char **mnemonic, char **param1, char **param2, uint8_t *n_params, data_t *data, vector_t *macro)
{
	char		*start;
	char		*param_buf = NULL;
	uint32_t	param_len = 0;

	/*
	**	SET MNEMONIC
	*/
	if (**s == ':')
	{
		(*s)++;
		goto __empty_label;
	}
	if (!is_alpha(**s) && **s != '_')
		goto __unexpected_char;

	start = *s;
	while (!is_space(**s) && !is_endl(**s))
	{
		if (**s == ':')
		{
			*mnemonic = strndup(start, *s - start);
			(*s)++;
			*n_params = IS_LABEL;
			return (0);
		}
		if (!is_alnum(**s) && **s != '_')
			goto __unexpected_char;
		(*s)++;
	}
	*mnemonic = strndup(start, *s - start);




	uint8_t		i = 0;
	char		is_addr = 0;
	uint8_t		parent;
	uint8_t		is_string;
	uint8_t		not;
	uint8_t		neg;
	uint8_t		first_param;


	/*
	**	LOOP IN EACH PARAMETER
	*/
	while (i < 2)
	{
		while (is_space(**s)) (*s)++;

		if (is_endl(**s))
		{
			if (i == 0)
				break;
			else
				goto __incomplete_param; // example: "ld A, "
		}
		if (**s == '[')
		{
			is_addr = 1;
			(*s)++;
			while (is_space(**s)) (*s)++;
		}
		first_param = 1;

		param_len = 0;
		param_buf = NULL;
		while (1)
		{
			/*
			**	get operand
			*/
			parent = 0;
			while (**s == '(') {
				parent++;
				(*s)++;
			}

			if (**s == '~') {
				not = 1;
				(*s)++;
			}
			else
				not = 0;

			if (**s == '-') {
				neg = 1;
				(*s)++;
			}
			else
				neg = 0;
			start = *s;

			if (is_alpha(*start) || *start == '_')
			{
				// replace hl+ and hl- with hli or hld
				if (is_addr && !not && !neg
						&& LOWER(*start) == 'h' && LOWER(start[1]) == 'l'
						&& (start[2] == '+' || start[2] == '-'))
				{
					register char	*tmp = start + 3;
					register char	sign = start[2];

					if (tmp[0] == tmp[-1])
						tmp++;
					while (is_space(*tmp)) tmp++;
					if (*tmp == ']')
					{
						is_addr = 0;
						tmp++;
						while (is_space(*tmp)) tmp++;
						if (is_endl(*tmp) || *tmp == ',')
						{
							if (*tmp == ',') tmp++;
							param_len = 4;
							param_buf = (sign == '-') ? strndup("[HLD", param_len) : strndup("[HLI", param_len);
							*s = tmp;
							goto __next_param;
						}
					}
				}
				is_string = 1;
				(*s)++;
				while (is_alnum(**s) || **s == '_')
					(*s)++;
			}
			else if (is_digit(*start))
			{
				if (is_numeric_inc(s) == 0)
					goto __error_syntax_digit;
				is_string = 0;
			}
			else
				goto __unexpected_char;

			if (!is_space(**s) && !is_endl(**s) && **s != ',' && !is_operator(**s) && (is_addr && **s != ']'))
				goto __unexpected_char;





			/*
			**	dup param or dup macro content
			*/
			if (is_string)
			{
				register char		*macro_content;

				macro_content = replace_macro_without_param(start, *s - start, macro, data, &is_string);
				if (macro_content == NULL)
				{
					if (!first_param)
						goto __operands_are_numeric_value_only;
					if (not)
						goto __unexpected_not_operation;
					if (neg)
						goto __unexpected_minus_character;

					param_buf = malloc((*s - start) + 5 + parent);
					if (is_addr)
						param_buf[param_len++] = '[';
					while (parent) {
						param_buf[param_len++] = '(';
						parent--;
					}
					strncpy(param_buf + param_len, start, *s - start);
					param_len += (*s - start);
				}
				else
				{
					// create a function
					if (first_param)
					{
						param_buf = malloc(strlen(macro_content) + 3 + parent);
						if (is_addr)
							param_buf[param_len++] = '[';
					}
					else
						param_buf = realloc(param_buf, param_len + strlen(macro_content) + 2 + parent);

					while (parent) {
						param_buf[param_len++] = '(';
						parent--;
					}
					if (not)
						param_buf[param_len++] = '~';
					if (neg)
						param_buf[param_len++] = '-';
					strncpy(param_buf + param_len, macro_content, strlen(macro_content));
					param_len += strlen(macro_content);
				}
			}
			else
			{
				// the same function
				if (first_param)
				{
					param_buf = malloc((*s - start) + 3 + parent);
					if (is_addr)
						param_buf[param_len++] = '[';
				}
				else
					param_buf = realloc(param_buf, param_len + (*s - start) + 2 + parent);

				while (parent) {
					param_buf[param_len++] = '(';
					parent--;
				}
				if (not)
					param_buf[param_len++] = '~';
				if (neg)
					param_buf[param_len++] = '-';
				strncpy(param_buf + param_len, start, *s - start);
				param_len += (*s - start);
			}
			first_param = 0;



			/*
			**	add closed parents
			*/
			while (is_space(**s)) (*s)++;

			while (**s == ')')
			{
				parent++;
				(*s)++;
			}
			if (parent)
			{
				param_buf = realloc(param_buf, param_len + parent + 2);
				while (parent)
				{
					param_buf[param_len++] = ')';
					parent--;
				}
				while (is_space(**s)) (*s)++;
			}

			if (is_addr && **s == ']')
			{
				(*s)++;
				while (is_space(**s)) (*s)++;
				is_addr = 0;
			}





			// verify last character
			if (**s == ',')
			{
				(*s)++;
				goto __next_param;
			}
			else if (is_endl(**s))
				goto __next_param;
			else if (is_operator(**s))
			{
				if (**s == '>' || **s == '<')
				{
					(*s)++;
					if ((*s)[-1] != **s)
						goto __unexpected_char;
				}

				param_buf[param_len++] = **s;
				(*s)++;
				while (is_space(**s)) (*s)++;
			}
			else
				goto __unexpected_char;
		}

	__next_param:
		if (is_addr)
			goto __expected_parent; // [ ]
		param_buf[param_len] = 0;
		if (i == 0)
			*param1 = param_buf;
		else
			*param2 = param_buf;
		param_buf = NULL;
		param_len = 0;
		if (is_endl(**s))
			break;

		//parent = 0;
		i++;
	}
	*n_params = i + 1;	// if n_params == 3: too_many_arguments
	return (0);


	register const char	*error_msg;
/* ERRORS */
__operands_are_numeric_value_only:
	error_msg = "first operand only can be a symbol";
	goto __print_error;
__unexpected_not_operation:
	error_msg = "cannot use not operator in a symbol";
	goto __print_error;
__unexpected_minus_character:
	error_msg = "cannot use minus character in a symbol";
	goto __print_error;
__error_syntax_digit:
	error_msg = "error digit";
	goto __print_error;
__empty_label:
	error_msg = "empty label";
	goto __print_error;
__expected_parent:
	print_error(data->filename, data->lineno, data->line, "missing closed square parent at end of expression");
	goto __print_error;
__incomplete_param:
	print_error(data->filename, data->lineno, data->line, "empty parameter");
	goto __print_error;
__unexpected_char:
	sprintf(data->buf, "unexpected character `%c`", **s);
__print_error_fmt:
	error_msg = data->buf;
__print_error:
	print_error(data->filename, data->lineno, data->line, error_msg);

__free_all:
	if (*mnemonic) free(*mnemonic);
	if (*param1) free(*param1);
	if (*param2) free(*param2);
	if (param_buf) free(param_buf);
	while (!is_endl(**s)) (*s)++;
	return (-1);
}

param_t	get_param_type(char *param, value_t *n)
{
	char		*s = param;
	int			parent = 0;


	// addr param
	if (*s == '[')
	{
		parent++;
		s++;
		
		// (C)
		if (LOWER(*s) == 'c' && s[1] == '\0')
			return (FF00_C);
		if (LOWER(*s) == 'f' && LOWER(s[1]) == 'f' && s[2] == '0' && s[3] == '0'
				&& (s[4] == 0 || (s[4] == 'h' && s[5] == '0')))
			return (FF00_IMM8);
		// (n) (nn)
		if (is_digit(*s) || *s == '(' || *s == '~' || *s == '-')
		{
			if (n->value > 0xff)
				return (ADDR16);
			return (ADDR8);
		}
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
			if (LOWER(s[2]) == 'i' && s[3] == '\0')
				return (HLI);
			if (LOWER(s[2]) == 'd' && s[3] == '\0')
				return (HLD);
			return (UNKNOWN);
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
		if (LOWER(*s) == 'n')
		{
			if (LOWER(s[1]) == 'z' && s[2] == '\0')
				return (_NZ_);
			if (LOWER(s[1]) == 'c' && s[2] == '\0')
				return (_NC_);
		}
		else if (LOWER(*s) == 'z' && s[1] == '\0')
			return (_Z_);
		else if (LOWER(*s) == 'c' && s[1] == '\0')
			return (_C_);
		else if (is_digit(*s) || *s == '(' || *s == '~' || *s == '-')
		{
			if (n->value > 0xff)
				return (IMM16);
			return (IMM8);
		}
		return (SYMBOL);
	}
	if (is_alpha(*param) || *param == '_')
		return (SYMBOL);
	return (UNKNOWN);
}

char	*replace_macro(char **s, vector_t *macro, data_t *data)
{
	char		*p = *s;
	char		*param[10] = {NULL};
	uint32_t	n_params = 0;
	char		*content;
	char		*end;
	macro_t		*m;

	if (!is_alpha(*p) && *p != '_')
		return (NULL);
	p++;
	while (is_alnum(*p) || *p == '_') p++;
	end = p;
	if (*p == '(')
	{
		p++;
		while (is_space(*p)) p++;
		if (*p == ')')
		{
			p++;
			goto __no_arguments;
		}
		*end = 0;
		m = get_macro(macro, *s);
		*end = '(';
		if (m == NULL)
			return (NULL);

		content = m->content;
		n_params = get_params(&p, param);

		if (*p != ')')
			goto __unexpected_char;
		p++;
		while (is_space(*p)) p++;
		if (!is_endl(*p))
			goto __unexpected_char;

		if (n_params == 0xffu)
			goto __undefined_macro_param_error;
		if (n_params != m->argc)
			goto __argc_error;
		if (n_params > 10)
			goto __too_many_params; // inutile ?
		content = replace_content(m, param);
		*s = p;
		return (content);
	}

	__no_arguments:
	while (is_space(*p)) p++;
	if (!is_endl(*p))
		return (NULL);

	char	tmp = *end;
	*end = 0;
	m = get_macro(macro, *s);
	*end = tmp;
	if (m == NULL)
		return (NULL);
	if (m->argc)
		goto __params_expected;
	*s = p;
	return (strdup(m->content));


	const char	*error_msg;
__unexpected_char:
	sprintf(data->buf, "unexpected character `%c`", *p);
	goto __print_error_fmt;
__undefined_macro_param_error:
	error_msg = "undefined macro arguments error";
	goto __print_error;
__params_expected:
	sprintf(data->buf, "too few arguments (%u expected, 0 given)", m->argc);
	goto __print_error_fmt;
__too_many_params:
	sprintf(data->buf, "too many arguments (%u expected, %u given)", m->argc, n_params);
	goto __print_error_fmt;
__argc_error:
	*end = 0;
	sprintf(data->buf, "wrong number of arguments in macro %s", *s);
	*end = '(';
	goto __print_error;
__print_error_fmt:
	error_msg = (const char *)data->buf;
__print_error:
	print_error(data->filename, data->lineno, data->line, error_msg);
	return (NULL);
}

char	*parse_instruction(char *s, vector_t *area, vector_t *ext_symbol, loc_sym_t *loc_symbol, vector_t *macro, data_t *data)
{
	char		*mnemonic = NULL;
	char		*param1 = NULL;
	char		*param2 = NULL;
	uint8_t		n_params;
	char		*replacement;

	// replace line if it's a macro w/wo params
	replacement = replace_macro(&s, macro, data);	// return NULL if no macro
	// lexer (check spaces, comma, ...). cut the string
	/* example:
	**   "ld A, B"
	** -->
	**   mnemonic	= "ld"
	**   param1		= "A"
	**   param2		= "B"
	**   n_params	= 2
	*/
	if (replacement) {
//		char	*p = replacement;
		char	*line = data->line;
		char	*tmp = replacement;

		data->line = replacement;
		/* /!\ DANGER: risque de recursive infinie: interdire les appels recursifs dans une macro /!\ */
		while (1)
		{
			tmp = parse_instruction(tmp, area, ext_symbol, loc_symbol, macro, data);
			if (*tmp == '\0')
				break;
			tmp++;
			data->line = tmp;
		}
//		if (set_mnemonic_and_params(&p, &mnemonic, &param1, &param2, &n_params, data, macro) == -1)
//		{
			data->line = line;
			free(replacement);
//			data->line = line;
			return (s);
//		}
//		replacement = line;
	}
	else {
		if (set_mnemonic_and_params(&s, &mnemonic, &param1, &param2, &n_params, data, macro) == -1)
			return (s);
	}

	if (n_params == 3)
		goto __too_many_parameters;

	// if it's a label, add it and return
	if (n_params == IS_LABEL)
	{
		add_label(mnemonic, area, ext_symbol, loc_symbol, data);
		return (s);
	}


	/* parse params
	** example:
	**   "ld A, 10+5"
	** -->
	**   param[0] = A
	**   param[1] = IMM8
	**   val = {0, 15, 0} ?
	*/
	param_t		param[2] = {NONE, NONE};	//enum ?
	value_t		val = {0, 0};

	str_to_lower(mnemonic);
	if (n_params == 2)
		instruction_replace(&mnemonic, &param1, &param2);	// "ld (ff00h)" -> ldff, "ld HL, SP" -> ldhl
	
	if (param1)
	{
		if (calcul_param(param1, &val, data, 1) == 0xffffffffu)
		{
			puts("calcul_param(param1) ERROR");
			goto __free_and_ret;
		}
		param[0] = get_param_type(param1, &val);

		if (param2)
		{
			value_t	tmp_val = {0, 0};
			if (calcul_param(param2, &tmp_val, data, 2) == 0xffffffffu)
			{
				puts("calcul_param(param2) ERROR");
				goto __free_and_ret;
			}
			param[1] = get_param_type(param2, &tmp_val);

			if (param[0] < FF00_IMM8 && param[1] >= FF00_IMM8)
				val = tmp_val;
		}
	}
//	printf("\e[0;36m%s %s, %s\e[0m (%s, %s) :: \e[0;33mvalue = %u\e[0m\n", mnemonic, param1, param2, get_param_type_str(param[0]), get_param_type_str(param[1]), val.value);

	char	*symbol = param2;
	if (param[0] == SYMBOL)
	{
		symbol = param1;
		free(param2);
	}
	else
		free(param1);


	param_error_t	error;
	uint8_t			bin[4] = {0, 0, 0, val.sign};

	error = get_bin_instruction(mnemonic, param, &val, bin);
	if (error.p1 || error.p2)
	{
		const char *errtable[OVERFLOW + 1] = {
			"Missing parameter for instruction",
			"Too many parameters for instruction",
			"Invalid destination for instruction",
			"Invalid source for instruction",
			"Address overflow"
		};
		
		if (error.p1 == UNKNOWN_INSTRUCTION)
		{
			sprintf(data->buf, "unknown instruction %s", mnemonic);
			print_error(data->filename, data->lineno, data->line, data->buf);
			goto __free_and_ret;
		}
		if (error.p1)
		{
			sprintf(data->buf, "%s", errtable[error.p1]);
			print_error(data->filename, data->lineno, data->line, data->buf);
		}
		if (error.p2)
		{
			sprintf(data->buf, "%s", errtable[error.p2]);
			print_error(data->filename, data->lineno, data->line, data->buf);
		}
		goto __free_and_ret;
	}
	else
		push_instruction(VEC_ELEM(code_area_t, area, data->cur_area), bin, param, symbol, ext_symbol, loc_symbol, data);

	if (replacement) {
		free(data->line);
		data->line = replacement;
	}
	return (s);

	/* ERRORS */
__too_many_parameters:
	fprintf(stderr, "TOO MANY PARAM\n");
__free_and_ret:
	if (mnemonic)
		free(mnemonic);
	if (replacement) {
		free(data->line);
		data->line = replacement;
	}
	return (s);
}
