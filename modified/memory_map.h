/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   memory_map.h                                     .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/23 11:44:01 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/06/21 19:00:46 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef MEMORY_MAP_H
# define MEMORY_MAP_H

# include <stdint.h>
# include <stddef.h>
# include <stdbool.h>
//# include "execute.h"
# include "ram_registers.h"
# include "debug_tools.h"

# define _REG_DUMP	false
# define _CPU_LOG	false

/*
		memory_map_t	memmap;

		uint8_t	*g_get_real_addr[16] = {
			memmap.fixed_rom,				//0x0
			memmap.fixed_rom + 0x1000,		//0x1
			memmap.fixed_rom + 0x2000,		//0x2
			memmap.fixed_rom + 0x3000,		//0x3
			memmap.switch_rom,				//0x4
			memmap.switch_rom + 0x1000,		//0x5
			memmap.switch_rom + 0x2000,		//0x6
			memmap.switch_rom + 0x3000,		//0x7
			memmap.vram,					//0x8
			memmap.vram + 0x1000,			//0x9
			memmap.extern_ram,				//0xa
			memmap.extern_ram + 0x1000,		//0xb
			memmap.fixed_ram,				//0xc
			memmap.switch_ram,				//0xd
			NULL,
			NULL
		};

		uint8_t	*g_get_real_write_addr[16] = {
			NULL,							//0x0
			NULL,							//0x1
			NULL,							//0x2
			NULL,							//0x3
			NULL,							//0x4
			NULL,							//0x5
			NULL,							//0x6
			NULL,							//0x7
			memmap.vram,					//0x8
			memmap.vram + 0x1000,			//0x9
			memmap.extern_ram,				//0xa
			memmap.extern_ram + 0x1000,		//0xb
			memmap.fixed_ram,				//0xc
			memmap.switch_ram,				//0xd
			NULL,
			NULL
		};


		exemple:
```````````````````````````````````````````````````````````````````````````
			// acces a l'adresse 0x7e04 -> (memmap.switch_rom)

			uint8_t		*virtual_addr = 0x7e04;
			uint8_t		*real_addr;

			real_addr = GET_REAL_READ_ADDR(virtual_addr);
````````````````````````````````````````````````````````````````````````````
*/

enum	e_cartridge_types
{
	ROM_ONLY, MBC1, MBC2, MBC3, MBC5
};

# define MBC1_RAM_ENABLE		0
# define MBC1_ROM_NUM			1
# define MBC1_RAM_NUM			2
# define MBC1_MODE				3

# define MBC2_RAM_ENABLE		0
# define MBC2_ROM_NUM			1

# define MBC3_RAM_TIMER_ENABLE	0
# define MBC3_ROM_NUM			1
# define MBC3_ROM_RTC_SELECT	2
# define MBC3_CLOCK_DATA		3

# define MBC5_RAM_ENABLE		0
# define MBC5_LOW_ROM_NUM		1
# define MBC5_HI_ROM_NUM		2
# define MBC5_RAM_NUM			3


# define CUR_RAM		g_memmap.cur_extern_ram
# define EXTERN_RAM		g_memmap.extern_ram
# define SWITCH_ROM		g_memmap.switch_rom
# define RAM_BANK		g_memmap.extern_ram_banks
# define ROM_BANK		g_memmap.rom_banks
# define CART_REG		g_memmap.cart_reg

# define NORMAL_MODE	0
# define HALT_MODE		1
# define STOP_MODE		2

# define LYC_REGISTER	g_memmap.complete_block[LYC]
# define LY_REGISTER	g_memmap.complete_block[LY]
# define TIMA_REGISTER	g_memmap.complete_block[TIMA]
# define TMA_REGISTER	g_memmap.complete_block[TMA]
# define TAC_REGISTER	g_memmap.complete_block[TAC]
# define DIV_REGISTER	g_memmap.complete_block[DIV]
# define DMA_REGISTER	g_memmap.complete_block[DMA]
# define IF_REGISTER	g_memmap.complete_block[IF]
# define IE_REGISTER	g_memmap.complete_block[IE]
# define LCDC_REGISTER	g_memmap.complete_block[LCDC]
# define SCX_REGISTER	g_memmap.complete_block[SCX]
# define SCY_REGISTER	g_memmap.complete_block[SCY]
# define WX_REGISTER	g_memmap.complete_block[WX]
# define WY_REGISTER	g_memmap.complete_block[WY]
# define STAT_REGISTER	g_memmap.complete_block[STAT]
# define P1_REGISTER	g_memmap.complete_block[P1]
# define BGP_REGISTER	g_memmap.complete_block[BGP]
# define OBP0_REGISTER	g_memmap.complete_block[OBP0]
# define OBP1_REGISTER	g_memmap.complete_block[OBP1]
# define VBK_REGISTER	g_memmap.complete_block[VBK]
# define SVBK_REGISTER	g_memmap.complete_block[SVBK]
# define IME_REGISTER	g_memmap.ime
//# define _REGISTER	g_memmap.complete_block[]

