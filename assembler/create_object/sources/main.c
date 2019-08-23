/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   main.c                                           .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/11 10:36:42 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/23 18:47:28 by fcordon     ###    #+. /#+    ###.fr     */
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

#define	DIR_SEPARATOR	'/'
#define	SKIP_SPACES(ptr)		{while (is_space(*(ptr))) (ptr)++;}

uint32_t		g_error;
uint32_t		g_warning;
cart_info_t		cartridge_info;
char			*includes_list[128] = {NULL};
char			*base;
uint32_t		base_length;

void		print_directive_arg_error(char *keyword, data_t *data)
{
	sprintf(data->buf, "`%s` keyword expects arguments", keyword);
	free(keyword);
	print_error(data->filename, data->lineno, data->line, data->buf);
	fprintf(stderr, "\e[1;31m%u errors\e[0m\n", g_error);
}


static char	*get_file_path(const char *start)
{
	const char *end = start + strlen(start + 1);

	while (end >= start && *end != DIR_SEPARATOR) end++;
	if (end < start)
		return (NULL);
	base_length = (end - start) + 1;
	return (strndup(start, base_length));
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

#undef	ADD_MACRO

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

	// lexe/parse source file (parse.c)
	if (parse_file(argv[1], code_area, macro, extern_symbol, &local_symbol, 0) == -1)
	{
		fprintf(stderr, "cannot open %s\n", argv[1]);
		return (1);
	}

	// check readed data (check_readed_data.c)
	check_undefined_symbols(local_symbol.label);
	check_code_area_overflow(code_area);

	// ----------------- (replace_internal_labels.c)
	replace_internal_labels(code_area, &local_symbol);
	if (g_error)
	{
		fprintf(stderr, "\e[1;31m%u errors\e[0m\n", g_error);
		goto __free_all;
	}

	// ----------------- (create_object_file.c)
	create_object_file(code_area, &local_symbol, extern_symbol, argv[0]);

__free_all:
	vector_destroy(code_area);
	vector_destroy(macro);
	vector_destroy(local_symbol.label);
	vector_destroy(local_symbol.memblock);
	return (g_error);
}
