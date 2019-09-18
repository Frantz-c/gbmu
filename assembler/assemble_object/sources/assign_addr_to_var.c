/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   assign_addr_to_var.c                             .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <fcordon@le-101.fr>                +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/09/09 15:02:56 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/18 13:43:08 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "callback.h"

extern void		assign_addr_to_var(loc_symbols_t *loc, vector_t *ext)
{
	vector_t	*memblock;

	memblock = vector_init(sizeof(memblock_t));
	memblock->search = &memblock_search;
	memblock->compar = &memblock_compar;


	// add all memory blocks
	{
		register loc_block_t	*elem = VEC_ELEM_FIRST(loc_block_t, loc->block);

		for (uint32_t i = 0; i < loc->block->nitems; i++, elem++)
		{
			memblock_t	new = {elem->start, elem->end, elem->end - elem->start, 0, elem->name};
			if (memblock->nitems == 0)
				vector_push(memblock, (void*)&new);
			else
				VEC_SORTED_INSERT(memblock, elem->name, new);
		}
	}

	// add var in blocks
	{
		register loc_var_t	*elem = VEC_ELEM_FIRST(loc_var_t, loc->var);

		for (uint32_t i = 0; i < loc->var->nitems; i++, elem++)
		{
			register ssize_t	index;

			if ((index = vector_search(memblock, (void*)&elem->blockname)) != -1)
			{
				register memblock_t	*block = VEC_ELEM(memblock_t, memblock, index);

				if (block->space < elem->size)
				{
					fprintf(stderr, "too few space in %s memory block for variable %s\n", block->name, elem->name);
					g_error++;
				}
				else
				{
					elem->addr = block->end - block->space;
					block->space -= elem->size;

					if ((index = vector_search(ext, (void *)&elem->name)) != -1)
					{
						ext_sym_t	*sym = VEC_ELEM(ext_sym_t, ext, index);

						sym->value = elem->addr;
						if (elem->data)
						{
							var_data_t	*d = sym->data;
							while (d->next) d = d->next;
							d->next = elem->data;
						}
					}
					else
					{
						ext_sym_t	new = {elem->name, elem->data, elem->addr, VAR};
						vector_push(ext, (void*)&new);
					}
				}
			}
		}
	}

	// print remaind space in memblocks
	for (uint32_t i = 0; i < memblock->nitems; i++)
	{
		register memblock_t	*block = VEC_ELEM(memblock_t, memblock, i);

		if (block->space != 0)
		{
			fprintf
			(
				stderr,
				"WARNING: %u bytes left in %s memory block\n",
				block->space, block->name
			);
		}
	}

	vector_destroy(memblock);
}
