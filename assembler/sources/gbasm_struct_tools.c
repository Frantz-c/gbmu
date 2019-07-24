/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_struct_tools.c                             .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/16 13:17:53 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/24 12:52:56 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "gbasm_struct.h"

#define BYTE_ALLOC_SIZE		8

#define OP1	0
#define OP2	1
#define P11	2
#define P12	3
#define P21	4
#define P22	5

extern int32_t		variables_match_name(const vector_t *memblock, const char *s)
{
	register uint32_t	end = memblock->nitems * sizeof(memblock_t);
	for (uint32_t i = 0; i < end; i += sizeof(memblock_t))
	{
		memblock_t	*block = (memblock_t *)(memblock->data + i);
		ssize_t		index;

		if ((index = vector_search(block->var, s)) != -1)
			return ((int32_t)index);
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

extern void	push_instruction(code_area_t *area, uint8_t bin[3], param_t p[2], char *symbol,
						vector_t *ext_symbol, loc_sym_t *loc_symbol, data_t *data)
{
	uint32_t	size;
	uint8_t		opsize = 0;


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
			size++;
			area->cur->opcode[2] = bin[2];
		}
		else if (param >= FF00_IMM8) {
			size++;
			opsize++;
			area->cur->opcode[1] = bin[1];
		}

		//symbol
		if (param == SYMBOL)
		{
			area->cur->symbol = strdup(symbol);

			if (vector_search(loc_symbol->label, symbol) == -1
				&& variables_match_name(loc_symbol->memblock, symbol) == -1)
			{
				if (memblock_match_name(loc_symbol->memblock, symbol) != -1)
				{
					//ERROR
					fprintf(stderr, "can't use variables block as operand\n");
				}
				else if (vector_search(ext_symbol, symbol) == -1)
				{
					label_t	sym = {symbol, 0, NOT_DECLARED};
					size_t	index = vector_index(loc_symbol->label, (void*)&symbol);
					vector_insert(loc_symbol->label, (void*)&sym, index);
				}
			}
		}
	}
	area->cur->size = size;
}

extern void	new_instruction(code_area_t *area)
{
	code_t	*new;

	new = calloc(1, sizeof(code_t));

	if (area->data == NULL)
	{
		puts("area->data == NULL");
		area->data = new;
		area->cur = new;
		return;
	}
	puts("area->data != NULL");
	area->cur->next = new;
	area->cur = new;
}

extern int	push_byte(code_area_t *area, uint8_t byte)
{
	if (area->cur->size == 247)
		return (-1);

	if (area->cur->symbol == NULL)
		area->cur->symbol = (void *)malloc(sizeof(uint8_t) * BYTE_ALLOC_SIZE);
	else if ((area->cur->size & 0x7) == 0)
		area->cur->symbol = realloc(area->cur->symbol, area->cur->size + BYTE_ALLOC_SIZE);
	((uint8_t*)(area->cur->symbol))[area->cur->size++] = byte;

	return (0);
}
