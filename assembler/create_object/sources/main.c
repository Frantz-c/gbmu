/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   main.c                                           .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/11 10:36:42 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/13 15:16:48 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "tools.h"
#include "macro.h"
#include "keywords.h"
#include "callback.h"
#include "error.h"
#include "struct.h"
#include "instruction_or_label.h"
#include "create_object_file.h"

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
cart_info_t		cartridge_info;
char			*includes_list[128] = {NULL};
char			*base;

void	check_undefined_symbols(vector_t *label)
{
	register label_t	*l = VEC_ELEM_FIRST(label_t, label);

	for (uint32_t i = 0; i < label->nitems; i++, l++)
	{
		if (l->base_or_status == 0xffffffffu)
		{
			g_error++;
			fprintf(stderr, "\e[1;31mundefined symbol \e[0m\"%s\"\n", l->name);
		}
	}
}

void	check_code_area_overflow(vector_t *area)
{
	register code_area_t	*a;
	register uint32_t	end;
	
	if (area->nitems == 0)
		return;
	a = (code_area_t *)area->data;
	end = a->addr + a->size;
	a++;

	for (uint32_t i = 1; i < area->nitems; i++, a++)
	{
		if (end >= a->addr)
		{
			g_error++;
			fprintf(stderr, "overlap (end = 0x%x, start = 0x%x)\n", end, a->addr);
		}
	}
}

/*
 *	replace labels only
 */
void		replace_internal_symbols(vector_t *area, loc_sym_t *local_symbol)
{
	register code_area_t	*a;

	a = (code_area_t *)area->data;
	for (uint32_t i = 0; i < area->nitems; i++, a++)
	{
		register code_t	*c = a->data;
		for (; c; c = c->next)
		{
			if (c->symbol && (c->size & 0xffffff00u) == 0)
			{
				// remplacement + calcul
				register ssize_t	index;

				if ((index = vector_search(local_symbol->label, (void*)&c->symbol)) != -1)
				{
					register label_t	*lab = VEC_ELEM(label_t, local_symbol->label, index);


					if (*c->opcode == JR || *c->opcode == JRZ || *c->opcode == JRNZ
						|| *c->opcode == JRC || *c->opcode == JRNC)
					{
						register int32_t	val = c->opcode[1] | (c->opcode[2] << 8);
						register int32_t	lab_addr = lab->base_or_status;

						lab_addr -= (int32_t)c->addr;
						val = (c->opcode[3] == '-') ? lab_addr - val : lab_addr + val;
						val -= 2;
						if (val > 0x7f || val < -128)
						{
							g_error++;
							fprintf(stderr, "too big jump (%d)\n", val);
						}
						c->opcode[1] = (uint8_t)val;
						c->opcode[2] = 0;
					}
					else
					{
						register uint32_t	lab_addr = lab->base_or_status;
						register uint32_t	val = c->opcode[1] | (c->opcode[2] << 8);

						if (lab_addr >= 0x8000)
							lab_addr = (lab_addr % 0x4000) + 0x4000;
						val = (c->opcode[3] == '-') ? lab_addr - val: lab_addr + val;
						c->opcode[1] = (uint8_t)val;
						c->opcode[2] = (val >> 8);
						if (val & 0xffff0000u)
						{
							g_error++;
							fprintf(stderr, "overflow label (0x%x)\n", val);
						}
					}
					free(c->symbol);
					c->symbol = NULL;
				}
			}
		}
	}
}

void		print_directive_arg_error(char *keyword, data_t *data)
{
	sprintf(data->buf, "`%s` keyword expects arguments", keyword);
	free(keyword);
	print_error(data->filename, data->lineno, data->line, data->buf);
	fprintf(stderr, "\e[1;31m%u errors\e[0m\n", g_error);
}

// prendre en charge les doubles quotes
// ajouter le chemin du fichier passé en argument
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
	sprintf(data->buf, "(#0) unexpected character `%c`", **s);
	print_error(data->filename, data->lineno, data->line, data->buf);
	fprintf(stderr, "\e[1;31m%u errors\e[0m\n", g_error);
	return (NULL);
}

