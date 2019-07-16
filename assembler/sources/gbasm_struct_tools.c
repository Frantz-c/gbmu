#include "std_includes.h"
#include "gbasm_struct.h"

#define BYTE_ALLOC_SIZE		8

extern void	new_byte_instruction(code_area_t *area)
{
	mnemonics_t	*new;

	new = calloc(sizeof(code_t));

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
	operands_t *new;

	if (area->size == 247)
	{
		return (-1);
	}

	if (area->unkwn == NULL)
	{
		new = malloc(sizeof(uint8_t) * BYTE_ALLOC_SIZE);
	}
	else if ((area->size & 0x7) == 0)
	{
		area->unknw = realloc(area->unkwn, area->size, area->size + BYTE_ALLOC_SIZE);
	}
	(uint8_t*)(area->unkwn)[area->size++] = byte;
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
