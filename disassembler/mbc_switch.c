/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   mbc_switch.c                                     .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/29 17:52:58 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/05/29 20:50:51 by fcordon     ###    #+. /#+    ###.fr     */
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




#define GET_MBC1_MODE_0_ROM_ADDR()	g_memmap.rom_banks[\
	g_memmap.cart_reg[1] | (g_memmap.cart_reg[2] << 5)]

#define GET_MBC1_MODE_1_ROM_ADDR()	\
		g_memmap.rom_banks[g_memmap.cart_reg[MBC1_ROM_NUM]]

#define SET_MBC1_MODE_0_RAM_ADDR()	\
	do\
	{\
		if (g_memmap.cart_reg[0])\
			g_memmap.extern_ram = g_memmap.extern_ram_banks[0];\
		g_memmap.cur_extern_ram = 0;\
	} while (0)\

#define SET_MBC1_MODE_1_RAM_ADDR()	\
	do\
	{\
		if (g_memmap.cart_reg[0])\
			g_memmap.extern_ram = g_memmap.rom_banks[g_memmap.cart_reg[2]]\
		g_memmap.cur_extern_ram = g_memmap.cart_reg[2];\
	} while (0)\

#define	SWITCH_RAM_ROM_MBC1()	\
	do\
	{\
		if (g_memmap.cart_reg[MBC1_MODE]) {\
			g_memmap.switch_rom = GET_MBC1_MODE_0_ROM_ADDR();\
			SET_MBC1_MODE_0_RAM_ADDR();\
		}\
		else {\
			g_memmap.switch_rom = GET_MBC1_MODE_1_ROM_ADDR();\
			SET_MBC1_MODE_1_RAM_ADDR();\
		}\
	} while (0)

// trash... if write on disabled ram is a fatal error, please remove 0x2000 offset
#define ENABLE_EXTERNAL_RAM_MBC1()	\
	if (value == 0x0a)\
	{\
		g_memmap.extern_ram = 1;\
		g_memmap.extern_ram_banks[g_memmap.cur_extern_ram];\
	}\
	else\
	{\
		g_memmap.extern_ram = 0;\
		g_memmap.complete_block + 0x2000;\
	}



void	write_cartridge_register(uint32_t i, uint8_t value)
{
	const void *const	jump_to_mbcx[5][8] = {
		{&&ret, &&ret, &&ret, &&ret, &&ret, &&ret, &&ret, &&ret},
		{&&mbc1_0, &&mbc1_0, &&mbc1_1, &&mbc1_1, &&mbc1_2, &&mbc1_2, &&mbc1_3, &&mbc1_3},
		{&&mbc2_0, &&mbc2_0, &&mbc2_1, &&mbc2_1, &&ret, &&ret, &&ret, &&ret},
		{&&mbc3_0, &&mbc3_0, &&mbc3_1, &&mbc3_1, &&mbc3_2, &&mbc3_2, &&mbc3_3, &&mbc3_3},
		{&&mbc5_0, &&mbc5_0, &&mbc5_1, &&mbc5_2, &&mbc5_3, &&mbc5_3, &&ret, &&ret}
	};

	goto *jump_to_mbcx[g_memmap.mbc][i];


/****************
**    MBC_1    **
****************/
mbc1_0:
	ENABLE_EXTERNAL_RAM_MBC1();
	return;


mbc1_1:
	// 5 bits ROM addr
	g_memmap.cart_reg[MBC1_ROM_NUM] =
		((value & 0x1f) == 0) ? 0 : (value & 0x1f) - 1;
	SWITCH_RAM_ROM_MBC1();
	return;

mbc1_2:
	// 2 bits ROM/RAM addr
	g_memmap.cart_reg[MBC1_RAM_NUM] = (value & 0x03);
	SWITCH_RAM_ROM_MBC1();
	return;

mbc1_3:
	// 1 bit MODE (0 = ROM:ROM, 1 = RAM:ROM)
	g_memmap.cart_reg[MBC1_MODE] = (value & 0x01);
	SWITCH_RAM_ROM_MBC1();
	return;

	// soon...
/****************
**    MBC_2    **
****************/
mbc2_0:
mbc2_1:

	// soon...
/****************
**    MBC_3    **
****************/
mbc3_0:
mbc3_1:
mbc3_2:
mbc3_3:

	// soon...
/****************
**    MBC_5    **
****************/
mbc5_0:
mbc5_0:
mbc5_1:
mbc5_2:
mbc5_3:

/****************
**    ERROR    **
****************/
ret:
	g_memmap.redzone = 0xff; // read-only memory is written
}
