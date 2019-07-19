/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_main.c                                     .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/11 10:36:42 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/19 15:05:57 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "gbasm_tools.h"
#include "gbasm_macro.h"
#include "gbasm_keywords.h"
#include "gbasm_callback.h"
#include "gbasm_error.h"
#include "gbasm_struct.h"
#include "gbasm_instruction_or_label.h"

#define BEG						do{
#define END						}while(0)
#define	SKIP_SPACES(ptr)		BEG while (is_space(*(ptr))) (ptr)++; END

#define CHECK_ERROR_DIRECTIVE(n)	\
	if (!is_space(s[n]))\
	{\
		s += n;\
		while (!is_endl(*s)) s++;\
		print_directive_arg_error(get_keyword(data.line), &data);\
		continue;\
	}

#define CHECK_ERROR_KEYWORD(n)	\
	if (!is_space(s[n]))\
	{\
		s += n;\
		while (!is_endl(*s)) s++;\
		print_keyword_arg_error(get_keyword(data.line), &data);\
		continue;\
	}


uint32_t		g_error;
uint32_t		g_warning;


void		print_directive_arg_error(char *keyword, data_t *data)
{
	sprintf(data->buf, "`%s` keyword expects arguments", keyword);
	free(keyword);
	print_error(data->filename, data->lineno, data->line, data->buf);
	fprintf(stderr, "\e[1;31m%u errors\e[0m\n", g_error);
}

static char	*get_include_filename(char **s, data_t *data)
{
	char	*name;

	while (is_space(**s)) (*s)++;
	name = *s;
	while (!is_space(**s) && !is_endl(**s)) (*s)++;
	name = strndup(name, *s - name);
	while (is_space(**s)) (*s)++;
	if (!is_endl(**s))
		goto __unexpected_char;
	return (name);

__unexpected_char:
	sprintf(data->buf, "unexpected character `%c`", **s);
	print_error(data->filename, data->lineno, data->line, data->buf);
	fprintf(stderr, "\e[1;31m%u errors\e[0m\n", g_error);
	return (NULL);
}

void			area_print(const void *a)
{
	code_area_t	*area = (code_area_t *)a;

	printf("addr = 0x%04x\n", area->addr);
	for (code_t *c = area->data; c; c = c->next)
	{
		if (c->size > 3)
		{
			uint8_t	*bytes = (uint8_t*)c->unkwn;
			for (uint8_t i = 0; i < c->size - 3; i++)
			{
				if (i % 8 == 7)
					puts("");
				if (i == 0)
					printf("\tbytes[%hhu] : 0x%hhx", (uint8_t)(c->size - 3), bytes[i]);
				else
					printf(", 0x%hhx", bytes[i]);
			}
		}
		else
		{
			if (c->opcode[0] == 0xcb)
				printf("\t\"0x%hhX 0x%hhX\", ", c->opcode[0], c->opcode[1]);
			else
				printf("\t\"0x%hhX\", ", c->opcode[0]);

			if ((c->ope_size & 0x0f) == 0)
				printf("NO_OPERANDS");
			else
			{
				if ((c->ope_size & 0x0f) == 1)
					printf("0x%hhX", c->operand1[0]);
				else
					printf("0x%hhX 0x%hhX", c->operand1[0], c->operand1[0]);
				if (((c->ope_size & 0xf0) >> 4) > 0)
				{
					if (((c->ope_size & 0xf0) >> 4) == 1)
						printf(", 0x%hhX", c->operand2[0]);
					else
						printf(", 0x%hhX 0x%hhX", c->operand2[0], c->operand2[0]);
				}
			}
		}
		printf("\n");
		return;
	}
	printf("\tno data\n");
}

void			symbol_print(const void *a)
{
	symbol_t	*s = (symbol_t *)a;

	printf("\t%s\n", s->name);
}

void			label_print(const void *a)
{
	label_t	*l = (label_t *)a;

	printf("\t%s\n", l->name);
}

void			macro_print(const void *a)
{
	register macro_t	*p = (macro_t *)a;

	printf("%%define %s(%u) %s\n", p->name, p->argc, p->content);
}

void			memblock_print(const void *a)
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


void			vector_print(vector_t *vec, char *name, void (*print)(const void *))
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

static char	*get_keyword(char *s)
{
	char	*start = s;

	while (*s != '\0' && *s != ' ' && *s != '\t' && *s != '\n')
		s++;
	return (strndup(start, s - start));
}

uint8_t		get_params(char **s, char *param[10])
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

char	*replace_content(char *content, uint32_t argc, char *param[10])
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