typedef struct
{
	int32_t		jump_addr;				//0x102 - 0x104
	char		game_title[16];			//0x134 - 0x13e
	char		game_code[8];			//0x13f - 0x142
	int32_t		cgb_support_code;		//0x143
	char		maker_code[8];			//0x144-0x145
	int32_t		sgb_support_code;		//0x146
	int32_t		type;					//0x147
	int32_t		rom_size;				//0x148
	int32_t		extern_ram_size;		//0x149
	int32_t		destination_code;		//0x14a
	int32_t		rom_version;			//0x14c
	int32_t		sum_complement;			//0x14d
	int32_t		hi_check_sum;			//0x14e
	int32_t		lo_check_sum;			//0x14f

	uint32_t	size;					//cartridge total size
	uint32_t	n_rom_banks;			//total of additionnal ROM banks
	uint32_t	n_ram_banks;			//total of additionnal RAM banks
	uint32_t	mbc;					//mbc number (0 == ROM_ONLY)
}
cartridge_t;

typedef struct	memory_map_s
{

	/**** Pointer to the complete malloced block ****/
	uint8_t		*complete_block;


	// Pointers to the different memory areas


	/****** CARTRIDGE ******/

	uint8_t		*fixed_rom;		// ROM:			0x0000 - 0x7fff
								// MBC:			0x0000 - 0x3fff
	uint8_t		*switch_rom;	// MBC Only:	0x4000 - 0x7fff
								// banks 1 to n, bank_size=16384 (16 Kbytes)
		uint8_t	*rom_banks[512];
	
	uint8_t		*vram;			// 0x8000 - 0x9800
		uint8_t	*vram_banks[2];
	uint8_t		*vram_bg[2];	// 0x9800 - 0x9fff
	
	uint8_t		*extern_ram;	// 0xa000 - 0xbfff
		uint8_t	*extern_ram_banks[16];


	/******* GAMEBOY *******/

	uint8_t		*fixed_ram;		// DMG:			0xc000 - 0xdfff
								// CGB:			0xc000 - 0xcfff
	uint8_t		*switch_ram;	// CGB Only:	0xd000 - 0xdfff
		uint8_t	*ram_banks[7];

	uint8_t		*redzone;		// 0xe000 - 0xfdff
	uint8_t		*oam;			// 0xfe00 - 0xfe9f
	uint8_t		*cpu_redzone;	// 0xfea0 - 0xfeff
	uint8_t		*hardware_regs;	// 0xf000 - 0xff7f
	uint8_t		*stack_ram;		// 0xff80 - 0xfffe
	uint8_t		*int_flags;		// 0xffff
	_Bool		ime;
	_Bool		stop_mode;

	uint32_t	cur_extern_ram;	// Numero of extern ram bank

	uint8_t		cart_reg[8];	// cartridge registers
	char		*save_name;		// saved game file name
	uint32_t	mbc;			// mbc number (0 == ROM_ONLY)
	uint32_t	save_size;		// size of ram
	unsigned long	cpu_speed;
}
memory_map_t;


uint8_t			*g_get_real_addr[16];
//uint8_t			*g_get_real_write_addr[16] = {NULL};
memory_map_t	g_memmap;
uint32_t		GAMEBOY;

# define GET_REAL_ADDR(virtual_addr)	\
		 (\
			(g_get_real_addr[((virtual_addr) >> 12)]) ?\
				g_get_real_addr[((virtual_addr) >> 12)] + (virtual_addr & 0xfff) :\
				(virtual_addr) + g_memmap.complete_block\
		 )
# define WRITE_REGISTER_IF_ROM_AREA(virtual_addr, _value, _cycles)	\
		do {\
			if ((virtual_addr) < 0x8000)\
			{\
				cycles = (_cycles);\
				value = (_value); \
				goto *jump_to_mbcx[g_memmap.mbc][(virtual_addr) >> 12];\
			}\
			else if (g_memmap.cart_reg[0] != 0x0a && (virtual_addr) >= 0xa000 && (virtual_addr) < 0xc000)\
			{\
				return (_cycles);\
			}\
		} while(0)


extern void		plog(const char *s);
extern void		plog2(const char *s, uint32_t size);

#endif
