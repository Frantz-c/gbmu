/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_byte_keyword.c                             .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/12 16:22:55 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/16 16:04:55 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "gbasm_struct.h"
#include "gbasm_tools.h"
#include "gbasm_struct_tools.h"
#include "gbasm_macro_func.h"
#include "gbasm_error.h"

extern char		*add_bytes(vector_t *area, char *s, data_t *data)
{
	uint32_t	byte;
	int32_t		error;

	new_instruction( VEC_ELEM(code_area_t, area, data->cur_area) );
	do
	{
		while (is_space(*s)) s++;
		if (is_endl(*s))
			goto __warning_no_bytes;
		byte = atou_inc_all(&s, &error);
		if (error == 1)
			goto __unexpected_char;
		if (byte > 0xffu)
			print_warning(data->filename, data->lineno, data->line, "byte overflow: value truncated");

		if (push_byte(VEC_ELEM(code_area_t, area, data->cur_area) , byte & 0xffu) == -1)
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
		else if (is_endl(*s))
			break;
	}
	while (1);
	VEC_ELEM(code_area_t, area, data->cur_area)->cur->size += 3;
	return (s);

__warning_no_bytes:
	print_warning(data->filename, data->lineno, data->line, ".byte -> no bytes specified");
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
