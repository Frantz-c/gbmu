/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   registers.h                                      .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/23 12:40:01 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/05/23 15:52:19 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef REGISTERS_H
# define REGISTERS_H

# include "memory_map.h"

# if (BYTE_ORDER == LITTLE_ENDIAN)
#  define reg_b	byte[0]
#  define reg_c	byte[1]
#  define reg_d	byte[2]
#  define reg_e	byte[3]
#  define reg_h	byte[4]
#  define reg_l	byte[5]
#  define reg_a byte[6]
#  define reg_f	byte[7]

#  define reg(n)	(n)
# else
#  define reg_b	byte[1]
#  define reg_c	byte[0]
#  define reg_d	byte[3]
#  define reg_e	byte[2]
#  define reg_h	byte[5]
#  define reg_l	byte[4]
#  define reg_a	byte[7]
#  define reg_f	byte[6]

#  define reg(n)	(n ^ 1)
# endif

# define reg_bc	packed_bytes[0]
# define reg_de	packed_bytes[1]
# define reg_hl	packed_bytes[2]
# define reg_sp	packed_bytes[4]
# define reg_pc	packed_bytes[5]

# define FLAG_C	1u
# define FLAG_H	2u
# define FLAG_N	4u
# define FLAG_Z	8u

# define SHL_C	0
# define SHL_H	1
# define SHL_N	2
# define SHL_Z	3

typedef uint32_t	cycle_count_t;

typedef union		reg_u
{
	uint8_t		byte[12];
	uint16_t	packed_bytes[6];
}					reg_t;

cycle_count_t	execute(reg_t *regs, memory_map_t *map);

#endif
