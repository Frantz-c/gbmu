/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   export_local_var_to_ext_symbols.c                .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/09/11 12:24:53 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/12 13:18:54 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"

extern void	export_local_var_to_ext_symbols(loc_symbols_t *loc, vector_t *ext)
{
	register loc_var_t	*var = VEC_ELEM_FIRST(loc_var_t, loc->var);
	ext_sym_t			sym;
	
	for (uint32_t i = 0; i < loc->var->nitems; i++, var++)
	{
		if (vector_search(ext, (void*)&var->name) == -1)
		{
			printf("TRANSFER %s\n", var->name);
			sym.name = var->name;
			sym.data = var->data;
			sym.value = var->addr;
			sym.type = VAR;
			VEC_SORTED_INSERT(ext, var->name, sym);
		}
	}
}
