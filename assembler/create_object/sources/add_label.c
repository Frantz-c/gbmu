/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   add_label.c                                      .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <fcordon@le-101.fr>                +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/22 22:53:58 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/20 20:09:21 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "struct.h"
#include "struct_tools.h"
#include "tools.h"
#include "error.h"

void	add_label(char *name, vector_t *area, vector_t *ext_symbol, loc_sym_t *loc_symbol, data_t *data)
{
	int32_t		index;

	if ((index = vector_search(loc_symbol->label, (void*)&name)) != -1)
	{
		label_t	*lab = VEC_ELEM(label_t, loc_symbol->label, (uint32_t)index);

		if (lab->base_or_status == NOT_DECLARED)
		{
			register uint32_t	addr = VEC_ELEM(code_area_t, area, data->cur_area)->addr;
			register uint32_t	pos = VEC_ELEM(code_area_t, area, data->cur_area)->size;

			if (addr > 0x7999)
				addr = (addr % 0x4000) + 0x4000;
			lab->base_or_status = addr + pos;
			lab->pos = pos;
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
	else if ((index = vector_search(loc_symbol->memblock, (void *)&name)) != -1)
	{
		memblock_t	*block = VEC_ELEM(memblock_t, loc_symbol->memblock, index);
		
		sprintf(
					data->buf,
					"(#1) duplicate symbol `%s` (previous declaration in file %s:%u)",
					name, block->filename, block->line
				);
		print_error(data->filename, data->lineno, data->line, data->buf);
		free(name);
		return;
	}
	else if ((index = vector_search(loc_symbol->var, (void *)&name)) != -1)
	{
		variable_t	*var = VEC_ELEM(variable_t, loc_symbol->var, index);

		sprintf(
					data->buf,
					"(#2) duplicate symbol `%s` (previous declaration in file %s:%u)",
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
					"(#3) duplicate symbol `%s` (previous declaration in file %s:%u)",
					name, sym->filename, sym->line
				);
		print_error(data->filename, data->lineno, data->line, data->buf);
		free(name);
		return;
	}

	register uint32_t	addr = VEC_ELEM(code_area_t, area, data->cur_area)->addr;
	register uint32_t	pos = VEC_ELEM(code_area_t, area, data->cur_area)->size;
	register size_t		i = vector_index(loc_symbol->label, (void*)&name);

	if (addr > 0x7999)
		addr = ((addr % 0x4000) + 0x4000);
	label_t	new = {name, pos, addr + pos, data->lineno, strdup(data->filename)};

	vector_insert(loc_symbol->label, (void*)&new, i);
}
