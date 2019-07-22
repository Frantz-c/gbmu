/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_struct_tools.c                             .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/16 13:17:53 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/22 20:05:42 by fcordon     ###    #+. /#+    ###.fr     */
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

extern void	push_instruction(code_area_t *area, uint8_t bin[6], param_t p[2], char *symbol,
						vector_t *ext_symbol, loc_sym_t *loc_symbol, data_t *data)
{
	uint8_t	size = 0;
	uint8_t	opsize = 0;

	// opcode
	if (bin[0] == 0xCB) {
		size++;
		area->cur->opcode[1] = bin[OP2];
	}
	size++;
	area->cur->opcode[0] = bin[OP1];

	//param1
	if (p[0] >= IMM16) {
		size++;
		opsize++;
		area->cur->operand1[1] = bin[P12];
	}
	if (p[0] >= IMM8) {
		size++;
		opsize++;
		area->cur->operand1[0] = bin[P11];
	}
	area->cur->ope_size = opsize;
	
	//param2
	opsize = 0;
	if (p[1] >= IMM16) {
		size++;
		opsize++;
		area->cur->operand2[1] = bin[P22];
	}
	if (p[1] >= IMM8) {
		size++;
		opsize++;
		area->cur->operand2[0] = bin[P21];
	}
	area->cur->ope_size |= (opsize << 4);

	//symbol
	if (p[0] == SYMBOL || p[1] == SYMBOL)
	{
		unkwn_sym_t	*new = malloc(sizeof(unkwn_sym_t));
		new->name = strdup(symbol);
		new->lineno = data->lineno;
		new->filename = data->filename;
		area->cur->unkwn = new;
		// si non contenu dans loc_symbol
		symbol_t	sym = {new->name, VAR_OR_LABEL};
		vector_push(ext_symbol, (void*)&sym);
	}
	area->cur->size = size;
}

extern void	new_instruction(code_area_t *area)
{
	code_t	*new;

	new = calloc(1, sizeof(code_t));

	if (area->data == NULL)
	{
		area->data = new;
		area->cur = new;
		return;
	}
	area->cur->next = new;
	area->cur = new;
}

extern int	push_byte(code_area_t *area, uint8_t byte)
{
	if (area->cur->size == 247)
		return (-1);

	if (area->cur->unkwn == NULL)
		area->cur->unkwn = (void *)malloc(sizeof(uint8_t) * BYTE_ALLOC_SIZE);
	else if ((area->cur->size & 0x7) == 0)
		area->cur->unkwn = realloc(area->cur->unkwn, area->cur->size + BYTE_ALLOC_SIZE);
	((uint8_t*)(area->cur->unkwn))[area->cur->size++] = byte;

	return (0);
}
