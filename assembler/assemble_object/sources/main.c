/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_main.c                                     .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/11 10:36:42 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/11 12:36:41 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "assign_addr_to_var.h"
#include "get_symbols_and_cartridge_info.h"
#include "all_extern_symbols_exist.h"
#include "callback.h"
#include "get_code_with_replacement.h"
#include "write_binary.h"
#include "export_local_var_to_ext_symbols.h"

uint16_t	cart_info = 0;
cart_data_t	cartridge = {{0x0, 0xC3},{0},{0},{0},0,{0},0,0,0,0,0,0x33,0,0,{0}};
uint32_t	g_error = 0;
uint32_t	g_warning = 0;

const char	*get_type_str(uint32_t type)
{
	if (type == LABEL)
		return ("label");
	if (type == VAR)
		return ("variable");
	if (type == MEMBLOCK)
		return ("block");
	return ("unknown");
}


static void __attribute__((always_inline))	ext_init(vector_t **ext)
{
	(*ext) = vector_init(sizeof(ext_sym_t));

	(*ext)->destroy = &ext_destroy;
	(*ext)->compar = &loc_ext_compar;
	(*ext)->search = &loc_ext_search;
}

static void __attribute__((always_inline))	loc_init(loc_symbols_t *loc)
{
	loc->var = vector_init(sizeof(loc_var_t));
	loc->label = vector_init(sizeof(loc_label_t));
	loc->block = vector_init(sizeof(loc_block_t));

	loc->var->destroy = &loc_var_destroy;
	loc->var->compar = &loc_ext_compar;
	loc->var->search = &loc_ext_search;

	loc->label->destroy = &loc_destroy;
	loc->label->compar = &loc_ext_compar;
	loc->label->search = &loc_ext_search;

	loc->block->destroy = &loc_destroy;
	loc->block->compar = &loc_ext_compar;
	loc->block->search = &loc_ext_search;
}

static void __attribute__((always_inline))	code_init(vector_t **code)
{
	*code = vector_init(sizeof(all_code_t));
	(*code)->destroy = &code_destroy;
	(*code)->compar = &code_compar;
	(*code)->search = &code_search;
}
/*
	argv = {executable, object, ...}

	idee d'amelioration: separer les variables des blocks et des labels
*/
int main(int argc, char *argv[])
{
	if (argc < 2)
		exit(1);

	loc_symbols_t	loc;
	vector_t		*ext;
	vector_t		*code;
	char		*exe = argv[0];

	loc_init(&loc);
	ext_init(&ext);
	code_init(&code);


	argv++;
	for (uint32_t i = 0; argv[i]; i++)
		get_symbols_and_cartridge_info(argv[i], &loc, ext, i);

	all_extern_symbols_exist_and_write_label_addr_in_ext(&loc, ext, argv);
	if (g_error)
		exit(1);

	assign_addr_to_var(&loc, ext); // loc.var -> ext
	if (g_error)
		exit(2);
	export_local_var_to_ext_symbols(&loc, ext);

	// DEBUG
	puts("\e[1;34mintern labels:\e[0m");
	{
		register loc_label_t	*tmp = VEC_ELEM_FIRST(loc_label_t, loc.label);
		for (uint32_t i = 0; i < loc.label->nitems; i++, tmp++)
		{
			printf(
				"%s ==> addr = 0x%0.4x\n",
				tmp->name, tmp->value
			);
		}
	}
	puts("\e[1;34mintern variables:\e[0m");
	{
		register loc_var_t	*tmp = VEC_ELEM_FIRST(loc_var_t, loc.var);
		for (uint32_t i = 0; i < loc.var->nitems; i++, tmp++)
		{
			printf(
				"%s ==> addr = 0x%0.4x, size = %u, block = \"%s\"\n",
				tmp->name, tmp->addr, tmp->size, tmp->blockname
			);
		}
	}
	puts("\e[1;34mintern blocks:\e[0m");
	{
		register loc_block_t	*tmp = VEC_ELEM_FIRST(loc_block_t, loc.block);
		for (uint32_t i = 0; i < loc.block->nitems; i++, tmp++)
		{
			printf(
				"%s ==> start = 0x%0.4x, end = 0x%0.4x\n",
				tmp->name, tmp->start, tmp->end
			);
		}
	}

	puts("\e[1;36mextern symbols:\e[0m");
	{
		register ext_sym_t	*tmp = VEC_ELEM_FIRST(ext_sym_t, ext);
		for (uint32_t i = 0; i < ext->nitems; i++, tmp++)
		{
			printf(
				"%s (%s)\n",
				tmp->name, get_type_str(tmp->type)
			);
		}
	}
	// END DEBUG


	get_code_with_replacement(&loc, ext, code, argv);
	if (g_error)
		exit(3);

	write_binary(code, exe);

	return (0);
}
