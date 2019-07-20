/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_var_keyword.c                              .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/13 22:59:31 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/20 19:35:48 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "gbasm_tools.h"
#include "gbasm_struct.h"
#include "gbasm_error.h"

static uint32_t	get_block_addr(char *block, vector_t *memblock, uint32_t size, uint32_t *index)
{
	uint32_t	var_addr;

	for (uint32_t i = 0; i < memblock->nitems; i++)
	{
		memblock_t	*b = (memblock_t *)(memblock->data + (i * sizeof(memblock_t)));
		if (strcmp(b->name, block) == 0)
		{
			var_addr = b->end - b->space;
			b->space -= size;
			if (b->space >= 0xffffu)
				return (0xfffffffeu);
			*index = i;
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
	char		*blockname;

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
	blockname = s;
	while (is_alnum(*s) || *s == '_') s++;
	blockname = strndup(blockname, s - blockname);

	while (is_space(*s)) s++;
	if (*s != '\n' && *s != '\0')
	{
		free(blockname);
		free(name);
		goto __error;
	}

	uint32_t	index = 0;
	addr = get_block_addr(blockname, memblock, size, &index);
	if (addr == 0xffffffffu)
		goto __unknown_memblock;
	if (addr == 0xfffffffeu)
		goto __no_space;

	memblock_t	*block = VEC_ELEM(memblock_t, memblock, index);
	if (block->var == NULL)
		block->var = vector_init(sizeof(variable_t));

	variable_t	new = {name, addr, size};
	vector_push(block->var, (void*)&new);
	free(blockname);
	return (s);

__no_space:
	sprintf(data->buf, "to few space in `%s`", blockname);
	goto __free_before_print_error;

__unknown_memblock:
	sprintf(data->buf, "unknown memblock `%s`", blockname);
__free_before_print_error:
	free(blockname);
	free(name);
	goto __print_error;

__error:
	sprintf(data->buf, "unexpected character `%c`", *s);
__print_error:
	print_error(data->filename, data->lineno, data->line, data->buf);
	while (*s && *s != '\n') s++;
	return (s);
}
