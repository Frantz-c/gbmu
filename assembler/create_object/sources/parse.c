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
\
	if (strncmp(s + 1, "bank", 4) == 0 && !is_alnum(s[5]))\
		s = bank_switch(area, s + 6, &data);\
	else if (strncmp(s + 1, "byte", 4) == 0 && !is_alnum(s[5]))\
		s = add_bytes(area, s + 6, &data);\
	else if (strncmp(s + 1, "memlock", 7) == 0 && !is_alnum(s[8]))\
		s = set_memlock_area(loc_symbol->memblock, s + 9, &data);\
	else if (strncmp(s + 1, "var", 3) == 0 && is_numeric(s + 4, &len) && is_space(s[len + 4]))\
		s = assign_var_to_memory(loc_symbol, ext_symbol, s + 4, &data);\
	else if (strncmp(s + 1, "extern", 6) == 0 && !is_alnum(s[7]))\
		s = set_extern_symbol(ext_symbol, s + 8, &data);\
	else if (strncmp(s + 1, "program_start", 13) == 0 && !is_alnum(s[14]))\
		s = set_program_start(s + 14);\
	else if (strncmp(s + 1, "game_title", 10) == 0 && !is_alnum(s[11]))\
		s = set_game_title(s + 11);\
	else if (strncmp(s + 1, "game_code", 9) == 0 && !is_alnum(s[10]))\
		s = set_game_code(s + 10);\
	else if (strncmp(s + 1, "cgb_support", 11) == 0 && !is_alnum(s[12]))\
		s = set_cgb_support(s + 12);\
	else if (strncmp(s + 1, "maker_code", 10) == 0 && !is_alnum(s[11]))\
		s = set_maker_code(s + 11);\
	else if (strncmp(s + 1, "sgb_support", 11) == 0 && !is_alnum(s[12]))\
		s = set_sgb_support(s + 12);\
	else if (strncmp(s + 1, "rom_size", 8) == 0 && !is_alnum(s[9]))\
		s = set_rom_size(s + 9);\
	else if (strncmp(s + 1, "ram_size", 8) == 0 && !is_alnum(s[9]))\
		s = set_ram_size(s + 9);\
	else if (strncmp(s + 1, "code_dest", 9) == 0 && !is_alnum(s[10]))\
		s = set_code_dest(s + 10);\
	else if (strncmp(s + 1, "version", 7) == 0 && !is_alnum(s[8]))\
		s = set_version(s + 8);\
	else\
	{\
		char *keyword = get_keyword(data.line);\
		sprintf(data.buf, "unknown keyword `%s`", keyword);\
		free(keyword);\
		print_error(data.filename, data.lineno, data.line, data.buf);\
		while (!is_endl(*s)) s++;\
	}\
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
			PREPROCESSOR_DIRECTIVES();
		else if (*s == '.')
			KEYWORDS();
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
