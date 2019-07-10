/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   vector.h                                         .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/09 12:05:53 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/07/09 12:58:22 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef VECTOR_H
# define VECTOR_H

# include <string.h>
# include <stdlib.h>
# include <errno.h>

typedef struct	vector_s
{
	void	*data;
	size_t	itemsize;
	size_t	nitems;
	size_t	maxitems;
}				vector_t;

vector_t	*vector_init(size_t itemsize);
vector_t	*vector_init_from(void *data, size_t itemsize, size_t nitems);
vector_t	*vector_init_copy(const void *data, size_t itemsize, size_t nitems);
vector_t	*vector_init_other(const vector_t *other);
vector_t	*vector_init_filter(const vector_t *other, int (*filter)(const void *));

void		vector_pushback(vector_t *vec, const void *item);
void		vector_pushfront(vector_t *vec, const void *item);
void		vector_insert(vector_t *vec, const void *item, size_t index);
void		vector_insert_block(vector_t *vec, const void *items, size_t index, size_t nitems);

void		*vector_popback(vector_t *vec);
void		*vector_popfront(vector_t *vec);
void		*vector_delete(vector_t *vec, size_t index);
void		*vector_delete_block(vector_t *vec, size_t index, size_t nitems);

void		*vector_at(const vector_t *vec, size_t index);
inline void	*vector_at_fast(const vector_t *vec, size_t index)
{
	return (vec->data + index * vec->itemsize);
}

void		vector_set(vector_t *vec, const void *item, size_t index);
inline void	vector_set_fast(vector_t *vec, const void *item, size_t index)
{
	memcpy(vec->data + index * itemsize, item, vec->itemsize);
}

void		vector_sort(vector_t *vec, int (*compar)(const void *, const void *));
void		*vector_locate(const vector_t *vec, int (*compar)(const void *, const void *));
void		vector_filter(vector_t *vec, int (*filter)(const void *));

void		vector_quit(vector_t *vec);

#endif
