/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_callback.c                                 .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/12 10:39:48 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/12 17:01:37 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "gbasm_struct.h"

extern void		variables_destroy(void *a)
{
	variables_t	*var = (variables_t *)a;

	free(var->name);
}

extern void		memblock_destroy(void *a)
{
	vector_t	*v;

	free(((memblocks_t *)a)->name);
	v = (((memblocks_t *)a)->var);

	if (v != NULL)
		vector_destroy(v);
}

extern void		macro_destroy(void *a)
{
	macro_t	*macro = (macro_t *)a;

	if (macro->allocated)
	{
		free(macro->name);
		free(macro->content);
	}
}

extern int		macro_destroy_allocated(void *a)
{
	macro_t	*macro = (macro_t *)a;

	if (macro->allocated)
	{
		free(macro->name);
		free(macro->content);
		return (1);
	}
	return (0);
}


extern void		area_destroy(void *a)
{
	mnemonics_t	*m = ((code_area_t*)a)->data;

	while (m)
	{
		operands_t	*o = m->operands;
		while (o)
		{
			operands_t	*tmp = o;
			o = o->next;
			free(tmp->name);
			free(tmp);
		}
		mnemonics_t *tmp = m;
		m = m->next;
		free(tmp->name);
		free(tmp);
	}
}

extern int		area_match(const void *a, const void *b)
{
	if (((code_area_t *)a)->addr > *(uint32_t *)b)
		return (1);
	else if (((code_area_t *)a)->addr == *(uint32_t *)b)
		return (0);
	return (-1);
}

extern int		macro_match(const void *a, const void *b)
{
	return (strcmp(((macro_t *)a)->name, *(char**)b));
}

extern void		label_destroy(const void *a)
{
	free(((label_t *)a)->name);
}

extern int		label_match(const void *a, const void *b)
{
	return (strcmp(((label_t *)a)->name, *(char**)b));
}

extern int		label_cmp(const void *a, const void *b)
{
	return (strcmp(((label_t *)a)->name, ((label_t *)b)->name));
}

extern int		area_cmp(const void *a, const void *b)
{
	if (((code_area_t *)a)->addr > ((code_area_t *)b)->addr)
		return (1);
	else if (((code_area_t *)a)->addr == ((code_area_t *)b)->addr)
		return (0);
	return (-1);
}

extern int		macro_cmp(const void *a, const void *b)
{
	return (strcmp(((macro_t *)a)->name, ((macro_t *)b)->name));
}
