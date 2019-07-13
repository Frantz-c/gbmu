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

#include "gbasm_tools.h"

uint32_t	get_block_addr(char *block, vector_t *memblock)
{
	for (uint32_t i = 0; i < memblock->nitems; i++)
	{
		
	}
}

extern char	*assign_var_to_memory(vector_t *memblock, char *s, data_t *data)
{
	uint32_t	size;
	uint32_t	addr;
	int			error;
	char		*name;
	char		*block;
	int			alloc = 0;

	size = atou_inc(&s, &error);
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
	alloc = 1;

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

	addr = get_block_addr(block, memblock);
	variables_t	new = {name, addr, size};
	vector_push(memblock, (void*)&new);
	free(block);
	return (s);

__error:
	sprintf(data->buf, "unexpected character `%c`", *s);
	print_error(data->filename, data->lineno, data->line, data->buf);
	while (*s && *s != '\n') s++;
	return (s);
}
