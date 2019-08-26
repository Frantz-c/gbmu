/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   bank_keyword.c                                   .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/12 16:22:40 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/26 17:47:44 by fcordon     ###    #+. /#+    ###.fr     */
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
	if (args[1].type & STRING_TYPE)
		goto __wrong_type_arg1;


	addr = *(uint32_t*)(args->value) * 0x4000u;
	if (args[1].value) // 2 arguments
	{
		if (args[1].type == STRING_TYPE)
			goto __wrong_type_arg2;
		register uint32_t	offset = *(uint32_t*)(args[1].value);

		if ((addr % 0x4000u) + offset > 0x4000u)
			print_warning(data->filename, data->lineno, data->line, "following code may overlap an other bank");
		addr += offset;
	}


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


/* ||||||||||||||||||||||||||||||||||||||||||*\
** ================ errors ==================**
\* ||||||||||||||||||||||||||||||||||||||||||*/
	register const char	*error_msg;
__wrong_type_arg1:
	error_msg = "argument 1 must be an integer: .bank number[, offset]?";
	goto __print_error;
__wrong_type_arg2:
	error_msg = "argument 2 must be an integer: .bank number[, offset]?";
	goto __print_error;
__too_few_arguments:
	error_msg = "too few arguments: .bank number[, offset]?";
	goto __print_error;
__too_many_arguments:
	error_msg = "too many arguments: .bank number[, offset]?";
	goto __print_error;
__addr_already_used:
	sprintf(data->buf, "address 0x%x already used", addr);
	error_msg = (const char *)data->buf;
__print_error:
	print_error(data->filename, data->lineno, data->line, error_msg);
}
