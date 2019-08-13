/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_bank_keyword.c                             .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/12 16:22:40 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/25 09:13:19 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "gbasm_struct.h"
#include "gbasm_tools.h"
#include "gbasm_error.h"

static int	set_addr(char **s, uint32_t *addr, data_t *data)
{
	int32_t		error;

	while (**s == ' ' || **s == '\t') (*s)++;
	
	*addr = atou_inc_all(s, &error);
	if (error == 1)
		goto __unexpected_char;

	*addr *= 0x4000;
	while (**s == ' ' && **s == '\t') (*s)++;
	if (**s == '\n' || **s == '\0')
		return (0);
	if (**s == ',')
		(*s)++;
	else
		goto __unexpected_char;

	*addr += atou_inc_all(s, &error);
	if (error == 1)
		goto __unexpected_char;
	return (0);

/*
 *	=========ERRORS=========
 */
__unexpected_char:
	sprintf(data->buf, "unexpected character `%c`", **s);
	print_error(data->filename, data->lineno, data->line, data->buf);
	return (-1);
}

extern char	*bank_switch(vector_t *area, char *s, data_t *data)
{
	uint32_t	addr;

	if (set_addr(&s, &addr, data) == -1)
		goto __ret_s;

	if	(addr == 0 &&
			(
				vector_size(area) == 1
				|| VEC_ELEM(code_area_t, area, 0)->data == NULL
			)
		)
	{
		//if (vector_size(area) != 1)
			data->cur_area = 0;
		goto __ret_s;
	}

	ssize_t		index;
	if ((index = vector_search(area, (void*)&addr)) != -1)
	{
		if (VEC_ELEM(code_area_t, area, index)->data != NULL)
			goto __addr_already_used;
		data->cur_area = (uint32_t)index;
		goto __ret_s;
	}

	code_area_t	new = {addr, 0, NULL, NULL};

	data->cur_area = vector_index(area, (void*)&addr);
	vector_insert(area, (void*)&new, (size_t)data->cur_area);

	__ret_s:
	while (!is_endl(*s)) s++;
	return (s);

/*
 *	=========ERRORS=========
 */
__addr_already_used:
	sprintf(data->buf, "address 0x%x already used", addr);
	print_error(data->filename, data->lineno, data->line, data->buf);
	goto __ret_s;
}
