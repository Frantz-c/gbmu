/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   parse.c                                          .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/08/13 14:04:54 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/13 15:20:30 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

typedef struct	arguments_s
{
	int32_t	type;
	void	*value;
}
arguments_t;

static char	*get_keyword(char *s)
{
	char	*start = s;

	while (*s != '\0' && *s != ' ' && *s != '\t' && *s != '\n')
		s++;
	return (strndup(start, s - start));
}

STATIC_DEBUG void			symbol_print(const void *a)
{
	symbol_t	*s = (symbol_t *)a;

	printf("\t%s\n", s->name);
}

STATIC_DEBUG void			label_print(const void *a)
{
	label_t	*l = (label_t *)a;

	printf("\t%s\n", l->name);
}

STATIC_DEBUG void			macro_print(const void *a)
{
	register macro_t	*p = (macro_t *)a;

	printf("%%define %s(%u) %s\n", p->name, p->argc, p->content);
}

STATIC_DEBUG void			memblock_print(const void *a)
{
	memblock_t *b = (memblock_t *)a;
	vector_t	*v = ((memblock_t *)a)->var;

	printf("block %s, 0x%x - 0x%x\n", b->name, b->start, b->end);

	if (v)
	{
		for (uint32_t i = 0; i < v->nitems * sizeof(variable_t); i += sizeof(variable_t))
		{
			variable_t	*var = (variable_t *)(v->data + i);
			printf("\tvar %s, 0x%x, %u octet(s)\n", var->name, var->addr, var->size);
		}
	}
}

STATIC_DEBUG void			vector_print(vector_t *vec, char *name, void (*print)(const void *))
{
	printf("\n==> %s:\n", name);
	if (vec == NULL)
	{
		puts("NULL\n");
		return;
	}
	for (uint32_t i = 0; i < vec->nitems; i++)
	{
		print(vec->data + (i * vec->elemsize));
	}
	printf("\n");
}

#define	PREPROCESSOR_DIRECTIVES()	\
{\
	if (strncmp(s + 1, "define", 6) == 0 && !is_alnum(s[7])) {\
		CHECK_ERROR_DIRECTIVE(7);\
		s = define_macro(macro, s + 8, &data);\
	}\
	else if (strncmp(s + 1, "undef", 5) == 0 && !is_alnum(s[6])) {\
		CHECK_ERROR_DIRECTIVE(6);\
		s = undef_macro(macro, s + 7, &data);\
	}\
	else if (strncmp(s + 1, "include", 7) == 0 && !is_alnum(s[8]))\
	{\
		CHECK_ERROR_DIRECTIVE(8);\
		s += 9;\
		include_filename = get_include_filename(&s, &data);\
		parse_file(include_filename, area, macro, ext_symbol, loc_symbol, data.cur_area);\
		free(include_filename);\
	}\
	else if (strncmp(s + 1, "if", 3) == 0 && !is_alnum(s[4]))\
	{\
		CHECK_ERROR_DIRECTIVE(4);\
		fprintf(stderr, "Preprocessor conditions not implemented\n");\
		exit(1);\
	}\
	else\
	{\
		char *keyword = get_keyword(data.line);\
		sprintf(data.buf, "unknown directive `%s`", keyword);\
		free(keyword);\
		print_error(data.filename, data.lineno, data.line, data.buf);\
		while (!is_endl(*s)) s++;\
	}\
}


#define KEYWORDS()	\
{\
	uint32_t	len;\
	uint32_t	type;\
\
	if (strcmp(keyword, "bank") == 0)\
		s = bank_switch(area, &args, &data);\
	else if (strcmp(keyword, "byte") == 0)\
		s = add_bytes(area, &args, &data);\
	else if (strcmp(keyword, "memlock") == 0)\
		s = set_memlock_area(loc_symbol->memblock, &args, &data);\
	else if (strncmp(keyword, "var", 3) == 0 && (type = is_numeric(keyword + 3, &len)) && is_space(keyword[len + 3]))\
		s = assign_var_to_memory(loc_symbol, ext_symbol, atou_type(keyword + 3, NULL, type), &args, &data);\
	else if (strcmp(keyword, "extern") == 0)\
		s = set_extern_symbol(ext_symbol, &args, &data);\
	else if (strncmp(keyword, "program_start", 13) == 0)\
		s = set_program_start(&args, &data);\
	else if (strncmp(keyword, "game_title", 10) == 0)\
		s = set_game_title(&args, &data);\
	else if (strncmp(keyword, "game_code", 9) == 0)\
		s = set_game_code(&args, &data);\
	else if (strncmp(keyword, "cgb_support", 11) == 0)\
		s = set_cgb_support(&args, &data);\
	else if (strncmp(keyword, "maker_code", 10) == 0)\
		s = set_maker_code(&args, &data);\
	else if (strncmp(keyword, "sgb_support", 11) == 0)\
		s = set_sgb_support(&args, &data);\
	else if (strncmp(keyword, "cart_type", 11) == 0)\
		s = set_cartridge_type(&args, &data);\
	else if (strncmp(keyword, "rom_size", 8) == 0)\
		s = set_rom_size(&args, &data);\
	else if (strncmp(keyword, "ram_size", 8) == 0)\
		s = set_ram_size(&args, &data);\
	else if (strncmp(keyword, "destination", 11) == 0)\
		s = set_code_dest(&args, &data);\
	else if (strncmp(keyword, "version", 7) == 0)\
		s = set_version(&args, &data);\
	else\
	{\
		sprintf(data.buf, "unknown keyword `.%s`", keyword);\
		print_error(data.filename, data.lineno, data.line, data.buf);\
	}\
}


