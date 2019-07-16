/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_struct_tools.c                             .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/16 13:17:53 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/16 23:36:56 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "gbasm_struct.h"

#define BYTE_ALLOC_SIZE		8

extern void	push_params(code_area_t	*area, uint8_t inst[2], uint8_t p1[2], uint8_t p2[2], param_t t1, param_t t2, char *symbol)
{
	uint8_t	size = 0;
	uint8_t	opsize = 0;

	// opcode
	if (inst[0] == 0xCB) {
		size++;
		area->cur->opcode[1] = inst[1];
	}
	size++;
	area->cur->opcode[0] = inst[0];

	//param1
	if (t1 >= IMM16) {
		size++;
		opsize++;
		area->cur->operand1[1] = p1[1];
	}
	if (t1 >= IMM8) {
		size++;
		opsize++;
		area->cur->operand1[0] = p1[0];
	}
	area->cur->ope_size = opsize;
	
	//param2
	opsize = 0;
	if (t2 >= IMM16) {
		size++;
		opsize++;
		area->cur->operand2[1] = p2[1];
	}
	if (t2 >= IMM8) {
		size++;
		opsize++;
		area->cur->operand2[0] = p2[0];
	}
	area->cur->ope_size |= (opsize << 4);

	//symbol
	if (t1 == SYMBOL || t1 == SYMBOL8 || t2 == SYMBOL || t2 == SYMBOL8)
	{
		unkwn_sym_t	new = malloc(sizeof(unkwn_sym_t));
		new->name = strdup(symbol);
		new->type = VAR_OR_LABEL;
		area->cur->umkwn = new;
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


/*
extern void	new_instruction(code_area_t *area, char *instruction)
{
	mnemonics_t	*new;

	new = malloc(sizeof(mnemonics_t));
	new->name = instruction;
	new->n_operand = 0;
	new->next = NULL;
	new->operands = NULL;

	if (area->data == NULL)
	{
		area->data = new;
		area->cur = new;
		return;
	}
	area->cur->next = new;
	area->cur = new;
}

extern void	push_operand(code_area_t *area, char *name)
{
	operands_t *new;

	new = malloc(sizeof(operands_t));
	new->name = name;
	new->next = NULL;

	area->cur->n_operand++;
	if (area->cur->operands == NULL)
	{
		area->cur->operands = new;
		return;
	}
	operands_t	*p;

	for (p = area->cur->operands; p->next; p = p->next);
	p->next = new;
}
*/
