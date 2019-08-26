/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   parse_instruction.c                              .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/08/13 14:05:50 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/13 15:22:22 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

static uint8_t		get_params(char **s, char *param[10])
{
	char	*start;
	uint8_t	nparam = 0;

	for (;;)
	{
		while (is_space(**s)) s++;
		start = *s;
		while (**s != ',' && !is_endl(**s) && **s != ')') (*s)++;
		if (!is_endl(**s))
		{
			char	*end = *s;

			if (nparam == 10)
				return (0xffu); // too many params
			if (end == start)
				return (0xffu); // empty param
			while (is_space(end[-1])) {
				end--;
				if (end == start)
					return (0xffu); // empty param
			}
			param[nparam++] = strndup(start, end - start);
			if (**s == ')')
				break;
			(*s)++;
		}
		if (is_endl(**s))
		{
			while (nparam)
				free(param[--nparam]); // missing ')'
			return (0xffu);
		}
	}
	return (nparam);
}

static char	*replace_content(char *content, uint32_t argc, char *param[10])
{
	char		*new = NULL;
	int32_t		diff = 0;
	uint32_t	len[10];
	char		*pos;
	char		*tofree = content;

	for (uint32_t i = 0; i != argc; i++)
	{
		len[i] = strlen(param[i]);
		diff += len[i] - 2;
	}

	new = malloc(strlen(content) + diff + 1);
	new[0] = '\0';
	pos = content;
	while ((pos = strchr(content, '#')))
	{
		register uint8_t	i = pos[1] - '0';

		strncat(new, content, pos - content);
		strncat(new, param[i], len[i]);
		content = pos + 2;
	}
	strcat(new, content);
	free(tofree);
	return (new);
}

#define IS_LABEL	0xffu
/*
typedef struct	lex_param_s
{
	uint32_t	len;		// string length
	uint32_t	info;		// garbage
	char		*str;		// string pointer
}
lex_param_t;
*/
char	*replace_if_macro_without_param(char *s, const uint32_t len, vector_t *macro, data_t *data)
{
	const char	char_backup = s[len];

	s[len] = '\0';
	ssize_t	index = vector_search(macro, (void*)&s);

	if (index == -1)
	{
		s[len] = char_backup;
		return (strndup(s, len));
	}

	register macro_t	*m = VEC_ELEM(macro_t, macro, index);
	if (m->argc != 0)
	{
		print_error(data->filename, data->lineno, data->line, "can't use macro with parameters as argument");
		return (NULL);
	}

	register char	*p = m->content;

	// verify macro content validity
	if (LOWER(*p) >= 'a' && LOWER(*p) <= 'f')
	{
		uint32_t	len;

		if (is_numeric(p, &len) == 0 || is_alnum(p[len]) || p[len] == '_')
		{
			p += len;
			goto __string;
		}
	}
	if (is_alpha(*p) || *p == '_')
	{
		p++;
	__string:
		while (!is_space(*p) && !is_end(*p) && *p != ',')
		{
			if (!is_alnum(*p) && *p != '_')
				goto __unexpected_char;
			p++;
		}
	}
	else if (is_digit(*p))
	{
		uint32_t	len;

		if (is_numeric(p, &len) == 0)
			goto __error_syntax_digit;
		p += len;
	}
	else
		goto __unexpected_char;

	if (*p != '\0')
		goto __unexpected_char;

	return (strdup(m->content));

	register char	*error_msg;
__unexpected_char:
	sprintf(data->buf, "in macro `%s` content, unexpected character `%c`", m->name, *p);
__print_error_fmt:
	error_msg = data->buf;
__print_error:
	print_error(data->filename, data->lineno, data->line, error_msg);
	return (NULL);
}

