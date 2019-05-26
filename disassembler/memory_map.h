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
			
			switch "virtuel"
				j'ai implemente une version non testee,
				a base de tableau de label.
				c'est aussi faisable avec un tableau de pointeur ou en cachant
				le tableau de label dans une fonction.
			ex:
			`````````````````````````````````````````````````````````````````
				wk_ram_switch = wk_ram_banks[4];

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

		const void	*const	get_real_addr[16] = {
			&&fixed_rom_0,				//0x0
			&&fixed_rom_1,				//0x1
			&&fixed_rom_2,				//0x2
			&&fixed_rom_3,				//0x3
			&&switch_rom_0,				//0x4
			&&switch_rom_1,				//0x5
			&&switch_rom_2,				//0x6
			&&switch_rom_3,				//0x7
			&&vram_0,					//0x8
			&&vram_1,					//0x9
			&&extern_ram_0,				//0xa
			&&extern_ram_1,				//0xb
			&&fixed_ram,				//0xc
			&&switch_ram,				//0xd
			&&out_of_range,
			&&out_of_range
		};

		fixed_rom_0:
			real_addr = memmap.fixed_rom + (virtual_addr & 0xfff);
			goto after_call;
		fixed_rom_1:
			real_addr = memmap.fixed_rom + (virtual_addr & 0xfff) + 0x1000;
			goto after_call;
		fixed_rom_2:
			real_addr = memmap.fixed_rom + (virtual_addr & 0xfff) + 0x2000;
			goto after_call;
		fixed_rom_3:
			real_addr = memmap.fixed_rom + (virtual_addr & 0xfff) + 0x3000;
			goto after_call;
		switch_rom_0:
			real_addr = memmap.switch_rom + (virtual_addr & 0xfff);
			goto after_call;
		switch_rom_1:
			real_addr = memmap.switch_rom + (virtual_addr & 0xfff) + 0x1000;
			goto after_call;
		switch_rom_2:
			real_addr = memmap.switch_rom + (virtual_addr & 0xfff) + 0x1000;
			goto after_call;
		switch_rom_3:
			real_addr = memmap.switch_rom + (virtual_addr & 0xfff) + 0x1000;
			goto after_call;
		vram_0:
			real_addr = memmap.vram + (virtual_addr & 0xfff);
			goto after_call;
		vram_1:
			real_addr = memmap.vram + (virtual_addr & 0xfff) + 0x1000;
			goto after_call;
		extern_ram_0:
			real_addr = memmap.extern_ram + (virtual_addr & 0xfff);
			goto after_call;
		extern_ram_1:
			real_addr = memmap.extern_ram + (virtual_addr & 0xfff) + 0x1000;
			goto after_call;
		fixed_ram:
			real_addr = memmap.extern_ram + (virtual_addr & 0xfff);
			goto after_call;
		switch_ram:
			real_addr = memmap.extern_ram + (virtual_addr & 0xfff);
			goto after_call;
		out_of_range:
			//reste juste les adresses superieures a 0xdfff

/*
		exemples:
			acces a l'adresse 0x7e04 -> (memmap.switch_rom)
*/
			uint8_t		*virtual_addr = 0x7e04;
			uint8_t		*real_addr;
			uint32_t	i;

			i = (virtual_addr >> 12);
			goto get_real_addr[i];
			after_call:

/*			
	il faudrait separer get_real_write_addr[] & get_real_read_addr
	(ecrire dans la ROM modifie des registres,
	lire la ROM c'est juste recuperer le code du jeu.)
*/



typedef struct	memory_map_s
{

	/**** Pointer to the complete malloced block ****/
	void	*complete_block;


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
