#ifndef GBASM_STRUCT_TOOLS_H
# define GBASM_STRUCT_TOOLS_H

# include "std_includes.h"

extern void	new_instruction(code_area_t *area);
extern int	push_byte(code_area_t *area, uint8_t byte);
/*
extern void	new_instruction(code_area_t *area, char *instruction);
extern void	push_operand(code_area_t *area, char *name);
*/
#endif
