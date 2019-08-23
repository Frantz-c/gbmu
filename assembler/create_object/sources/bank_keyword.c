/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   bank_keyword.c                                   .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/12 16:22:40 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/23 21:51:33 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "struct.h"
#include "tools.h"
#include "error.h"


// .bank number(, offset)?
extern void	bank_switch(vector_t *area, arguments_t args[4], data_t *data)
{
	uint32_t	addr;

	if (args[0].value == NULL)	// 0 arguments
		goto __too_few_arguments;
	if (args[2].value != NULL)	// more than 2 arguments
		goto __too_many_arguments;


	addr = *(uint32_t*)(args->value) * 0x4000u;
	if (args[1].value) // 2 arguments
	{
		register uint32_t	offset = *(uint32_t*)(args[1].value);

		if ((addr % 0x4000u) + offset > 0x4000u)
			print_warning(data->filename, data->lineno, data->line, "following code may overlap an other bank");
		addr += offset;
	}

/*
	if	(addr == 0 &&
			(
				vector_size(area) == 1
				|| VEC_ELEM(code_area_t, area, 0)->data == NULL
			)
		)
	{
			data->cur_area = 0;
		return;
	}
*/

	// check if addr is already used
	// if any, print an error if no empty
	ssize_t		index;
	if ((index = vector_search(area, (void*)&addr)) != -1)
	{
		if (VEC_ELEM(code_area_t, area, index)->data != NULL)
			goto __addr_already_used;
		data->cur_area = (uint32_t)index;
		return;
	}

	code_area_t	new = {addr, 0, NULL, NULL};

	data->cur_area = vector_index(area, (void*)&addr);
	vector_insert(area, (void*)&new, (size_t)data->cur_area);
	return;


__too_few_arguments:
	print_error(data->filename, data->lineno, data->line, "too few arguments (1 or 2 expected)");
	return;
__too_many_arguments:
	print_error(data->filename, data->lineno, data->line, "too many arguments (1 or 2 expected)");
	return;
__addr_already_used:
	sprintf(data->buf, "address 0x%x already used", addr);
	print_error(data->filename, data->lineno, data->line, data->buf);
}
