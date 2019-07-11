/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   vector.c                                         .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/11 12:49:01 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/07/11 13:07:01 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "vector.h"

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
	vector->tmp_elem = malloc(elemsize);
	if (vector->tmp_elem == NULL)
	{
		free(vector);
		return (NULL);
	}
	vector->shift = NOT_POWER_OF_TWO;
	if (((elemsize - 1) & elemsize) == 0)
	{
		vector->shift = 0;
		while ((elemsize >>= 1) != 0)
			vector->shift++;
	}
	vector->lock = 0;
	return (vector);
}
