/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   vector_init.c                                    .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/09 12:30:37 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/07/09 12:57:44 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "vector.h"

vector_t	*vector_init(size_t itemsize)
{
	vector_t	*vector;

	vector = (vector_t *)malloc(sizeof(vector_t));

	if (vector == NULL)
		return (vector);

	vector->data = NULL;
	vector->itemsize = itemsize;
	vector->nitems = 0;
	vector->maxitems = 0;
	return (vector);
}

vector_t	*vector_init_from(void *data, size_t itemsize, size_t nitems)
{
	vector_t	*vector;

	vector = (vector_t *)malloc(sizeof(vector_t));

	if (vector == NULL)
		return (vector);

	vector->data = data;
	vector->itemsize = itemsize;
	vector->nitems = nitems;
	vector->maxitems = nitems;
	return (vector);
}

vector_t	*vector_init_copy(const void *data, size_t itemsize, size_t nitems)
{
	vector_t	*vector;

	vector = (vector_t *)malloc(sizeof(vector_t));

	if (vector == NULL)
		return (vector);

	vector->data = malloc(itemsize * nitems);
	if (vector->data == NULL)
	{
		free(vector);
		vector = NULL;
		return (vector);
	}
	memcpy(vector->data, data, itemsize * nitems);
	vector->itemsize = itemsize;
	vector->nitems = nitems;
	vector->maxitems = nitems;
	return (vector);
}

vector_t	*vector_init_other(const vector_t *other)
{
	return (vector_init_copy(other->data, other->itemsize, other->nitems));
}

vector_t	*vector_init_filter(const vector_t *other, int (*filter)(const void *))
{
	vector_t	*vector;
	size_t		i;
	void		*item;

	vector = vector_init_other(other);

	if (vector == NULL)
		return (vector);

	i = 0;
	while (i < vector->nitems)
	{
		item = vector->data + i * vector->itemsize;
		if (filter(item))
		{
			memcpy(item, item + vector->itemsize, vector->nitems - i + 1);
			vector->nitems--;
		}
		else
			i++;
	}
	return (vector);
}
