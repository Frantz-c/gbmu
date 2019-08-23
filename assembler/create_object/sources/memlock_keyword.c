/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   memlock.c                                        .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/10 19:00:27 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/23 22:04:32 by fcordon     ###    #+. /#+    ###.fr     */
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
	int			error;
	uint32_t	end;
	char		*name = NULL;

	if (args->value == NULL || args[1].value == NULL || args[2].value == NULL)
		goto __too_few_arguments;
	if (args[3].value != NULL)
		goto __too_many_arguments;
	if (args->type != STRING_TYPE)
		goto __wrong_type_arg1;
	if (args[1].type != INTEGER_TYPE)
		goto __wrong_type_arg2;
	if (args[2].type != INTEGER_TYPE)
		goto __wrong_type_arg3;

//	ARGUMENT 1 -> (string) name
	name = strdup((char *)args->value);

//	ARGUMENT 2 -> (uint32) start_addr
	addr = *(uint32_t *)(args[1].value);
	if (addr >= 0xfffe || (addr >= 0xfea0 && addr < 0xff80) || (addr >= 0xe000 && addr < 0xfe00) || addr < 0x8000)
		goto __invalid_region;

//	ARGUMENT 3 -> (uint32) end_addr
	end = *(uint32_t *)(args[2].value);
	if (end < addr)
		goto __too_little_end;
	if (end >= 0xfffe || (end >= 0xfea0 && end < 0xff80) || (end >= 0xe000 && end < 0xfe00) || end < 0x8000)
		goto __invalid_region2;

	memblock_t	new = {addr, end, end - addr, data->lineno, name, strdup(data->filename), NULL};
	printf("new.name = \"%s\"\n", new.name);
	vector_push(memblock, (void*)&new);
	printf("\e[1;44m   >  \e[0m  blockname[0] = %s\n", VEC_ELEM_FIRST(memblock_t, memblock)->name);

	return;



__too_few_arguments:
	print_error(data->filename, data->lineno, data->line, "too few arguments (3 expected)");
	return;
__too_many_arguments:
	print_error(data->filename, data->lineno, data->line, "too many arguments (3 expected)");
	return;
__too_little_end:
	sprintf(data->buf, "in memory block %s, end < start", name);
	goto __print_error;
__invalid_region:
	sprintf(data->buf, "in memory block %s, invalid start address (0x%hX)", name, (uint16_t)addr);
	goto __print_error;
__invalid_region2:
	sprintf(data->buf, "in memory block %s, invalid end address (0x%hX)", s, (uint16_t)end);
	goto __print_error;
__print_error:
	print_error(data->filename, data->lineno, data->line, data->buf);
	if (name) free(name);
	return (s);
}
