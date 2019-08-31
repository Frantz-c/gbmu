/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   vector.c                                         .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/11 12:49:01 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/08/31 19:06:23 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "vector.h"

void		*_vector_at_mul(vector_t *vec, size_t index);
void		*_vector_at_shl(vector_t *vec, size_t index);

vector_t	*vector_init(size_t elemsize)
{
	vector_t	*vector;

	vector = (vector_t *)malloc(sizeof(vector_t));
	if (vector == NULL || elemsize == 0)
		return (NULL);
	vector->data = NULL;
	vector->elemsize = elemsize;
	vector->nitems = 0;
	vector->maxitems = 0;
	vector->compar = NULL;
	vector->search = NULL;
	vector->destroy = NULL;
	vector->shift = NOT_POWER_OF_TWO;
	if (((elemsize - 1) & elemsize) == 0)
	{
		vector->shift = 0;
		while ((elemsize >>= 1) != 0)
			vector->shift++;
	}
	return (vector);
}

vector_t	*vector_clone(vector_t *other)
{
	vector_t	*vector;

	vector = (vector_t *)malloc(sizeof(vector_t));
	if (vector == NULL)
		return (NULL);
	vector->data = malloc(other->elemsize * other->nitems);
	if (vector->data == NULL)
	{
		free(vector);
		return (NULL);
	}
	memcpy(vector->data, other->data, other->elemsize * other->nitems);
	vector->elemsize = other->elemsize;
	vector->nitems = other->nitems;
	vector->maxitems = other->nitems;
	vector->compar = other->compar;
	vector->shift = other->shift;
	vector->search = other->search;
	vector->destroy = other->destroy;
	return (vector);
}

void		*vector_at(vector_t *vec, size_t index)
{
	if (vec->shift == NOT_POWER_OF_TWO)
	{
		return (_vector_at_mul(vec, index));
	}
	else
	{
		return (_vector_at_shl(vec, index));
	}
}

int			vector_insert(vector_t *vec, const void *elem, size_t index)
{
	void	*tmp;

	if (vec->nitems == vec->maxitems)
	{
		vec->maxitems = (vec->maxitems == 0) ? 16 : vec->maxitems * 2;
		tmp = realloc(vec->data, vec->maxitems * vec->elemsize);
		if (tmp == NULL)
			return (-1);
		vec->data = tmp;
	}
	if (vec->shift == NOT_POWER_OF_TWO)
	{
		memmove(_vector_at_mul(vec, index + 1), _vector_at_mul(vec, index),
				(vec->nitems - index) * vec->elemsize);
		memcpy(_vector_at_mul(vec, index), elem, vec->elemsize);
	}
	else
	{
		memmove(_vector_at_shl(vec, index + 1), _vector_at_shl(vec, index),
				(vec->nitems - index) << vec->shift);
		memcpy(_vector_at_shl(vec, index), elem, vec->elemsize);
	}
	vec->nitems++;
	return (0);
}

int			vector_push(vector_t *vec, const void *elem)
{
	void	*tmp;

	if (vec->nitems == vec->maxitems)
	{
		vec->maxitems = (vec->maxitems == 0) ? 16 : vec->maxitems * 2;
		tmp = realloc(vec->data, vec->maxitems * vec->elemsize);
		if (tmp == NULL)
			return (-1);
		vec->data = tmp;
	}
	if (vec->shift == NOT_POWER_OF_TWO)
	{
		memcpy(_vector_at_mul(vec, vec->nitems), elem, vec->elemsize);
	}
	else
	{
		memcpy(_vector_at_shl(vec, vec->nitems), elem, vec->elemsize);
	}
	vec->nitems++;
	return (0);
}

void		vector_delete(vector_t *vec, size_t index)
{
	if (vec->shift == NOT_POWER_OF_TWO)
	{
		if (vec->destroy != NULL)
			vec->destroy(_vector_at_mul(vec, index));

		memcpy(_vector_at_mul(vec, index), _vector_at_mul(vec, index + 1),
				(vec->nitems - index) * vec->elemsize);
	}
	else
	{
		if (vec->destroy != NULL)
			vec->destroy(_vector_at_shl(vec, index));

		memcpy(_vector_at_shl(vec, index), _vector_at_shl(vec, index + 1),
				(vec->nitems - index));
	}
	vec->nitems--;
}

void		vector_sort(vector_t *vec)
{
	qsort(vec->data, vec->nitems, vec->elemsize, vec->compar);
}

void		vector_filter(vector_t *vec, int (*filter)(void *))
{
	size_t	i;

	i = 0;
	while (i < vec->nitems)
	{
		if (filter(_vector_at_mul(vec, i)) == 1)
		{
			vector_delete(vec, i);
		}
		else
		{
			i++;
		}
	}
}

ssize_t		vector_search(vector_t *vec, const void *elem)
{
	size_t	left, right, middle;
	int		side;

	if (vec->nitems == 0)
		return (-1);
	left = 0;
	right = vec->nitems;
	while (left < right)
	{
		middle = (left + right) / 2;
		if (vec->shift == NOT_POWER_OF_TWO)
			side = vec->search(elem, _vector_at_mul(vec, middle));
		else
			side = vec->search(elem, _vector_at_shl(vec, middle));
		if (side < 0)
			left = middle + 1;
		else if (side > 0)
			right = middle;
		else
			return ((ssize_t)middle);
	}
	return (-1);
}

size_t		vector_index(vector_t *vec, const void *elem)
{
	size_t	left, right, middle;
	int		side;

	if (vec->nitems == 0)
		return (0);
	left = 0;
	right = vec->nitems;
	while (left < right)
	{
		middle = (left + right) / 2;
		if (vec->shift == NOT_POWER_OF_TWO)
			side = vec->search(elem, _vector_at_mul(vec, middle));
		else
			side = vec->search(elem, _vector_at_shl(vec, middle));
		if (side < 0)
			left = middle + 1;
		else
			right = middle;
	}
	return (left);
}

void		vector_reset(vector_t *vec)
{
	if (vec->destroy != NULL)
	{
		if (vec->shift == NOT_POWER_OF_TWO)
			for (size_t i = 0; i < vec->nitems; i++)
				vec->destroy(_vector_at_mul(vec, i));
		else
			for (size_t i = 0; i < vec->nitems; i++)
				vec->destroy(_vector_at_shl(vec, i));
	}
	vec->nitems = 0;
}

void		vector_destroy(vector_t *vec)
{
	vector_reset(vec);
	if (vec->data != NULL)
		free(vec->data);
	free(vec);
}