int	__attribute__((always_inline))
set_mnemonic_and_params(char **s, char **mnemonic, char **param1, char **param2, uint8_t *n_params, data_t *data)
{
	uint8_t	i = 0;
	char	*start;
	int		type;

	// empty label
	if (**s == ':')
	{
		(*s)++;
		goto __empty_label;
	}
	// invalid first character
	if (!is_alpha(**s) && **s != '_')
		goto __unexpected_char;

	// set mnemonic
	start = *s;
	if (is_space(**s) && !is_endl(**s))
	{
		if (**s == ':') // label
		{
			*mnemonic = strndup(start, *s - start);
			(*s)++;
			*n_params = IS_LABEL;
			return (0);
		}
		if (!is_alnum(**s) && **s != '_')
			goto __unexpected_char;
		(*s)++; //check wrong characters ??
	}
	*mnemonic = strndup(start, *s - start);



	while (i < 2)	// loop in each parameter
	{
		while (is_space(**s)) (*s)++;
		if (is_endl(**s))
		{
			if (i == 0)
				break;
			else
				goto __incomplete_param; // example: "ld A, "
		}
		start = *s;


		// verify all characters
		if (LOWER(*start) >= 'a' && LOWER(*start) <= 'f')
		{
			uint32_t	len;

			if (is_numeric(start, &len) == 0 || is_alnum(start[len]) || start[len] == '_')
			{
				*s += len;
				goto __string;
			}
		}
		if (is_alpha(*start) || *start == '_')
		{
			(*s)++;
		__string:
			while (!is_space(**s) && !is_end(**s) && **s != ',')
			{
				if (!is_alnum(**s) && **s != '_')
					goto __unexpected_char;
				(*s)++;
			}
		}
		else if (is_digit(*start))
		{
			uint32_t	len;

			if (is_numeric(start, &len) == 0)
				goto __error_syntax_digit;
			*s += len;
		}
		else
			goto __unexpected_char;

		// dup param or dup macro content
		{
			register char	*param = replace_if_macro_without_param(start, *s - start, macro);

			if (param == NULL)
				goto __free_all;
			if (i == 0)
				param1 = param;
			else
				param2 = param;
		}
		while (is_space(**s)) (*s)++;
		// verify last character
		if (**s == ',')
		{
			(*s)++;
			continue;
		}
		else if (is_endl(**s))
		{
			break;
		}
		else if (**s == '+' || **s == '-' || **s == '*' || **s == '/'
				|| **s == '&' || **s == '|' || **s == '^'
				|| ((s[0] == '<' || s[0] == '>') && s[1] == s[0]))
		{
			// function (replace all macro)
		}
		else
			goto __unexpected_char;

		i++;
	}
	*n_param = i + 1;	// if n_params == 3: too_many_arguments
	return (0);

/* ERRORS */
__incomplete_param:
	print_error(data->filename, data->lineno, data->line, "empty parameter");
	goto __free_all;
__unexpected_char:
	sprintf(data->buf, "unexpected character `%c`", **s);
	print_error(data->filename, data->lineno, data->line, data->buf);

__free_all:
	if (mnemonic) free(mnemonic);
	if (param1) free(param1);
	if (param2) free(param2);
	while (!is_endl(**s)) (*s)++;
	return (-1);
}


char	*parse_instruction(char *s, vector_t *area, vector_t *ext_symbol, loc_sym_t *loc_symbol, vector_t *macro, data_t *data)
{
	char		*mnemonic = NULL;
	char		*param1 = NULL;
	char		*param2 = NULL;
	uint8_t		n_params;
	char		*replacement;

	// replace line if it's a macro w/wo params
	if (replace_macro(&s, macro, &replacement, data) == -1);	// return NULL if no macro
		return (s);

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
		char	*p = replacement;
		if (set_mnemonic_and_params(&p, &mnemonic, &param1, &param2, &n_params, data) == -1)
			return (s);
	}
	else {
		if (set_mnemonic_and_params(&s, &mnemonic, &param1, &param2, &n_params, data) == -1);
			return (s);
	}

	if (n_params == 3)
		goto __too_many_paramters;

	// if it's a label, add it and return
	if (n_param == IS_LABEL)
	{
		add_label(mnemonic, area, ext_symbol, loc_symbol, data);
		return (s);
	}

	// verify if all params are macro without params, and replace them
	replace_macro_without_param(param, n_params, data);	//param[x].str = macro.content_addr;

	/* parse params
	** example:
	**   "ld A, 10 + 5"
	** -->
	**   param[0] = A
	**   param[1] = IMM8
	**   val = {0, 15, 0} ?
	*/
	char		*mne = strndup(mnemonic.str, mnemonic.len);
	param_t		param[2] = {NONE, NONE};	//enum ?
	value_t		val = {0, 0, 0};
