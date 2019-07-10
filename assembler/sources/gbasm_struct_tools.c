#include "std_includes.h"
#include "gbasm_struct.h"

extern void	new_instruction(zones_t *z, char *instruction)
{
	mnemonics_t	*new;

	new = malloc(sizeof(mnemonics_t));
	new->name = instruction;
	new->n_operand = 0;
	new->next = NULL;
	new->operands = NULL;

	if (z->data == NULL)
	{
		z->data = new;
		z->cur = new;
		return;
	}
	z->cur->next = new;
	z->cur = new;
}

extern void	push_operand(zones_t *z, char *name)
{
	operands_t *new;

	new = malloc(sizeof(operands_t));
	new->name = name;
	new->next = NULL;

	z->cur->n_operand++;
	if (z->cur->operands == NULL)
	{
		z->cur->operands = new;
		return;
	}
	
	operands_t	*p;
	for (p = z->cur->operands; p->next; p = p->next);
	p->next = new;
}
