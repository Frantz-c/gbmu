/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_add_label.c                                .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/22 22:53:58 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/26 19:40:49 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "gbasm_struct.h"
#include "gbasm_struct_tools.h"
#include "gbasm_tools.h"
#include "gbasm_error.h"

void	add_label(char *name, vector_t *area, vector_t *ext_symbol, loc_sym_t *loc_symbol, data_t *data)
{
	int32_t		index;
	int32_t		block_index;

	if ((index = vector_search(loc_symbol->label, (void*)&name)) != -1)
	{
		label_t	*lab = VEC_ELEM(label_t, loc_symbol->label, (uint32_t)index);

		if (lab->base_or_status == NOT_DECLARED)
		{
			lab->base_or_status = VEC_ELEM(code_area_t, area, data->cur_area)->addr;
			lab->pos = VEC_ELEM(code_area_t, area, data->cur_area)->count;
		}
		else
		{
			sprintf(
						data->buf,
						"duplicate symbol `%s` (previous declaration in file %s:%u)",
						name, lab->filename, lab->line
					);
			print_error(data->filename, data->lineno, data->line, data->buf);
		}
		free(name);
		return;	
	}
	else if ((index = memblock_match_name(loc_symbol->memblock, name)) != -1)
	{
		memblock_t	*block = VEC_ELEM(memblock_t, loc_symbol->memblock, index);
		
		sprintf(
					data->buf,
					"duplicate symbol `%s` (previous declaration in file %s:%u)",
					name, block->filename, block->line
				);
		print_error(data->filename, data->lineno, data->line, data->buf);
		free(name);
		return;
	}
	else if ((index = variables_match_name(loc_symbol->memblock, name, &block_index)) != -1)
	{
		memblock_t	*block = VEC_ELEM(memblock_t, loc_symbol->memblock, block_index);
		variable_t	*var = VEC_ELEM(variable_t, block->var, index);

		sprintf(
					data->buf,
					"duplicate symbol `%s` (previous declaration in file %s:%u)",
					name, var->filename, var->line
				);
		print_error(data->filename, data->lineno, data->line, data->buf);
		free(name);
		return;
	}
	else if ((index = vector_search(ext_symbol, (void*)&name)) != -1)
	{
		symbol_t	*sym = VEC_ELEM(symbol_t, ext_symbol, index);

		sprintf(
					data->buf,
					"duplicate symbol `%s` (previous declaration in file %s:%u)",
					name, sym->filename, sym->line
				);
		print_error(data->filename, data->lineno, data->line, data->buf);
		free(name);
		return;
	}

	register uint32_t	addr = VEC_ELEM(code_area_t, area, data->cur_area)->addr;
	register uint32_t	pos = VEC_ELEM(code_area_t, area, data->cur_area)->count;
	register size_t		i = vector_index(loc_symbol->label, (void*)&name);

	label_t	new = {name, pos, addr, data->lineno, strdup(data->filename)};
	vector_insert(loc_symbol->label, (void*)&new, i);
}