//======= char *mnemonic, char *param1, char *param2
	str_to_lower(mnemonic);
	if (n_params == 2)
		instruction_replace(&mnemonic, &param1, &param2);	// "ld (ff00h)" -> ldff, "ld HL, SP" -> ldhl
	
	if (param1)
	{
		if (calcul_param(param1, &val) == -1)
			goto __error_calcul;
		param[0] = get_param_type(param1, &val);

		if (param2)
		{
			value_t	tmp_val = {0, 0, 0};
			if (calcul_param(param2, &tmp_val) == -1)
				goto __error_calcul;
			param[1] = get_param_type(param2, &tmp_val);

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
	uint8_t			bin[4] = {0, 0, 0, val.sign};

	error = get_bin_instruction(inst, param, &val, bin);
	if (error.p1 || error.p2)
	{
		const char *errtable[OVERFLOW + 1] = {
			"Missing parameter for instruction\n",
			"Too many parameters for instruction\n",
			"Invalid destination for instruction\n",
			"Invalid source for instruction\n",
			"Address overflow\n"
		};
		
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

	return (s);

	/* ERRORS */
__free_and_ret:
	if (mnemonic)
		free(mnemonic);
	return (s);
}

char	*parse_instruction(char *s, vector_t *area, vector_t *ext_symbol, loc_sym_t *loc_symbol, vector_t *macro, data_t *data)
{
	char		*name = s;
	uint32_t	macro_index = 0xffffffffu;
	char		*macro_param[10] = {NULL};
	uint8_t		n_params = 0;
	char		*content;
	uint32_t	argc;
	char		*search = NULL;

	if (!is_alpha(*s) && *s != '_')
	{
		fprintf(stderr, "(#0) ");
		goto __unexpected_char;
	}
	s++;
	while (is_alnum(*s) || *s == '_') s++;

	// LABEL
	if (*s == ':')
	{
		// start = name
		char *end = s;
		s++;
		if (!is_space(*s) && !is_endl(*s))
		{
			fprintf(stderr, "(#1) ");
			goto __unexpected_char;
		}
		add_label(strndup(name, end - name), area, ext_symbol, loc_symbol, data); //test if not mnemonic
		return (s);
	}
	else if (!is_space(*s) && *s != '(' && !is_endl(*s))
	{
		fprintf(stderr, "(#2) ");
		goto __unexpected_char;
	}
	/*
	if (!is_endl(*s))
		goto __add_instruction;
*/
	search = strndup(name, s - name);
	while (is_space(*s)) s++;
	if ((macro_index = (uint32_t)vector_search(macro, (void*)&search)) != 0xffffffffu)
	{
		char		*content_ptr = NULL;
		data_t		new_data;
		content = VEC_ELEM(macro_t, macro, macro_index)->content;
		argc = VEC_ELEM(macro_t, macro, macro_index)->argc;

		if (*s == '(')
		{
			s++;
			n_params = get_params(&s, macro_param);
			if (n_params == 0xffu)
			{
				fprintf(stderr, "(#3) ");
				goto __unexpected_char;
			}
			if (n_params != argc)
				goto __argc_error;
			if (n_params)
			{
				content = replace_content(content, argc, macro_param);
				content_ptr = content;
			}
			for (uint8_t i = 0; macro_param[i]; i++)
				free(macro_param[i]);
		}
		char	*tmp = content;
		while (!is_space(*tmp) && !is_endl(*tmp)) tmp++;
		if (!is_endl(*tmp))
		{
			while (is_space(*tmp)) tmp++;
			if (!is_endl(*tmp))
				goto __next;
		}
		
		name = (argc) ? content : strdup(content);
		s = add_instruction(name, area, ext_symbol, loc_symbol, macro, s, data);
		free(name);
		return (s);

		
__next:
		new_data.filename = malloc(strlen(data->filename) + strlen(name) + 10);
		sprintf(new_data.filename, "%s in macro %s", data->filename, name);
		new_data.line = content;
		new_data.lineno = 1;
		new_data.cur_area = data->cur_area;


		for (;;)
		{
			char *inst = content;
			while (!is_space(*content) && !is_endl(*content)) content++;
			inst = strndup(inst, content - inst);
			content = add_instruction(inst, area, ext_symbol, loc_symbol, NULL, content, &new_data);
			free(inst);
			if (*content == '\0')
				break;
			content += 1;
			new_data.line = content;
			new_data.lineno++;
		}
		if (content_ptr) free(content_ptr);
		free(new_data.filename);
		free(search);
		return (s);
	}
	else if (*s == '(' && !is_space(s[-1]))
	{
		fprintf(stderr, "(#5) ");
		goto __unexpected_char;
	}

//__add_instruction:
	s = add_instruction(search, area, ext_symbol, loc_symbol, macro, s, data);
	//free(search);
	return (s);

__argc_error:
	for (uint8_t i = 0; macro_param[i]; i++)
		free(macro_param[i]);
	sprintf(data->buf, "arguments expected: %hhu, %u given", (uint8_t)argc, n_params);
	goto __print_error;
	

__unexpected_char:
	sprintf(data->buf, "(#1) unexpected character `%c`", *s);
	if (search) free(search);
__print_error:
	print_error(data->filename, data->lineno, data->line, data->buf);
	while (!is_endl(*s)) s++;
	return (s);
}
