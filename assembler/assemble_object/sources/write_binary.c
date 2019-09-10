/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   write_binary.c                                   .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/09/10 11:18:42 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/10 11:20:30 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"

/*
00h -  32KBytes (no ROM banking)
01h -  64KBytes (4 banks)
02h - 128KBytes (8 banks)
03h - 256KBytes (16 banks)
04h - 512KBytes (32 banks)
05h -   1MByte (64 banks)  - only 63 banks used by MBC1
06h -   2MBytes (128 banks) - only 125 banks used by MBC1
07h -   4MBytes (256 banks)
08h -   8MBytes (512 banks)
52h - 1.1MBytes (72 banks)
53h - 1.2MBytes (80 banks)
54h - 1.5MBytes (96 banks)
*/
static uint32_t	get_rom_end(void)
{
	switch (cartridge.rom_size)
	{
		case 0x00: return (0x8000u);
		case 0x01: return (0x10000u);
		case 0x02: return (0x20000u);
		case 0x03: return (0x40000u);
		case 0x04: return (0x80000u);
		case 0x05: return (0x100000u); //if mc1 -1 bank
		case 0x06: return (0x200000u); //if mbc1 -1 bank
		case 0x07: return (0x400000u);
		case 0x08: return (0x800000u);
	}
	return (0);
}

extern void		write_binary(vector_t *code, const char *filename)
{
	register all_code_t		*elem = VEC_ELEM_FIRST(all_code_t, code);
	FILE					*file;
	uint8_t					fill[128] = {0};

	if ((file = fopen(filename, "w+")) == NULL)
	{
		fprintf(stderr, "cannot open the file %s\n", filename);
		exit(1);
	}

	register uint32_t	tmp = elem->start;
	register uint32_t	end = elem->end;
	
	printf("WRITE 0x%x\n", elem->start);
	while (tmp > 128)
	{
		fwrite(fill, 1, 128, file);
		tmp -= 128;
	}
	fwrite(fill, 1, tmp, file);
	fwrite(elem->code, 1, elem->end - elem->start, file);
	elem++;

	for (uint32_t i = 1; i < code->nitems; i++, elem++)
	{
	printf("WRITE 0x%x\n", elem->start);
		if (end < elem->start)
		{
			tmp = elem->start - end;
			
			while (tmp > 128)
			{
				fwrite(fill, 1, 128, file);
				tmp -= 128;
			}
			fwrite(fill, 1, tmp, file);
		}
		fwrite(elem->code, 1, elem->end - elem->start, file);
		end = elem->end;
	}

	if (end > get_rom_end())
	{
		fprintf(stderr, "ERROR: code too long\n");
		exit(1);
	}

	if (end < get_rom_end())
	{
		tmp = get_rom_end() - end;
		
		while (tmp > 128)
		{
			fwrite(fill, 1, 128, file);
			tmp -= 128;
		}
		fwrite(fill, 1, tmp, file);
	}
	fclose(file);
}


