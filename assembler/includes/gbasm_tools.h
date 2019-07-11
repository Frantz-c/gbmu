#ifndef GBASM_TOOLS_H
# define GBASM_TOOLS_H

# include "std_includes.h"

uint32_t	atou_all(char *s, int32_t *err);
uint32_t	atou_inc_all(char **s, int32_t *err);
void		*get_file_contents(const char *path, uint32_t *length);

#endif
