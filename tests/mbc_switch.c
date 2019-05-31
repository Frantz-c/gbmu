/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   mbc_switch.c                                     .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/29 17:52:58 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/05/30 16:30:29 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include "memory_map.h"
#include "execute.h"



#define SET_MBC1_MODE_0_ROM_ADDR()	\
	do\
	{\
		SWITCH_ROM = ROM_BANK [ CART_REG[1] | (CART_REG[2] << 5) ];\
		g_get_real_addr[4] = SWITCH_ROM;\
		g_get_real_addr[5] = SWITCH_ROM + 0x1000;\
		g_get_real_addr[6] = SWITCH_ROM + 0x2000;\
		g_get_real_addr[7] = SWITCH_ROM + 0x3000;\
	} while (0)

#define SET_MBC1_MODE_1_ROM_ADDR()	\
	do\
	{\
		ROM_BANK [ CART_REG [MBC1_ROM_NUM] ];\
		g_get_real_addr[4] = SWITCH_ROM;\
		g_get_real_addr[5] = SWITCH_ROM + 0x1000;\
		g_get_real_addr[6] = SWITCH_ROM + 0x2000;\
		g_get_real_addr[7] = SWITCH_ROM + 0x3000;\
	} while (0)

#define SET_MBC1_MODE_0_RAM_ADDR()	\
	do\
	{\
		if (CART_REG[0])\
		{\
			EXTERN_RAM = RAM_BANK[0];\
			g_get_real_addr[10] = EXTERN_RAM;\
			g_get_real_addr[11] = EXTERN_RAM + 0x1000;\
		}\
		CUR_RAM = 0;\
	} while (0)

#define SET_MBC1_MODE_1_RAM_ADDR()	\
	do\
	{\
		if (CART_REG[0])\
		{\
			EXTERN_RAM = RAM_BANK [ CART_REG[2] ];\
			g_get_real_addr[10] = EXTERN_RAM;\
			g_get_real_addr[11] = EXTERN_RAM + 0x1000;\
		}\
		CUR_RAM = CART_REG[2];\
	} while (0)

#define	SWITCH_RAM_ROM_MBC1()	\
	do\
	{\
		if (g_memmap.cart_reg[MBC1_MODE]) {\
			SET_MBC1_MODE_0_ROM_ADDR();\
			SET_MBC1_MODE_0_RAM_ADDR();\
		}\
		else {\
			SET_MBC1_MODE_1_ROM_ADDR();\
			SET_MBC1_MODE_1_RAM_ADDR();\
		}\
	} while (0)

// if write on disabled ram is a fatal error,
// please remove 0x2000 offset
#define ENABLE_EXTERNAL_RAM_MBC1()	\
	if (value == 0x0a)\
	{\
		g_memmap.cart_reg[0] = 1;\
		EXTERN_RAM = RAM_BANK[CUR_RAM];\
	}\
	else\
	{\
		g_memmap.cart_reg[0] = 0;\
		EXTERN_RAM = g_memmap.complete_block + 0x2000;\
	}\
	g_get_real_addr[10] = EXTERN_RAM;\
	g_get_real_addr[11] = EXTERN_RAM + 0x1000;\



//	write_cartridge_register(uint32_t i, uint8_t value, cycle_count_t cycles)
//{
#define	MBC_SWITCH_JUMP_ARRAY	\
	const void *const	jump_to_mbcx[5][8] = {\
		{&&ret, &&ret, &&ret, &&ret, &&ret, &&ret, &&ret, &&ret},\
		{&&mbc1_0, &&mbc1_0, &&mbc1_1, &&mbc1_1, &&mbc1_2, &&mbc1_2, &&mbc1_3, &&mbc1_3},\
		{&&mbc2_0, &&mbc2_0, &&mbc2_1, &&mbc2_1, &&ret, &&ret, &&ret, &&ret},\
		{&&mbc3_0, &&mbc3_0, &&mbc3_1, &&mbc3_1, &&mbc3_2, &&mbc3_2, &&mbc3_3, &&mbc3_3},\
		{&&mbc5_0, &&mbc5_0, &&mbc5_1, &&mbc5_2, &&mbc5_3, &&mbc5_3, &&ret, &&ret}\
	};

#define	MBC_SWITCH_CODE		\
\
/****************\
**    MBC_1    **\
****************/\
mbc1_0:\
	ENABLE_EXTERNAL_RAM_MBC1();\
	return (cycles);\
\
\
mbc1_1:\
	/* 5 bits register */\
	g_memmap.cart_reg[MBC1_ROM_NUM] =\
		((value & 0x1f) == 0) ? 0 : (value & 0x1f) - 1;\
	SWITCH_RAM_ROM_MBC1();\
	return (cycles);\
\
mbc1_2:\
	/* 3 bits register */\
	g_memmap.cart_reg[MBC1_RAM_NUM] = (value & 0x03);\
	SWITCH_RAM_ROM_MBC1();\
	return (cycles);\
\
mbc1_3:\
	/* 1 bit register */\
	g_memmap.cart_reg[MBC1_MODE] = (value & 0x01);\
	SWITCH_RAM_ROM_MBC1();\
	return (cycles);\
\
\
/****************\
**    MBC_2    **\
****************/\
mbc2_0:\
mbc2_1:\
\
\
/****************\
**    MBC_3    **\
****************/\
mbc3_0:\
mbc3_1:\
mbc3_2:\
mbc3_3:\
\
\
/****************\
**    MBC_5    **\
****************/\
mbc5_0:\
mbc5_1:\
mbc5_2:\
mbc5_3:\
\
\
/****************\
**    ERROR    **\
****************/\
ret:\
	*g_memmap.redzone = 0xff; /* read-only memory is written */\
	return (0);\
}
