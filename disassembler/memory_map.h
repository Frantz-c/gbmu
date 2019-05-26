/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   memory_map.h                                     .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/23 11:44:01 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/05/23 16:37:58 by mhouppin    ###    #+. /#+    ###.fr     */
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
				memcpy(wk_ram_switch, wk_ram_banks[4], 4096); //switch bank 5

			`````````````````````````````````````````````````````````````````

			ou
			
			switch "virtuel", ce qui implique de faire plusieurs
			condition lors du chargement/ecriture de donnees a une adresse.
			ex:
			`````````````````````````````````````````````````````````````````
				if (cgb == true && addr >= 0xd000 && addr < 0xe000)
				{
					...
				}

			`````````````````````````````````````````````````````````````````
	
		2) ROM:
			16384 octets a remplacer par copie...

			ou

			switch virtuel...


		faire une table de redirection serait peut-etre mieux.
		il suffira alors juste de changer les adresses
			dans la structure lors des switch.
*/
		memory_map_t	memmap;

		uint8_t	*redirection[4096] = {
			memmap.fixed_rom,			//0x000
			memmap.fixed_rom + 0x1000,	//0x100
			memmap.fixed_rom + 0x2000,	//0x200
			memmap.fixed_rom + 0x3000,	//0x300
			memmap.switch_rom,			//0x400
			memmap.switch_rom + 0x1000,	//0x500
			memmap.switch_rom + 0x2000,	//0x600
			memmap.switch_rom + 0x3000,	//0x700
			memmap.vram,				//0x800
			memmap.vram + 0x1000,		//0x900
			memmap.extern_ram,			//0xa00
			memmap.extern_ram + 0x1000,	//0xb00
			memmap.fixed_ram,			//0xc00
			memmap.switch_ram,			//0xd00
			NULL,						//0xe00
			memmap.oam,					//0xfe0
			NULL,						//0xfea
			memmap.hardware_regs,		//0xff0
			memmap.stack_ram			//0xff8
		};
		//PS: j'ai pas ecrit tous les elements parce que c'est long...
/*
		exemples:
			acces a l'adresse 0x7e04 -> (memmap.switch_rom)
*/
			uint8_t		*addr = 0x7e04;
			uint32_t	final_offset = (addr & 0xf); // % 16
			uint32_t	index = addr >> 4; // / 16

			uint8_t		addr = redirection[index] + final_offset;

/*			
	il faudrait separer read_redirection[] de write_redirection[]
	(ecrire dans la ROM modifie les registres,
	lire la ROM c'est juste recuperer le code du jeu.)
*/

typedef struct	memory_map_s
{

	/**** Pointer to the complete malloced block ****/
	void	*complete_block; //inutile si on utilise les tableaux de redirection


	// Pointers to the different memory areas


	/****** CARTRIDGE ******/

	uint8_t		*fixed_rom;		// ROM:			0x0000 - 0x7fff
								// MBC:			0x0000 - 0x3fff
	uint8_t		*switch_rom;	// MBC Only:		0x4000 - 0x7fff
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

#endif
