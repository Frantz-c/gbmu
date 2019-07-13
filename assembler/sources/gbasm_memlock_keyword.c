/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_memlock.c                                  .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/10 19:00:27 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/13 23:18:21 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "gbasm_struct.h"
#include "gbasm_tools.h"
#include "gbasm_error.h"
#include "gbasm_keywords.h"

#define	VEC_DATA_ELEM(_struct, _var, _index)	((_struct *)_var->data) + (_index * sizeof(_struct))
#define	VEC_DATA_ELEM_LAST(_struct, _var)	((_struct *)_var->data) + ((_var->nitems - 1) * sizeof(_struct))

extern char	*set_memlock_area(vector_t *memblock, char *s, data_t *data)
{
	uint32_t	addr;
	int			error;
	uint32_t	end;
	char		*name;
	int			alloc = 0;

//	ARGUMENT 1 -> (string) name
	while (*s == ' ' || *s == '\t') s++;
	name = s;
	if (!is_alpha(*s) && *s != '_')
		goto __error;

	while (is_alnum(*s) || *s == '_') s++;
	if (*s != ' ' && *s != '\t' && *s != ',')
		goto __error;
	name = strndup(name, s - name);
	alloc = 1;
	while (*s == ' ' || *s == '\t') s++;
	if (*s == ',') {
		s++;
		while (*s == ' ' || *s == '\t') s++;
	}

//	ARGUMENT 2 -> (uint32) start_addr
	addr = atou_inc_all(&s, &error);
	if (error)
		goto __error;

	while (*s == ' ' || *s == '\t') s++;
	if (*s == ',') {
		s++;
		while (*s == ' ' || *s == '\t') s++;
	}

//	ARGUMENT 3 -> (uint32) end_addr OR length
	if (strncmp(s, "end", 3) == 0)
		end = 0;
	else if (strncmp(s, "len", 3) == 0)
		end = addr;
	else
		goto __error;

	s += 3;
	while (*s == ' ' || *s == '\t') s++;
	if (*s != '=')
		goto __error;

	s++;

	end += atou_inc_all(&s, &error);
	if (error)
		goto __error;

	while (*s == ' ' || *s == '\t') s++;
	if (*s != '\0' && *s != '\n')
		goto __error;

	memblocks_t	new = {addr, end, end - addr, name, NULL};
	vector_push(memblock, (void*)&new);

	return (s);

__error:
	sprintf(data->buf, "unexpected character `%c`", *s);
	print_error(data->filename, data->lineno, data->line, data->buf);
	while (*s && *s != '\n') s++;
	if (alloc)
		free(name);
	return (s);
}
