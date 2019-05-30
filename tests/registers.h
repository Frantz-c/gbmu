/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   registers.h                                      .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/29 09:45:00 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/05/30 14:49:23 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef REGISTERS_H
# define REGISTERS_H

# include <stdint.h>

# if (BYTE_ORDER == LITTLE_ENDIAN)

#  define reg_a		bytes[0]
#  define reg_f		bytes[1]
#  define reg_b		bytes[2]
#  define reg_c		bytes[3]
#  define reg_d		bytes[4]
#  define reg_e		bytes[5]
#  define reg_h		bytes[6]
#  define reg_l		bytes[7]
#  define reg_ph	bytes[8]
#  define reg_pl	bytes[9]
#  define reg_sh	bytes[10]
#  define reg_sl	bytes[11]

# else

#  define reg_a		bytes[1]
#  define reg_f		bytes[0]
#  define reg_b		bytes[3]
#  define reg_c		bytes[2]
#  define reg_d		bytes[5]
#  define reg_e		bytes[4]
#  define reg_h		bytes[7]
#  define reg_l		bytes[6]
#  define reg_ph	bytes[9]
#  define reg_pl	bytes[8]
#  define reg_sh	bytes[11]
#  define reg_sl	bytes[10]

# endif

# define reg_af	packs[0]
# define reg_bc	packs[1]
# define reg_de	packs[2]
# define reg_hl	packs[3]
# define reg_pc	packs[4]
# define reg_sp	packs[5]

# define BIT_0	0x1u
# define BIT_1	0x2u
# define BIT_2	0x4u
# define BIT_3	0x8u
# define BIT_4	0x10u
# define BIT_5	0x20u
# define BIT_6	0x40u
# define BIT_7	0x80u
# define BIT_11	0x800u
# define BIT_15	0x8000u

# define FLAG_Z		0x80u
# define FLAG_N		0x40u
# define FLAG_H		0x20u
# define FLAG_CY	0x10u

typedef union	registers_u
{
	uint8_t		bytes[12];
	uint16_t	packs[6];
}				registers_t;

#endif
