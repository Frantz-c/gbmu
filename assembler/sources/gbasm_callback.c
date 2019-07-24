/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_callback.c                                 .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/12 10:39:48 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/20 21:50:04 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "gbasm_struct.h"

/*
 *	=============DESTROY=============
 */
extern void		variables_destroy(void *a)
{
	variable_t	*var = (variable_t *)a;

	free(var->name);
}

extern void		memblock_destroy(void *a)
{
	vector_t	*v;

	free(((memblock_t *)a)->name);
	v = (((memblock_t *)a)->var);

	if (v != NULL)
		vector_destroy(v);
}

extern int		macro_filter(void *a)
{
	macro_t	*macro = (macro_t *)a;

	if (macro->allocated)
		return (1);
	return (0);
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
	code_t	*c = ((code_area_t*)a)->data;

	while (c)
	{
		code_t	*tmp;

		if (c->unkwn)
			free(c->unkwn);
		tmp = c;
		c = c->next;
		free(tmp);
	}
}

extern void		label_destroy(void *a)
{
	free(((label_t *)a)->name);
}

extern void		symbol_destroy(void *a)
{
	free(((symbol_t *)a)->name);
}

/*
 *	=============MATCH/SEARCH=============
 */
extern int		area_match(const void *b, const void *a)
{
	if (((code_area_t *)a)->addr > *(uint32_t *)b)
		return (1);
	else if (((code_area_t *)a)->addr == *(uint32_t *)b)
		return (0);
	return (-1);
}

extern int		macro_match(const void *b, const void *a)
{
	return (strcmp( ((macro_t *)a)->name, *(char**)b) );
}

extern int		memblock_match(const void *b, const void *a)
{
	if ( ((memblock_t *)a)->start > *(uint32_t *)b )
		return (1);
	else if ( ((memblock_t *)a)->start == *(uint32_t *)b )
		return (0);
	return (-1);
}

extern int		ext_symbol_match(const void *b, const void *a)
{
	register symbol_t	*sym1 = (symbol_t *)a;

	return (strcmp(sym1->name, *(char **)b));
}

extern int		label_match(const void *b, const void *a)
{
	return (strcmp(((label_t *)a)->name, *(char**)b));
}

/*
 *	=============COMPAR=============
 */

extern int		ext_symbol_match(const void *a, const void *b)
{
	register symbol_t	*sym1 = (symbol_t *)a;
	register symbol_t	*sym2 = (symbol_t *)b;

	return (strcmp(sym1->name, sym2->name));
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

extern int		memblock_cmp(const void *a, const void *b)
{
	if ( ((memblock_t *)a)->start > ((memblock_t *)b)->start )
		return (1);
	else if ( ((memblock_t *)a)->start == ((memblock_t *)b)->start )
		return (0);
	return (-1);
}
