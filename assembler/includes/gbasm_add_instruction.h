
#ifndef GBASM_ADD_INSTRUCTION_H
# define GBASM_ADD_INSTRUCTION_H

#include "gbasm_struct.h"

extern char		*add_instruction_or_label(zones_t **zon, zones_t **curzon,
								defines_t *def[], char *s, error_t *err);

#endif
