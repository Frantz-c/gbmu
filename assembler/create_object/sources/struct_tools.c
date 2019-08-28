/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   struct_tools.c                                   .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/16 13:17:53 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/28 16:26:12 by fcordon     ###    #+. /#+    ###.fr     */
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

extern void			push_instruction(code_area_t *area, uint8_t bin[4], param_t p[2], char *symbol,
						vector_t *ext_symbol, loc_sym_t *loc_symbol, data_t *data)
{
	uint32_t	size;
//	uint8_t		opsize = 0;
	code_t		*new;
	
	printf("\e[1;40m<><><>\e[0m ADD {0x%x,  0x%x 0x%x,  0x%x}\n", bin[0], bin[1], bin[2], bin[3]);
	new = calloc(1, sizeof(code_t));
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

	// opcode
	if (bin[0] == 0xCB)
	{
		size = 2;
		area->cur->opcode[1] = bin[1];
		area->cur->opcode[0] = bin[0];
	}
	else
	{
		param_t		param = p[0];


		size = 1;
		area->cur->opcode[0] = bin[0];

		if (param < p[1])
			param = p[1];

		if (param >= IMM16) {
			size += 2;
			area->cur->opcode[2] = bin[2];
			area->cur->opcode[1] = bin[1];
		}
		else if (param >= FF00_IMM8) {
			size++;
//			opsize++;
			area->cur->opcode[1] = bin[1];
		}

		//symbol
		if (param == SYMBOL)
		{
			if (symbol[0] == '(')
				memmove(symbol, symbol + 1, strlen(symbol));

			area->cur->symbol = strdup(symbol);
			int32_t	index;

			if (vector_search(loc_symbol->label, (void*)&symbol) == -1
				&& variables_match_name(loc_symbol->memblock, symbol, NULL) == -1)
			{
				if (memblock_match_name(loc_symbol->memblock, symbol) != -1)
				{
					fprintf(stderr, "can't use memory block as operand\n");
					return;
				}
				else if ((index = vector_search(ext_symbol, (void*)&symbol)) == -1)
				{
					label_t	sym = {symbol, 0, NOT_DECLARED, data->lineno, data->filename};

					size_t	index = vector_index(loc_symbol->label, (void*)&symbol);
					vector_insert(loc_symbol->label, (void*)&sym, index);
					printf("\e[1;33minsert label undeclared %s\n", sym.name);
				}
				else if (index > -1)
				{
					symbol_t	*sym = VEC_ELEM(symbol_t, ext_symbol, index);

					free(symbol);
					if (sym->type == UNUSED)
					{
						printf("\e[1;44m<><>\e[0msymbol %s unused -> var_or_label\n", sym->name);
						sym->type = VAR_OR_LABEL;
					}
					else if (sym->type == MEMBLOCK)
					{
						fprintf(stderr, "can't use memory block as operand\n");
						return;
					}
				}
			}
			if (bin[3])
			{
				area->cur->opcode[3] = bin[3];
				area->cur->opcode[2] = bin[2];
				area->cur->opcode[1] = bin[1];
			}
		}
	}
	printf("\e[1;42m   \e[0maddr = 0x%x, size = %u\n", area->addr, area->size);
	area->cur->addr = area->size + area->addr;
	area->cur->size = size;
	area->size += size;
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
//	if (area->cur->size > )
//		return (-1);

	if (area->cur->symbol == NULL)
		area->cur->symbol = (void *)malloc(sizeof(uint8_t) * BYTE_ALLOC_SIZE);
	else if ((area->cur->size & 0x7) == 0)
		area->cur->symbol = realloc(area->cur->symbol, area->cur->size + BYTE_ALLOC_SIZE);
	((uint8_t*)(area->cur->symbol))[area->cur->size++] = byte;
	area->size++;

	return (0);
}
