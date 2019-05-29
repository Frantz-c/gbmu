/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   memory_map.h                                     .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/23 11:44:01 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/05/29 10:27:31 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef MEMORY_MAP_H
# define MEMORY_MAP_H

# include <stdint.h>
# include <stddef.h>

/*
	Realisation des differents switch:

		1) working ram:
			4096 octets a remplacer par copie
			ex:
			`````````````````````````````````````````````````````````````````
				memcpy(extern_ram, extern_ram_banks[4], 8192); //switch bank 5

			`````````````````````````````````````````````````````````````````

			ou
			
			switch "virtuel"
				j'ai implemente une version non testee,
				a base de tableau de label.
				c'est aussi faisable avec un tableau de pointeur ou en cachant
				le tableau de label dans une fonction.
			ex:
			`````````````````````````````````````````````````````````````````
				extern_ram = extern_ram_banks[4];
				get_real_addr[0xa] = extern_ram;
				get_real_addr[0xb] = extern_ram + 0x1000;

			`````````````````````````````````````````````````````````````````
	
		2) ROM:
			16384 octets a remplacer par copie...

			ou

			switch virtuel...


		memory_map_t	memmap;

		uint8_t	*g_get_real_read_addr[16] = {
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

	il faudrait faire une condition pour savoir si on veut
	ecrire ou lire l'adresse (et un deuxieme tableau avec juste la partie ROM).
	(ecrire dans la ROM modifie les registres,
	lire la ROM c'est juste recuperer le code du jeu.)
*/

enum	e_cartridge_types
{
	ROM_ONLY, MBC1, MBC2, MBC3, MBC5
};

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
	uint32_t	n_banks;				//additionnal ROM banks
	uint32_t	mbc;					//mbc number (0 == ROM_ONLY)
}
cartridge_t;

typedef struct	memory_map_s
{

	/**** Pointer to the complete malloced block ****/
	void	*complete_block;


	// Pointers to the different memory areas


	/****** CARTRIDGE ******/

	uint8_t		*fixed_rom;		// ROM:			0x0000 - 0x7fff
								// MBC:			0x0000 - 0x3fff
	uint8_t		*switch_rom;	// MBC Only:	0x4000 - 0x7fff
								// banks 1 to n, bank_size=16384 (16 Kbytes)
		uint8_t	*rom_banks[512];
	
	uint8_t		*vram;			// 0x8000 - 0x9fff
		uint8_t	*vram_banks[2];
	
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

}				memory_map_t;

uint8_t			*g_get_real_read_addr[16] = {NULL};
uint8_t			*g_get_real_write_addr[16] = {NULL};
memory_map_t	g_memmap = {NULL};

# define GET_REAL_READ_ADDR(virtual_addr)	\
		 (\
			(g_get_real_read_addr[(virtual_addr >> 12)]) ?\
				g_get_real_read_addr[(virtual_addr >> 12)] + (virtual_addr & 0xfff) :\
				virtual_addr + g_memmap.complete_block\
		 )
# define GET_REAL_WRITE_ADDR(virtual_addr)	\
		 (\
			(virtual_addr < 0x8000) ?\
				g_get_real_write_addr[(virtual_addr >> 12)] :\
				(g_get_real_write_addr[(virtual_addr >> 12)]) ?\
					g_get_real_write_addr[(virtual_addr >> 12)] + (virtual_addr & 0xfff) :\
					virtual_addr + g_memmap.complete_block\
		 )

#endif
