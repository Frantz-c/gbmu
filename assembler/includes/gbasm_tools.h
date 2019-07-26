/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_tools.h                                    .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/16 22:10:32 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/26 21:16:19 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef GBASM_TOOLS_H
# define GBASM_TOOLS_H

# include "std_includes.h"
# include "gbasm_struct.h"

uint32_t	atou_all(char *s, int32_t *err);
uint32_t	atou_inc_all(char **s, int32_t *err);
void		*get_file_contents(const char *path, uint32_t *length);
uint32_t	var_len(const char *s);
uint8_t		is_numeric(const char *s, uint32_t *len);
uint32_t	alnum_len(const char *s);
uint32_t	alpha_len(const char *s);
void		str_to_lower(char *s);
uint32_t	atou_type(char *s, uint32_t *len, uint8_t type);

extern const uint32_t	ascii[256];
extern const uint8_t	to_lower_char[128];

# define FILE_MAX_LENGTH	0x800000u

# define VEC_ELEM(_struct, _var, _index)	((_struct *)((_var)->data + ((_index) * sizeof(_struct))))
# define VEC_ELEM_LAST(_struct, _var)		((_struct *)((_var)->data + (((_var)->nitems - 1) * sizeof(_struct))))

# define HEXA_NUM		16
# define OCTAL_NUM		8
# define DECIMAL_NUM	10
# define BINARY_NUM		2

# define is_digit(c)		(ascii[(uint8_t)c] & 0x01)
# define is_alpha(c)		(ascii[(uint8_t)c] & 0x08)
# define is_lower_alpha(c)	(ascii[(uint8_t)c] & 0x20)
# define is_upper_alpha(c)	(ascii[(uint8_t)c] & 0x10)
# define is_space(c)		(ascii[(uint8_t)c] & 0x02)
# define is_alnum(c)		(ascii[(uint8_t)c] & 0x09)
# define is_endl(c)			(ascii[(uint8_t)c] & 0x40)
# define is_operator(c)		(ascii[(uint8_t)c] & 0x04)
# define is_parent(c)		(ascii[(uint8_t)c] & 0x80)

# define	LOWER(x)	to_lower_char[(uint8_t)x]
// is_operator = '+' || '*'

# define	VEC_SORTED_INSERT(_vector, _string, _new)	\
	register size_t		vindex = vector_index(_vector, (void*)&_string);\
	vector_insert(_vector, (void*)&_new, vindex);


#endif
