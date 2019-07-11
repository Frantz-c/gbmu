/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   vector.h                                         .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/11 11:54:57 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/07/11 13:42:30 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef VECTOR_H
# define VECTOR_H

# include <stdlib.h>

# define NOT_POWER_OF_TWO	(8 * sizeof(size_t))

typedef struct	vector_s
{
	void	*data;
	size_t	elemsize;
	size_t	nitems;
	size_t	maxitems;
	int		(*compar)(const void *, const void *);
	void	*tmp_elem;
	size_t	shift;
	size_t	lock;
}				vector_t;

vector_t		*vector_init(size_t elemsize);
vector_t		*vector_clone(vector_t *other);

inline void		*_vector_at_mul(vector_t *vec, size_t index)
{
	return (vec->data + (index * vec->elemsize));
}

inline void		*_vector_at_shl(vector_t *vec, size_t index)
{
	return (vec->data + (index << vec->shift));
}

void		*vector_at(vector_t *vec, size_t index);

int			vector_insert(vector_t *vec, const void *elem, size_t index);
void		vector_delete(vector_t *vec, size_t index);
void		vector_sort(vector_t *vec);
ssize_t		vector_search(vector_t *vec, const void *elem);
size_t		vector_index(vector_t *vec, const void *elem);
void		vector_destroy(vector_t *vec);

#define		vector_reset(vec)	((vec)->nitems = 0)
#define		vector_size(vec)	((vec)->nitems)

#endif
