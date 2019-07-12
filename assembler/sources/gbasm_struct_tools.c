#include "std_includes.h"
#include "gbasm_struct.h"

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