char	*parse_instruction(char *s, vector_t *area, vector_t *label, vector_t *symbol, vector_t *macro, data_t *data)
{
	char		*name = s;
	uint32_t	macro_index = 0xffffffffu;
	char		*macro_param[10] = {NULL};
	uint8_t		n_params = 0;
	char		*content;
	uint32_t	argc;


	if (!is_alpha(*s) && *s != '_')
		goto __unexpected_char;
	s++;
	while (is_alnum(*s) || *s == '_') s++;

	// LABEL
	if (*s == ':')
	{
		char *end = s;
		s++;
		if (!is_space(*s) && !is_endl(*s))
			goto __unexpected_char;
		add_label(strndup(name, end - name), area, label, data); //test if not mnemonic
		return (s);
	}
	else if (!is_space(*s) && *s != '(' && !is_endl(*s))
		goto __unexpected_char;
	if (!is_endl(*s))
		goto __add_instruction;

	while (is_space(*s)) s++;
	if ((macro_index = (uint32_t)vector_search(macro, (void*)&name)) != 0xffffffffu)
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
				goto __unexpected_char;
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
			content = add_instruction(inst, area, label, NULL, content, &new_data);
			free(inst);
			if (*content == '\0')
				break;
			content += 1;
			new_data.line = content;
			new_data.lineno++;
		}
		if (content_ptr) free(content_ptr);
		free(new_data.filename);
		return (s);
	}
	else if (*s == '(' && !is_space(s[-1]))
		goto __unexpected_char;

__add_instruction:
	name = strndup(name, s - name);
	s = add_instruction(name, area, label, macro, s + 1, data);
	free(name);
	return (s);

__argc_error:
	for (uint8_t i = 0; macro_param[i]; i++)
		free(macro_param[i]);
	sprintf(data->buf, "arguments expected: %hhu, %u given", (uint8_t)argc, n_params);
	goto __print_error;
	

__unexpected_char:
	sprintf(data->buf, "unexpected character `%c`", *s);
__print_error:
	print_error(data->filename, data->lineno, data->line, data->buf);
	while (!is_endl(*s)) s++;
	return (s);
}

static void		parse_file(char *filename, vector_t *area, vector_t *macro, vector_t *label, vector_t *symbol, vector_t *memblock, int32_t *cur_area)
{
	data_t		data;
	char		*s;
	char		*include_filename;

	if ((s = get_file_contents(filename, &data.length)) == NULL)
	{
		fprintf(stderr, "%s: file doesn't exist\n", filename);
		exit(1);
	}
	s[data.length] = '\0';

	data.filename = filename;
	data.line = s;
	data.lineno = 1;
	while (*s)
	{
		SKIP_SPACES(s);

		if (*s == '\n')
		{
			data.lineno++;
			data.line = ++s;
			continue;
		}
		if (*s == '%')
		{
			if (strncmp(s + 1, "define", 6) == 0 && !is_alnum(s[7])) {
				CHECK_ERROR_DIRECTIVE(7);
				s = define_macro(macro, s + 8, &data);
			}
			else if (strncmp(s + 1, "undef", 5) == 0 && !is_alnum(s[6])) {
				CHECK_ERROR_DIRECTIVE(6);
				s = undef_macro(macro, s + 7, &data);
			}
			else if (strncmp(s + 1, "include", 7) == 0 && !is_alnum(s[8]))
			{
				CHECK_ERROR_DIRECTIVE(8);
				s += 9;
				include_filename = get_include_filename(&s, &data);
				parse_file(include_filename, area, macro, label, symbol, memblock, cur_area);
				free(include_filename);
			}
			else
			{
				char *keyword = get_keyword(data.line);
				sprintf(data.buf, "unknown directive `%s`", keyword);
				free(keyword);
				print_error(data.filename, data.lineno, data.line, data.buf);
				while (!is_endl(*s)) s++;
			}
		}
		else if (*s == '.')
		{
			uint32_t	len;

			if (strncmp(s + 1, "bank", 4) == 0 && !is_alnum(s[5]))
				s = bank_switch(area, s + 6, &data);
			else if (strncmp(s + 1, "byte", 4) == 0 && !is_alnum(s[5]))
				s = add_bytes(area, s + 6, &data);
			else if (strncmp(s + 1, "memlock", 7) == 0 && !is_alnum(s[8]))
				s = set_memlock_area(memblock, s + 9, &data);
			else if (strncmp(s + 1, "var", 3) == 0 && is_numeric(s + 4, &len) && is_space(s[len]))
				s = assign_var_to_memory(memblock, s + 4, &data);
			else if (strncmp(s + 1, "extern", 6) == 0 && !is_alnum(s[7]))
				s = set_extern_symbol(symbol, s + 8, &data);
			else
			{
				char *keyword = get_keyword(data.line);
				sprintf(data.buf, "unknown keyword `%s`", keyword);
				free(keyword);
				print_error(data.filename, data.lineno, data.line, data.buf);
				while (!is_endl(*s)) s++;
			}
		}
		else
		{
			s = parse_instruction(s, area, label, symbol, macro, &data);
		}
	}
	
	puts("");
	vector_print(macro, "macro", &macro_print);
	vector_print(area, "code area", &area_print);
	vector_print(symbol, "extern symbol", &symbol_print);
	vector_print(label, "label", &label_print);
	vector_print(memblock, "block", &memblock_print);
}

/*
	1ere etape:

	"$"					-> bytes
	"[a-zA-Z0-9_]+"		-> instruction
*/


