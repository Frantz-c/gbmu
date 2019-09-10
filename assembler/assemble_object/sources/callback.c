/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   callback.c                                       .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <fcordon@le-101.fr>                +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/09/09 15:23:29 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/10 20:22:05 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"

typedef struct	universal_s
{
	char	*name;
}
universal_t;


// LOC SYMBOLS

extern void	loc_destroy(void *a)
{
	register universal_t	*u = (universal_t *)a;

	free(u->name);
}

extern void	loc_var_destroy(void *a)
{
	register loc_var_t	*v = (loc_var_t *)a;

	free(v->name);
	free(v->blockname);

	register void	*trash;
	for (register var_data_t *d = v->data; d; free(trash), d = d->next)
	{
		trash = d;
		free(d->pos);
	}
}

extern int	loc_ext_compar(const void *a, const void *b)
{
	register universal_t	*u1 = (universal_t *)a;
	register universal_t	*u2 = (universal_t *)b;

	return (strcmp(u1->name, u2->name));
}

extern int	loc_ext_search(const void *b, const void *a)
{
	register universal_t	*u = (universal_t *)a;
	register char			*name = *(char **)b;

	return (strcmp(u->name, name));
}


// EXT SYMBOLS

extern void	ext_destroy(void *a)
{
	register ext_sym_t	*s = (ext_sym_t *)a;

	free(s->name);
	register void	*trash;
	for (register var_data_t *d = s->data; d; free(trash), d = d->next)
	{
		trash = d;
		free(d->pos);
	}
}


// CODE

extern int			code_compar(const void *a, const void *b)
{
	register all_code_t	*aa = (all_code_t *)a;
	register all_code_t	*bb = (all_code_t *)b;

	if (aa->start > bb->start)
		return (1);
	else if (aa->start < bb->start)
		return (-1);
	return (0);
}

extern int			code_search(const void *b, const void *a)
{
	register all_code_t	*aa = (all_code_t *)a;
	register uint32_t	bb = *(uint32_t *)b;

	if (aa->start > bb)
		return (1);
	if (aa->start < bb)
		return (-1);
	return (0);
}

extern void			code_destroy(void *a)
{
	register all_code_t	*aa = (all_code_t *)a;

	free(aa->code);
}


extern int			memblock_compar(const void *a, const void *b)
{
	register memblock_t	*aa = (memblock_t *)a;
	register memblock_t	*bb = (memblock_t *)b;

	return (strcmp(aa->name, bb->name));
}

extern int			memblock_search(const void *b, const void *a)
{
	register memblock_t	*aa = (memblock_t *)a;
	register char			*bb = *(char **)b;

	return (strcmp(aa->name, bb));
}
