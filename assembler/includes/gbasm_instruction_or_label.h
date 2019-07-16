#ifndef GBASM_INSTRUCTION_OR_LABEL_H
# define GBASM_INSTRUCTION_OR_LABEL_H

char	*add_instruction(char *name, vector_t *area, vector_t *macro, char *s, data_t *data);
void	add_label(char *name, vector_t *area, vector_t *label, data_t *data);

#endif