// --cartridge_spec		mbc, n_ram_banks, n_rom_banks
// --cartridge_name		name
// --japanese_version
// --gameboy_type		{dmg, cdg, both}
// --show-memlock		-> affiche l'espace disponible dans chaque zone de variables
// --show-remaind-mem	-> affiche l'espace disponible dans chaque banque de la ROM

#define	ADD_MACRO(_name, _content, _macro)		new.name = _name;\
											new.content = _content;\
											vector_push(_macro, &new);

vector_t	*set_builtin_macro(void)
{
	vector_t		*macro;
	macro_t			new = {NULL, NULL, 0, 0};

	macro = vector_init(sizeof(macro_t));
	macro->compar = &macro_cmp;
	macro->search = &macro_match;
	macro->destroy = &macro_destroy;
	ADD_MACRO("P1", "0x00", macro);
	ADD_MACRO("SB", "0x01", macro);
	ADD_MACRO("SC", "0x02", macro);
	ADD_MACRO("DIV", "0x04", macro);
	ADD_MACRO("TIMA", "0x05", macro);
	ADD_MACRO("TMA", "0x06", macro);
	ADD_MACRO("TAC", "0x07", macro);
	ADD_MACRO("LCDC", "0x40", macro);
	ADD_MACRO("STAT", "0x41", macro);
	ADD_MACRO("SCY", "0x42", macro);
	ADD_MACRO("SCX", "0x43", macro);
	ADD_MACRO("LY", "0x44", macro);
	ADD_MACRO("LYC", "0x45", macro);
	ADD_MACRO("DMA", "0x46", macro);
	ADD_MACRO("BGP", "0x47", macro);
	ADD_MACRO("OBP0", "0x48", macro);
	ADD_MACRO("OBP1", "0x49", macro);
	ADD_MACRO("WY", "0x4A", macro);
	ADD_MACRO("WX", "0x4B", macro);
	ADD_MACRO("KEY1", "0x4D", macro);
	ADD_MACRO("VBK", "0x4F", macro);
	ADD_MACRO("HDMA1", "0x51", macro);
	ADD_MACRO("HDMA2", "0x52", macro);
	ADD_MACRO("HDMA3", "0x53", macro);
	ADD_MACRO("HDMA4", "0x54", macro);
	ADD_MACRO("HDMA5", "0x55", macro);
	ADD_MACRO("RP", "0x56", macro);
	ADD_MACRO("BCPS", "0x68", macro);
	ADD_MACRO("BCPD", "0x69", macro);
	ADD_MACRO("OCPS", "0x6A", macro);
	ADD_MACRO("OCPD", "0x6B", macro);
	ADD_MACRO("SVBK", "0x70", macro);
	ADD_MACRO("IF", "0x0F", macro);
	ADD_MACRO("IE", "0xFF", macro);
	vector_sort(macro);

	return (macro);
}

int		main(int argc, char *argv[])
{
	char			*file[3];
	vector_t		*macro = NULL;
	vector_t		*code_area = NULL;
	vector_t		*symbol = NULL;
	vector_t		*label = NULL;
	vector_t		*memblock = NULL;
	int32_t			cur_area;

	file[0]	=	"testfile1";
	file[1]	=	"testfile2";
	file[2]	=	NULL;

	macro = set_builtin_macro(); //macro + cmp()
	code_area = vector_init(sizeof(code_area_t));
	code_area->compar = &area_cmp;
	code_area->destroy = &area_destroy;
	code_area->search = &area_match;
	label = vector_init(sizeof(label_t));
	label->compar = &label_cmp;
	label->search = &label_match;
	label->destroy = &label_destroy;
	memblock = vector_init(sizeof(memblock_t));
	memblock->destroy = &memblock_destroy;
	memblock->compar = &memblock_cmp;
	memblock->search = &memblock_match;
	symbol = vector_init(sizeof(symbol_t));
	symbol->destroy = &symbol_destroy;
	symbol->compar = NULL;//&symbol_cmp;
	symbol->search = NULL;//&symbol_match;

	for (char **p = file; *p; p++)
	{
		g_error = 0;
		g_warning = 0;
		cur_area = 0;
		code_area_t		area_elem = {0, NULL, NULL};
		vector_push(code_area, (void*)&area_elem);

		parse_file(*p, code_area, macro, label, symbol, memblock, &cur_area);
		if (g_error)
		{
			fprintf(stderr, "\e[1;31m%u errors\e[0m\n", g_error);
			return (1);
		}

//		save_file_object(code_area, label, *p);
		vector_reset(code_area);
		vector_reset(label);
		vector_reset(memblock);
		vector_filter(macro, &macro_filter);
	}
	vector_destroy(code_area);
	vector_destroy(macro);
	vector_destroy(label);
	vector_destroy(memblock);
	return (0);
}
/*
 *	./prog -c file.gs -o file.go
 *	./prog -o bin.gb file.go
 *	./prog -o bin.gb file.gs   // supprimer les .o
 */
