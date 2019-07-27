/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_var_keyword.c                              .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/13 22:59:31 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/27 21:16:04 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "gbasm_tools.h"
#include "gbasm_struct_tools.h"
#include "gbasm_struct.h"
#include "gbasm_error.h"
#include "gbasm_callback.h"

enum symbol_name
{
	SYM_VAR, SYM_LAB, SYM_EXT, SYM_BLOCK
};

static int		duplicate_symbol(char *name, loc_sym_t *loc_symbol, vector_t *ext_symbol, data_t *data)
{
	int32_t						index;
	int32_t						block_index;
	static const char *const	err_msg = "duplicate symbol `%s` (previous declaration in file %s:%u)";

	if ((index = vector_search(loc_symbol->label, (void*)&name)) != -1)
	{
		label_t	*lab = VEC_ELEM(label_t, loc_symbol->label, (uint32_t)index);

		sprintf(data->buf, err_msg, name, lab->filename, lab->line);
		print_error(data->filename, data->lineno, data->line, data->buf);
		return (-1);	
	}
	if ((index = memblock_match_name(loc_symbol->memblock, name)) != -1)
	{
		memblock_t	*block = VEC_ELEM(memblock_t, loc_symbol->memblock, index);
		
		sprintf(data->buf, err_msg, name, block->filename, block->line);
		print_error(data->filename, data->lineno, data->line, data->buf);
		return (-1);	
	}
	if ((index = variables_match_name(loc_symbol->memblock, name, &block_index)) != -1)
	{
		memblock_t	*block = VEC_ELEM(memblock_t, loc_symbol->memblock, block_index);
		variable_t	*var = VEC_ELEM(variable_t, block->var, index);

		sprintf( data->buf, err_msg, name, var->filename, var->line);
		print_error(data->filename, data->lineno, data->line, data->buf);
		return (-1);	
	}
	if ((index = vector_search(ext_symbol, (void*)&name)) != -1)
	{
		symbol_t	*sym = VEC_ELEM(symbol_t, ext_symbol, index);

		sprintf(data->buf, err_msg, name, sym->filename, sym->line);
		print_error(data->filename, data->lineno, data->line, data->buf);
		return (-1);	
	}
	return (0);
}

static uint32_t	get_block_addr(char *block, vector_t *memblock, uint32_t size, uint32_t *index)
{
	uint32_t	var_addr;

	for (uint32_t i = 0; i < memblock->nitems; i++)
	{
		memblock_t	*b = (memblock_t *)(memblock->data + (i * sizeof(memblock_t)));
		if (strcmp(b->name, block) == 0)
		{
			var_addr = b->end - b->space;
			b->space -= size;
			if (b->space >= 0xffffu)
				return (0xfffffffeu);
			*index = i;
			return (var_addr);
		}
	}
	return (0xffffffffu);
}

extern char	*assign_var_to_memory(loc_sym_t *loc_symbol, vector_t *ext_symbol, char *s, data_t *data)
{
	uint32_t	size;
	uint32_t	addr;
	int			error;
	char		*name;
	char		*blockname;

	size = atou_inc_all(&s, &error);
	if (error)
		goto __error;

	while (is_space(*s)) s++;
	if (*s == '\n' || *s == '\0')
		goto __error;

	if (!is_alpha(*s) && *s != '_')
		goto __error;
	name = s;
	while (is_alnum(*s) || *s == '_') s++;
	name = strndup(name, s - name);
	if (duplicate_symbol(name, loc_symbol, ext_symbol, data))
	{
		free(name);
		goto __skip_line_and_ret;
	}

	while (is_space(*s)) s++;
	if (*s == ',') {
		s++;
		while (is_space(*s)) s++;
	}

	if (!is_alpha(*s) && *s != '_')
	{
		free(name);
		goto __error;
	}
	blockname = s;
	while (is_alnum(*s) || *s == '_') s++;
	blockname = strndup(blockname, s - blockname);

	while (is_space(*s)) s++;
	if (*s != '\n' && *s != '\0')
	{
		free(blockname);
		free(name);
		goto __error;
	}

	uint32_t	index = 0;
	addr = get_block_addr(blockname, loc_symbol->memblock, size, &index);
	if (addr == 0xffffffffu)
	{
		// verifier si le symbol est extern.
		// si oui, conserver la valeur actuelle d'addr
		goto __unknown_memblock;
	}
	if (addr == 0xfffffffeu)
		goto __no_space;

	memblock_t	*block = VEC_ELEM(memblock_t, loc_symbol->memblock, index);
	variable_t	new = {name, addr, size, data->lineno, strdup(data->filename)};

	if (block->var == NULL)
	{
		block->var = vector_init(sizeof(variable_t));
		block->var->destroy = &variable_destroy;
		block->var->compar = &variable_cmp;
		block->var->search = &variable_match;
		vector_push(block->var, (void*)&new);
	}
	else
	{
		register size_t		i = vector_index(block->var, (void*)&name);

		vector_insert(block->var, (void*)&new, i);
	}
	free(blockname);
	return (s);

__no_space:
	sprintf(data->buf, "to few space in `%s`", blockname);
	goto __free_before_print_error;

__unknown_memblock:
	sprintf(data->buf, "unknown memblock `%s`", blockname);
__free_before_print_error:
	free(blockname);
	free(name);
	goto __print_error;

__error:
	sprintf(data->buf, "unexpected character `%c`", *s);
__print_error:
	print_error(data->filename, data->lineno, data->line, data->buf);
__skip_line_and_ret:
	while (*s && *s != '\n') s++;
	return (s);
}
