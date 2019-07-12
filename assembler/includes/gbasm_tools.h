#ifndef GBASM_TOOLS_H
# define GBASM_TOOLS_H

# include "std_includes.h"

uint32_t	atou_all(char *s, int32_t *err);
uint32_t	atou_inc_all(char **s, int32_t *err);
void		*get_file_contents(const char *path, uint32_t *length);

extern const unsigned int	ascii[256];

#define	is_digit(c)			(ascii[c] & 0x01)
#define	is_alpha(c)			(ascii[c] & 0x08)
#define	is_lower_alpha(c)	(ascii[c] & 0x20)
#define	is_upper_alpha(c)	(ascii[c] & 0x10)
#define	is_space(c)			(ascii[c] & 0x02)

#endif
