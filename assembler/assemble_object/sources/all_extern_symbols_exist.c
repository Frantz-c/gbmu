/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   all_extern_symbols_exist.c                       .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <fcordon@le-101.fr>                +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/09/09 14:58:09 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/09 19:10:49 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"

extern void		all_extern_symbols_exist(loc_symbols_t *loc, vector_t *ext, char *file[])
{
	register ext_sym_t	*elem = VEC_ELEM_FIRST(ext_sym_t, ext);

	for (uint32_t i = 0; i < ext->nitems; i++, elem++)
	{
		if (vector_search(loc->label, (void*)&elem->name) == -1
				&& vector_search(loc->var, (void*)&elem->name) == -1
				&& vector_search(loc->block, (void*)&elem->name) == -1)
		{
			g_error++;
			fprintf(
				stderr,
				"assembler: undefined symbol %s (called in file(s): ",
				elem->name
			);
			if (elem->data)
			{
				for (var_data_t *d = elem->data; ; d = d->next)
				{
					if (d->next == NULL)
					{
						fprintf(stderr, "%s", file[d->file_number]);
						break;
					}
					else
						fprintf(stderr, "%s, ", file[d->file_number]);
				}
			}
			fprintf(stderr, ")\n");
		}
	}
}
