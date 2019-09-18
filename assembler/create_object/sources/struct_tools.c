/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   struct_tools.c                                   .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/16 13:17:53 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/18 18:19:39 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "tools.h"

#define BYTE_ALLOC_SIZE		8

extern macro_t			*get_macro(vector_t *macro, char *name)
{
	ssize_t	index = vector_search(macro, (void*)&name);

	if (index == -1)
		return (NULL);

	return(VEC_ELEM(macro_t, macro, index));
}

extern uint8_t		get_params(char **s, char *param[10])
{
	char	*start;
	uint8_t	nparam = 0;

	for (;;)
	{
		while (is_space(**s)) (*s)++;
		start = *s;
		while (**s != ',' && !is_endl(**s) && **s != ')') (*s)++;
		if (!is_endl(**s))
		{
			char	*end = *s;

			if (nparam == 10)
				return (0xffu); // too many params
			if (end == start)
				return (0xffu); // empty param
			while (is_space(end[-1])) {
				end--;
				if (end == start)
					return (0xffu); // empty param
			}
			param[nparam++] = strndup(start, end - start);
			if (**s == ')')
				break;
			(*s)++;
		}
		if (is_endl(**s))
		{
			while (nparam)
				free(param[--nparam]); // missing ')'
			return (0xffu);
		}
	}
	return (nparam);
}

extern char	*replace_content(macro_t *macro, char *param[10])
{
	char		*new = NULL;
	int32_t		diff = 0;
	uint32_t	len[10];
	char		*pos;
	char		*content = macro->content;
	uint32_t	maxlen;
	uint32_t	length = 0;

	for (uint32_t i = 0; i != macro->argc; i++)
	{
		len[i] = strlen(param[i]);
		diff += len[i] - 2;
	}

	maxlen = strlen(content) + diff;
	new = malloc(maxlen + 1);
//	new[0] = '\0';
	pos = content;
	while ((pos = strchr(content, '#')))
	{
		register uint8_t	i = pos[1] - '0';

		if (maxlen <= length + (pos - content) + len[i])
		{
			maxlen = length + (pos - content) + len[1];
			new = realloc(new, maxlen + 1);
		}
		strncpy(new + length, content, pos - content);
		length += (pos - content);
		strncpy(new + length, param[i], len[i]);
		length += len[i];
		content = pos + 2;
	}
	if (maxlen <= length + strlen(content))
	{
		maxlen = length + strlen(content);
		new = realloc(new, maxlen + 1);
	}
	strncpy(new + length, content, strlen(content));
	length += strlen(content);
	new[length] = '\0';
	return (new);
}
/*
extern int32_t		variables_match_name(const vector_t *memblock, const char *s, int32_t *block_i)
{
	register uint32_t	end = memblock->nitems * sizeof(memblock_t);

	for (uint32_t i = 0; i < end; i += sizeof(memblock_t))
	{
		memblock_t	*block = (memblock_t *)(memblock->data + i);
		ssize_t		index;

		if (block->var)
		{
			if ((index = vector_search(block->var, (void*)&s)) != -1)
			{
				if (block_i) *block_i = i / sizeof(memblock_t);
				return ((int32_t)index);
			}
		}
	}
	return (-1);
}

extern int32_t		memblock_match_name(const vector_t *memblock, const char *s)
{
	register uint32_t	end = memblock->nitems * sizeof(memblock_t);

	for (uint32_t i = 0; i < end; i += sizeof(memblock_t))
	{
		memblock_t *block = (memblock_t *)(memblock->data + i);
		if (strcmp(block->name, s) == 0)
			return ((int32_t)(i / sizeof(memblock_t)));
	}
	return (-1);
}
*/
extern void			push_instruction(code_area_t *area, uint8_t bin[4], param_t p[2], char *symbol,
						vector_t *ext_symbol, loc_sym_t *loc_symbol, data_t *data)
{
	code_t	*new;
	uint8_t	inst_size = inst_length[bin[0]];
	
	new = calloc(1, sizeof(code_t));

	if (inst_size == 2)
	{
		if (bin[0] != 0xCBu)
			new->opcode[2] = bin[3];
	}
	else if (inst_size == 3)
	{
		new->opcode[3] = bin[3];
		new->opcode[2] = bin[2];
	}
	new->opcode[1] = bin[1];
	new->opcode[0] = bin[0];


	//symbol
	if (p[0] == SYMBOL || p[1] == SYMBOL)
	{
		if (symbol[0] == '[')
			memmove(symbol, symbol + 1, strlen(symbol));

		new->symbol = strdup(symbol);
		int32_t	index;

		if (vector_search(loc_symbol->label, (void *)&symbol) == -1
			&& vector_search(loc_symbol->var, (void *)&symbol) == -1)
		{
			if (vector_search(loc_symbol->memblock, (void *)&symbol) != -1)
			{
				print_error(data->filename, data->lineno, data->line, "can't use memory block as operand");
				free(new);
				return;
			}
			else if ((index = vector_search(ext_symbol, (void*)&symbol)) == -1)
			{
				label_t	sym = {symbol, 0, NOT_DECLARED, data->lineno, data->filename};

				size_t	index = vector_index(loc_symbol->label, (void*)&symbol);
				vector_insert(loc_symbol->label, (void*)&sym, index);
			}
			else if (index > -1)
			{
				symbol_t	*sym = VEC_ELEM(symbol_t, ext_symbol, index);

				free(symbol);
				if (sym->type == UNUSED)
				{
					sym->type = VAR_OR_LABEL;
				}
				else if (sym->type == MEMBLOCK)
				{
					print_error(data->filename, data->lineno, data->line, "can't use memory block as operand");
					free(new);
					return;
				}
			}
		}
	}
	
	if (area->cur == NULL)
	{
		area->data = new;
		area->cur = new;
	}
	else
	{
		area->cur->next = new;
		area->cur = new;
	}

	area->cur->filename = strdup(data->filename);
	area->cur->lineno = data->lineno;
	area->cur->addr = area->size + area->addr;
	area->cur->size = inst_length[bin[0]];
	area->size += inst_length[bin[0]];
}

extern void	new_instruction(code_area_t *area)
{
	code_t	*new;

	new = calloc(1, sizeof(code_t));

	if (area->data == NULL)
	{
		area->data = new;
		area->cur = new;
//		puts("area->data == NULL");
		return;
	}
	area->cur->next = new;
	area->cur = new;
//	puts("area->data != NULL");
}

extern int	push_byte(code_area_t *area, uint8_t byte)
{
	if ((area->cur->size & 0x7) == 0)
		area->cur->symbol = realloc(area->cur->symbol, area->cur->size + BYTE_ALLOC_SIZE);
	((uint8_t*)(area->cur->symbol))[area->cur->size++] = byte;
	area->size++;

	return (0);
}
