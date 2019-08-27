/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   var_keyword.c                                    .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/13 22:59:31 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/27 14:00:11 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "tools.h"
#include "struct_tools.h"
#include "struct.h"
#include "error.h"
#include "callback.h"

enum symbol_name	//?????
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
		goto __print_error;
	}
	if ((index = memblock_match_name(loc_symbol->memblock, name)) != -1)
	{
		memblock_t	*block = VEC_ELEM(memblock_t, loc_symbol->memblock, index);
		
		sprintf(data->buf, err_msg, name, block->filename, block->line);
		goto __print_error;
	}
	if ((index = variables_match_name(loc_symbol->memblock, name, &block_index)) != -1)
	{
		memblock_t	*block = VEC_ELEM(memblock_t, loc_symbol->memblock, block_index);
		variable_t	*var = VEC_ELEM(variable_t, block->var, index);

		sprintf(data->buf, err_msg, name, var->filename, var->line);
		goto __print_error;
	}
	if ((index = vector_search(ext_symbol, (void*)&name)) != -1)
	{
		symbol_t	*sym = VEC_ELEM(symbol_t, ext_symbol, index);

		sprintf(data->buf, err_msg, name, sym->filename, sym->line);
		goto __print_error;
	}
	return (0);

__print_error:
	print_error(data->filename, data->lineno, data->line, data->buf);
	return (-1);	
}

static uint32_t	get_block_addr(const char *block, vector_t *memblock, uint32_t size, uint32_t *index)
{
	uint32_t	var_addr;
	memblock_t	*b = VEC_ELEM_FIRST(memblock_t, memblock);

	for (uint32_t i = 0; i < memblock->nitems; i++, b++)
	{
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

extern void	assign_var_to_memory(loc_sym_t *loc_symbol, vector_t *ext_symbol, uint8_t size, arguments_t args[4], data_t *data)
{
	uint32_t	addr;

	if (size == 0)
		goto __null_size;
	if (args[0].value == NULL || args[1].value == NULL)
		goto __too_few_arguments;
	if (args[2].value != NULL)
		goto __too_many_arguments;
	
	if (args[0].type & INTEGER_TYPE)
		goto __wrong_type_arg1;
	if ((args[0].type & ID_STRING_TYPE) == 0)
		goto __not_well_formated_arg1;
	if (args[1].type & INTEGER_TYPE)
		goto __wrong_type_arg2;
	if ((args[1].type & ID_STRING_TYPE) == 0)
		goto __not_well_formated_arg2;

	// verify if arg1 identifier is not already used
	if (duplicate_symbol((char *)args[0].value, loc_symbol, ext_symbol, data))
		return;

	// verify if arg2 memblock identifier exists and get addr
	const char *blockname = (char*)args[1].value;
	uint32_t	index = 0;
	addr = get_block_addr(blockname, loc_symbol->memblock, size, &index);
	if (addr == 0xffffffffu)
		goto __unknown_memblock;
	if (addr == 0xfffffffeu)
		goto __no_space;

	// push a new variable
	char	*name = strdup((char *)args[0].value);
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
	return;


/* ||||||||||||||||||||||||||||||||||||||||||*\
** ================ errors ==================**
\* ||||||||||||||||||||||||||||||||||||||||||*/
	register const char	*error_msg;
__null_size:
	error_msg = "variable size cannot be 0";
	goto __print_error;
__too_few_arguments:
	error_msg = "too few arguments: .varX var_name, block_name";
	goto __print_error;
__too_many_arguments:
	error_msg = "too many arguments: .varX var_name, block_name";
	goto __print_error;
__not_well_formated_arg1:
	error_msg = "argument 1 format must be [a-zA-Z_][a-zA-Z0-9_]*";
	goto __print_error;
__wrong_type_arg1:
	error_msg = "argument 1 must be a string: .varX var_name, block_name";
	goto __print_error;
__not_well_formated_arg2:
	error_msg = "argument 2 format must be [a-zA-Z_][a-zA-Z0-9_]*";
	goto __print_error;
__wrong_type_arg2:
	error_msg = "argument 2 must be a string: .varX var_name, block_name";
	goto __print_error;
__no_space:
	sprintf(data->buf, "no more space in memblock `%s`", blockname);
	goto __print_error_buffer;
__unknown_memblock:
	sprintf(data->buf, "unknown memblock `%s`", blockname);

__print_error_buffer:
	error_msg = (const char *)data->buf;
__print_error:
	print_error(data->filename, data->lineno, data->line, error_msg);
}
