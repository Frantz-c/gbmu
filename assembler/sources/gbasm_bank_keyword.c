#include "std_includes.h"
#include "gbasm_struct.h"
#include "gbasm_tools.h"

static int	set_addr(char **s, uint32_t *addr, error_t *err)
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
	zones_t		*new;
	uint32_t	addr;

	if (set_addr(&s, &addr, err) == -1)
		goto __ret_s;

	if	(addr == 0 &&
			(
				vector_size(area) == 1
				|| ((code_area_t **)(area->data))[0]->data == NULL
			)
		)
	{
		if (vector_size(area) != 1)
			data->cur_area = 0;
		goto _ret_s;
	}

	code_area_t	new = {addr, NULL, NULL};
	if (vector_search(area, (void*)&new) != -1)
		goto __addr_already_used;

	data->cur_area = vector_search(area, (void*)&new);
	vector_insert(area, (void*)&new, (size_t)data->cur_area);

	__ret_s:
	while (*s && *s != '\n') s++;
	return (s);

/*
 *	=========ERRORS=========
 */
__addr_already_used:
	sprintf(data->buf, "address 0x%x already used", addr);
	print_error(data->filename, data->lineno, data->line, data->buf);
	goto __ret_s;
}
