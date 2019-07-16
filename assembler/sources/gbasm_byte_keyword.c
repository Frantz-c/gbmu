/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_byte_keyword.c                             .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/12 16:22:55 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/13 21:40:58 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "gbasm_struct.h"
#include "gbasm_tools.h"
#include "gbasm_struct_tools.h"
#include "gbasm_macro_func.h"
#include "gbasm_error.h"

#define INT_TO_STRPTR(exp)	(char*)((long)(exp))

extern char		*add_bytes(vector_t *area, char *s, data_t *data)
{
	uint32_t	byte;
	int32_t		error;


	new_byte_instruction(area->data + (data->cur_area * sizeof(code_area_t)));
	do
	{
		while (*s == ' ' || *s == '\t') s++;
		byte = atou_inc_all(&s, &error);
		if (error == 1)
			goto __unexpected_char;
		if (byte > 0xffu)
			print_warning(data->filename, data->lineno, data->line, "byte overflow: value truncated");

		if (push_byte(area->data + (data->cur_area * sizeof(code_area_t)), byte & 0xffu) == -1)
			goto __too_many_bytes;

		while (*s == ' ' || *s == '\t') s++;
		if (*s == '\\')
		{
			s++;
			while (*s)
			{
				if (*s == '\n') {
					s++;
					break;
				}
				else if (*s != ' ' && *s != '\t')
					goto __unexpected_char;
				s++;
			}
		}
		else if (*s == ',')
			s++;
		else if (*s == '\n' || *s == '\0')
			break;
	}
	while (1);

	return (s);

__too_many_bytes:
	sprintf(data->buf, "more than 248 bytes, please cut this instruction");
	goto __print_error;
__unexpected_char:
	sprintf(data->buf, "unexpected char `%c`", *s);
__print_error:
	print_error(data->filename, data->lineno, data->line, data->buf);
	skip_macro(&s, &data->lineno);
	return (s);
}
