/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   registers.h                                      .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/23 12:40:01 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/05/23 12:54:40 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef REGISTERS_H
# define REGISTERS_H

# define reg_a	byte[0]
# define reg_f	byte[1]

# if (BYTE_ORDER == LITTLE_ENDIAN)
#  define reg_b	byte[2]
#  define reg_c	byte[3]
#  define reg_d	byte[4]
#  define reg_e	byte[5]
#  define reg_h	byte[6]
#  define reg_l	byte[7]
# else
#  define reg_b	byte[3]
#  define reg_c	byte[2]
#  define reg_d	byte[5]
#  define reg_e	byte[4]
#  define reg_h	byte[7]
#  define reg_l	byte[6]
# endif

# define reg_bc	packed_bytes[1]
# define reg_de	packed_bytes[2]
# define reg_hl	packed_bytes[3]
# define reg_sp	packed_bytes[4]
# define reg_pc	packed_bytes[5]

typedef union	regs_u
{
	char	byte[12];
	short	packed_bytes[6];
}		regs_t;

#endif
