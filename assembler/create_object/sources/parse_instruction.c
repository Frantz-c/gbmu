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