char	*get_keywords_and_arguments(char **s, args_t *arguments[4], data_t *data)
{
	char	*keyword = NULL;
	char	*arg_start;

	(*s)++;
	if (!is_alpha(**s))
		goto __unexpected_char;
	// keyword = keyword start
	keyword = (*s)++;

	while (is_alnum(**s)) (*s)++;
	keyword = strndup(keyword, *s - keyword);

	uint8_t i;
	for (i = 0; ; )
	{
		uint32_t	len;
		int32_t		type;
		uint8_t		minus = 0;

		while (is_space(**s)) (*s)++;
		if (**s == '-')
		{
			minus++;
			(*s)++;
		}

		if ((type = is_numeric(*s, &len)) != 0)
		{
			if (minus)
				goto __signed_not_expected;
			args[i].type = 'I';
			args[i].value = malloc(sizeof(uint32_t));
			*(uint8_t*)(args[i].value) = atou_type(*s, &len, type);
			*s += len;
		}
		else if (**s == '"')
		{
			arg_start = ++(*s);
			while (!is_endl(**s) && **s != '"')
			{
				if (**s < ' ' || **s > '_')
					goto __not_gameboy_ascii;
				(*s)++;
			}
			if (**s != '"')
				goto __unexpected_char;
			if (*s - arg_start > IDENTIFIER_MAX_LENGTH)
				goto __too_long_argument;
			args[i].type = '"';
			args[i].value = (void*)strndup(arg_start, *s - arg_start);
			(*s)++;
		}
		else
			goto __unexpected_char;

		while (is_space(**s)) (*s)++;
		if (is_endl(**s))
			break;
		if (++i == 4)
			goto __too_many_arguments;
		if (**s != ',')
			goto __unexpected_char;
		(*s)++;
	}
	arguments[i] = NULL;
	return (keyword);

__too_many_arguments:
	sprintf(data->buf, "too many arguments");
	goto __print_error_and_free_all;
__signed_not_expected:
	sprintf(data->buf, "signed numbers forbidden in keywords arguments");
	goto __print_error_and_free_all;
__not_gameboy_ascii:
	sprintf(data->buf, "not gameboy ascii character `%c`", **s);
	goto __print_error_and_free_all;
__too_long_argument:
	sprintf(data->buf, "too long argument (argument %u)", i + 1);
	goto __print_error_and_free_all;
__unexpected_char:
	sprintf(data->buf, "unexpected character `%c`", **s);

__print_error_and_free_all:
	if (keyword)
		free(keyword);
	for (uint8_t i = 0; args[i].value; i++)
		free(args[i].value);
	print_error(data->filename, data->lineno, data->line, data->buf);
	while (!is_endl(**s)) (*s)++;
	return (NULL);
}

static void		parse_file(char *filename, vector_t *area, vector_t *macro, vector_t *ext_symbol, loc_sym_t *loc_symbol, uint32_t cur_area)
{
	data_t		data;
	char		*s, *content_start;
	char		*include_filename;

	if ((s = get_file_contents(filename, &data.length)) == NULL)
	{
		fprintf(stderr, "%s: file doesn't exist\n", filename);
		exit(1);
	}
	content_start = s;
	s[data.length] = '\0';

	data.filename = filename;
	data.line = s;
	data.lineno = 1;
	data.cur_area = cur_area;

	while (*s)
	{
		SKIP_SPACES(s);


		if (*s == '\0')
			break;

		if (*s == '\n') {
			data.lineno++;
			data.line = ++s;
			continue;
		}


		if (*s == '%')
		{
			PREPROCESSOR_DIRECTIVES();
		}
		else if (*s == '.')
		{
			arguments_t	args[4] = {{0, NULL}};
			char	*keyword = get_keywords_and_arguments(&s, &args, &data);

			KEYWORDS();
		}
		else
			s = parse_instruction(s, area, ext_symbol, loc_symbol, macro, &data);
	}
	
	puts("");
	vector_print(macro, "macro", &macro_print);
	vector_print(area, "code area", &area_print);
	vector_print(ext_symbol, "extern symbol", &symbol_print);
	vector_print(loc_symbol->label, "label", &label_print);
	vector_print(loc_symbol->memblock, "block", &memblock_print);
	free(content_start);
}