STATIC_DEBUG void			area_print(const void *a)
{
	code_area_t	*area = (code_area_t *)a;

	printf("addr = 0x%04x\n", area->addr);
	if (area->data)
	{
		for (code_t *c = area->data; c; c = c->next)
		{
			if (c->size & 0xffffff00u)
			{
				uint8_t		*bytes = (uint8_t*)c->symbol;
				uint32_t	size = (c->size & 0xffffff00u) >> 8;
				for (uint8_t i = 0; i < size; i++)
				{
					if (i % 8 == 7)
						puts("");
					if (i == 0)
						printf("\tbytes[%u] : 0x%hhx", size, bytes[i]);
					else
						printf(", 0x%hhx", bytes[i]);
				}
			}
			else
			{
				uint8_t	size = c->size;
				printf("\t");
				for (uint8_t i = 0; i < size; i++)
				{
					printf("0x%hhX, ", c->opcode[i]);
				}
			}
			printf("\n");
		}
	}
	else
		printf("\tno data\n");
}



#define	DIR_SEPARATOR	'/'
char	*get_file_path(const char *start)
{
	const char *end = start + strlen(start + 1);

	while (end >= start && *end != DIR_SEPARATOR) end++;
	if (end < start)
		return (NULL);
	return (strndup(start, (end - start) + 1));
}


#define	ADD_MACRO(_name, _content, _macro)		\
	new.name = _name;\
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

void __attribute__((always_inline))
	set_vectors(&macro, &code_area, &local_symbol, &extern_symbol)
{
	*macro = set_builtin_macro();
	*code_area = vector_init(sizeof(code_area_t));
	(*code_area)->compar = &area_cmp;
	(*code_area)->destroy = &area_destroy;
	(*code_area)->search = &area_match;
	local_symbol->label = vector_init(sizeof(label_t));
	local_symbol->label->compar = &label_cmp;
	local_symbol->label->search = &label_match;
	local_symbol->label->destroy = &label_destroy;
	local_symbol->memblock = vector_init(sizeof(memblock_t));
	local_symbol->memblock->destroy = &memblock_destroy;
	local_symbol->memblock->compar = &memblock_cmp;
	local_symbol->memblock->search = &memblock_match;
	*extern_symbol = vector_init(sizeof(symbol_t));
	(*extern_symbol)->destroy = &ext_symbol_destroy;
	(*extern_symbol)->compar = &ext_symbol_cmp;
	(*extern_symbol)->search = &ext_symbol_match;

	// by default, code is writed in address 0x0000
	code_area_t		area_elem = {0, 0, NULL, NULL};
	vector_push(*code_area, (void*)&area_elem);
}

/*
**	argv = [obj, src, NULL];
*/
int		main(int argc, char *argv[])
{
	vector_t	*macro;
	vector_t	*code_area;
	vector_t	*extern_symbol;
	loc_sym_t	local_symbol;

	if (argc != 2) return (1);

	set_vectors(&macro, &code_area, &local_symbol, &extern_symbol);
	base = get_file_path(argv[1]);
	g_error = 0;
	g_warning = 0;

	// lexe/parse .gbs file
	parse_file(argv[1], code_area, macro, extern_symbol, &local_symbol, 0);

	// check loaded data
	check_undefined_symbols(local_symbol.label);
	check_code_area_overflow(code_area);
	replace_internal_symbols(code_area, &local_symbol);
	if (g_error)
	{
		fprintf(stderr, "\e[1;31m%u errors\e[0m\n", g_error);
		goto __free_all;
	}

	// generate .gbo file
	create_object_file(code_area, &local_symbol, extern_symbol, argv[0]);

__free_all:
	vector_destroy(code_area);
	vector_destroy(macro);
	vector_destroy(local_symbol.label);
	vector_destroy(local_symbol.memblock);
	return (g_error);
}