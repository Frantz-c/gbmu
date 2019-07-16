/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_var_keyword.c                              .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/13 22:59:31 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/13 23:34:50 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "gbasm_tools.h"
#include "gbasm_struct.h"
#include "gbasm_error.h"

static uint32_t	get_block_addr(char *block, vector_t *memblock, uint32_t size)
{
	uint32_t	var_addr;

	for (uint32_t i = 0; i < memblock->nitems; i++)
	{
		memblocks_t	*b = (memblocks_t *)(memblock->data + (i * sizeof(memblocks_t)));
		if (strcmp(b->name, block) == 0)
		{
			var_addr = b->end - b->space;
			b->space -= size;
			if (b->space >= 0xffffu)
				return (0xfffffffeu);
			return (var_addr);
		}
	}
	return (0xffffffffu);
}

extern char	*assign_var_to_memory(vector_t *memblock, char *s, data_t *data)
{
	uint32_t	size;
	uint32_t	addr;
	int			error;
	char		*name;
	char		*block;

	size = atou_inc_all(&s, &error);
	if (error)
		goto __error;

	while (is_space(*s)) s++;
	if (*s == '\n' || *s == '\0')
		goto __error;

	if (!is_alpha(*s) && *s != '_')
		goto __error;
	name = s;
	while (is_alnum(*s) || *s == '_') s++;
	name = strndup(name, s - name);

	while (is_space(*s)) s++;
	if (*s == ',') {
		s++;
		while (is_space(*s)) s++;
	}

	if (!is_alpha(*s) && *s != '_')
	{
		free(name);
		goto __error;
	}
	block = s;
	while (is_alnum(*s) || *s == '_') s++;
	block = strndup(block, s - block);

	while (is_space(*s)) s++;
	if (*s != '\n' && *s != '\0')
	{
		free(block);
		free(name);
		goto __error;
	}

	addr = get_block_addr(block, memblock, size);
	if (addr == 0xffffffffu)
		goto __unknown_memblock;
	if (addr == 0xfffffffeu)
		goto __no_space;
	variables_t	new = {name, addr, size};
	vector_push(memblock, (void*)&new);
	free(block);
	return (s);

__no_space:
	sprintf(data->buf, "to few space in `%s`", block);
	goto __free_before_print_error;

__unknown_memblock:
	sprintf(data->buf, "unknown memblock `%s`", block);
__free_before_print_error:
	free(block);
	free(name);
	goto __print_error;

__error:
	sprintf(data->buf, "unexpected character `%c`", *s);
__print_error:
	print_error(data->filename, data->lineno, data->line, data->buf);
	while (*s && *s != '\n') s++;
	return (s);
}
