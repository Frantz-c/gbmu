/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_memlock.c                                  .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/10 19:00:27 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/08 18:15:03 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "gbasm_struct.h"
#include "gbasm_tools.h"
#include "gbasm_error.h"
#include "gbasm_keywords.h"

extern char	*set_memlock_area(vector_t *memblock, char *s, data_t *data)
{
	uint32_t	addr;
	int			error;
	uint32_t	end;
	char		*name = NULL;

//	ARGUMENT 1 -> (string) name
	while (*s == ' ' || *s == '\t') s++;
	name = s;
	if (!is_alpha(*s) && *s != '_')
		goto __error;

	while (is_alnum(*s) || *s == '_') s++;
	if (*s != ' ' && *s != '\t' && *s != ',')
		goto __error;
	name = strndup(name, s - name);
	printf("NAME = \"%s\"\n", name);
	while (*s == ' ' || *s == '\t') s++;
	if (*s == ',') {
		s++;
		while (*s == ' ' || *s == '\t') s++;
	}

//	ARGUMENT 2 -> (uint32) start_addr
	addr = atou_inc_all(&s, &error);
	if (error)
		goto __error;
	if (addr >= 0xfffe || (addr >= 0xfea0 && addr < 0xff80) || (addr >= 0xe000 && addr < 0xfe00) || addr < 0x8000)
		goto __invalid_region;

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
	if (end < addr)
		goto __too_little_end;
	if (end >= 0xfffe || (end >= 0xfea0 && end < 0xff80) || (end >= 0xe000 && end < 0xfe00) || end < 0x8000)
		goto __invalid_region2;

	while (*s == ' ' || *s == '\t') s++;
	if (*s != '\0' && *s != '\n')
		goto __error;

	memblock_t	new = {addr, end, end - addr, data->lineno, name, strdup(data->filename), NULL};
	printf("new.name = \"%s\"\n", new.name);
	vector_push(memblock, (void*)&new);
	printf("\e[1;44m   >  \e[0m  blockname[0] = %s\n", VEC_ELEM_FIRST(memblock_t, memblock)->name);

	return (s);

__too_little_end:
	sprintf(data->buf, "in memory block %s, end < start", name);
	goto __print_error;
__invalid_region:
	sprintf(data->buf, "in memory block %s, invalid start address (0x%hX)", name, (uint16_t)addr);
	goto __print_error;
__invalid_region2:
	sprintf(data->buf, "in memory block %s, invalid end address (0x%hX)", s, (uint16_t)end);
	goto __print_error;
__error:
	sprintf(data->buf, "(#3) unexpected character `%c`", *s);
__print_error:
	print_error(data->filename, data->lineno, data->line, data->buf);
	while (*s && *s != '\n') s++;
	if (name)
		free(name);
	return (s);
}
