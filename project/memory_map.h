/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   memory_map.h                                     .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/23 11:44:01 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/05/23 12:36:53 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef MEMORY_MAP_H
# define MEMORY_MAP_H

typedef struct	memory_map
{

	// Pointer to the complete malloced block
	void	*complete_block;

	// Pointers to the different memory areas
	uint8_t	*rst_address;
	uint8_t	*rom_data;
	uint8_t	*user_program;
	uint8_t	*banks[2];
	uint8_t	*bg_display[2];
	uint8_t	*wk_ram;
	uint8_t	*unit_wk_rams[8];
	uint8_t	*redzone;
	uint8_t	*oam;
	uint8_t	*cpu_redzone;
	uint8_t	*hardware_regs;
	uint8_t	*stack_ram;
	uint8_t	*interrupt_flags;

}		memory_map_t;

#endif
