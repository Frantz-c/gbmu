#ifndef GBASM_TOOLS_H
# define GBASM_TOOLS_H

# include "std_includes.h"

uint32_t	atou_all(char *s, int32_t *err);
uint32_t	atou_inc_all(char **s, int32_t *err);
void		*get_file_contents(const char *path, uint32_t *length);

extern const uint32_t	ascii[256];

# define FILE_MAX_LENGTH	0x800000u

# define is_digit(c)		(ascii[(uint8_t)c] & 0x01)
# define is_alpha(c)		(ascii[(uint8_t)c] & 0x08)
# define is_lower_alpha(c)	(ascii[(uint8_t)c] & 0x20)
# define is_upper_alpha(c)	(ascii[(uint8_t)c] & 0x10)
# define is_space(c)		(ascii[(uint8_t)c] & 0x02)
# define is_alnum(c)		(ascii[(uint8_t)c] & 0x09)
# define is_endl(c)			(ascii[(uint8_t)c] & 0x42)

#endif
