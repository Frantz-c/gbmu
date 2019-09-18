/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   memlock.c                                        .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/10 19:00:27 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/18 18:03:37 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "struct.h"
#include "tools.h"
#include "error.h"
#include "keywords.h"

extern void	set_memlock_area(vector_t *memblock, arguments_t args[4], data_t *data)
{
	uint32_t	addr;
	uint32_t	end;
	char		*name = NULL;

	if (args[0].value == NULL || args[1].value == NULL || args[2].value == NULL)
		goto __too_few_arguments;
	if (args[3].value != NULL)
		goto __too_many_arguments;

	if ((args[0].type & STRING_TYPE) == 0)
		goto __wrong_type_arg1;
	if ((args[0].type & ID_STRING_TYPE) == 0)
		goto __not_well_formated_arg1;
	if ((args[1].type & INTEGER_TYPE) == 0)
		goto __wrong_type_arg2;
	if ((args[2].type & INTEGER_TYPE) == 0)
		goto __wrong_type_arg3;

//	ARGUMENT 1 -> (string) name
	name = strdup((char *)args->value);

//	ARGUMENT 2 -> (uint32) start_addr
	addr = *(uint32_t *)(args[1].value);
	if (addr < 0xA000 || (addr >= 0xE000 && addr < 0xFF80) || addr >= 0xFFFF)
		goto __invalid_region;

//	ARGUMENT 3 -> (uint32) length
	end = *(uint32_t *)(args[2].value) + addr;
	if (end < addr)
		goto __too_little_end;
	if (end > 0xFFFF || (end <= 0xFF80 && end > 0xE000) || end <= 0xA000)
		goto __invalid_region2;
	if (addr >= 0xA000 && addr < 0xC000 && end > 0xC000 && end <= 0xE000)
	{
		sprintf(data->buf, "block `%s` overlap distincts memory area (%hx:%hx)", name, (short)addr, (short)end);
		print_warning(data->filename, data->lineno, data->line, data->buf);
	}

	memblock_t	new = {addr, end, end - addr, data->lineno, name, strdup(data->filename)};
	vector_push(memblock, (void*)&new);

	return;


/* ||||||||||||||||||||||||||||||||||||||||||*\
** ================ errors ==================**
\* ||||||||||||||||||||||||||||||||||||||||||*/
	register const char	*error_msg;
__not_well_formated_arg1:
	error_msg = "argument 1 format must be [a-zA-Z_][a-zA-Z0-9_]*";
	goto __print_error;
__wrong_type_arg1:
	error_msg = "argument 1 must be a string: .memlock name, start_addr, end_addr";
	goto __print_error;
__wrong_type_arg2:
	error_msg = "argument 2 must be an integer: .memlock name, start_addr, end_addr";
	goto __print_error;
__wrong_type_arg3:
	error_msg = "argument 3 must be an integer: .memlock name, start_addr, end_addr";
	goto __print_error;
__too_few_arguments:
	error_msg = "too few arguments: .memlock name, start_addr, end_addr";
	goto __print_error;
__too_many_arguments:
	error_msg = "too many arguments: .memlock name, start_addr, end_addr";
	goto __print_error;
__too_little_end:
	sprintf(data->buf, "in memory block %s, end < start", name);
	error_msg = data->buf;
	goto __print_error;
__invalid_region:
	sprintf(data->buf, "in memory block %s, invalid start address (0x%hX)", name, (uint16_t)addr);
	error_msg = data->buf;
	goto __print_error;
__invalid_region2:
	sprintf(data->buf, "in memory block %s, invalid end address (0x%hX)", name, (uint16_t)end);
	error_msg = (const char *)data->buf;
	goto __print_error;
__print_error:
	print_error(data->filename, data->lineno, data->line, error_msg);
	if (name) free(name);
}
