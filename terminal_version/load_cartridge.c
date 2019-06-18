/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   load_cartridge.c                                 .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/29 17:38:18 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/06/18 15:22:01 by fcordon     ###    #+. /#+    ###.fr     */
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

/* en cours... */	

#define NITENDO_LOGO		"\xCE\xED\x66\x66\xCC\x0D\x00\x0B\x03\x73\x00\x83"\
							"\x00\x0C\x00\x0D\x00\x08\x11\x1F\x88\x89\x00\x0E"\
							"\xDC\xCC\x6E\xE6\xDD\xDD\xD9\x99\xBB\xBB\x67\x63"\
							"\x6E\x0E\xEC\xCC\xDD\xDC\x99\x9F\xBB\xB9\x33\x3E"

#define ROM_MAX_LENGTH		0x800000

static void		put_file_contents(const char *file, const void *content, uint32_t length)
{
	const int		fd = open(file, O_WRONLY | O_TRUNC | O_CREAT, 0664);

	if (fd == -1)
	{
		fprintf(stderr, "can't open file %s\n", file);
		exit(1);
	}
	if (write(fd, content, length) != length)
	{
		fprintf(stderr, "save failure\n");
		exit(1);
	}
	close(fd);
}

extern void		save_external_ram(void)
{
	put_file_contents(g_memmap.save_name, g_memmap.extern_ram, g_memmap.save_size);
}

static unsigned char	*get_file_contents(const char *file, uint32_t *length)
{
	unsigned char	*content;
	const int		fd = open(file, O_RDONLY);
	struct stat		filestat;

	if (fd == -1)
	{
		fprintf(stderr, "can't open file %s\n", file);
		return (NULL);
	}
	lstat(file, &filestat);
	if (filestat.st_size == 0)
	{
		fprintf(stderr, "empty file\n");
		return (NULL);
	}
	else if (filestat.st_size > ROM_MAX_LENGTH)
	{
		fprintf(stderr, "too heavy file\n");
		return (NULL);
	}
	if ((content = valloc(filestat.st_size)) == NULL)
	{
		fprintf(stderr, "malloc fatal error\n");
		return (NULL);
	}
	*length = read(fd, content, filestat.st_size);
	close(fd);
	return (content);
}

static uint32_t	get_external_ram_size(cartridge_t *cart)
{
	switch (cart->extern_ram_size)
	{
		case 0x02: cart->n_ram_banks = 0; return (0x2000);
		case 0x03: cart->n_ram_banks = 4; return (0x8000);
		case 0x04: cart->n_ram_banks = 16; return (0x20000);
		case 0x05: cart->n_ram_banks = 8; return (0x10000);
	}
	return (0);
}

static int		set_cartridge_info(uint8_t *mem, cartridge_t *cart)
{
	uint8_t	*start = mem;
	uint8_t	sum = 0;
	
	mem += 0x100;
	if (*mem != 0x00 || mem[1] != 0xc3)
		return (-1);
	mem += 2;

#if BYTE_ORDER == LITTLE_ENDIAN
	cart->jump_addr = *(unsigned short*)mem;
#else
	cart->jump_addr = (*mem | (mem[1] << 8));
#endif
	mem += 2;

	if (memcmp(mem, NITENDO_LOGO, 48))
		return (-1);
	mem += 48;

	for (uint32_t i = 0; i < 11; i++)
	{
		if (mem[i] && !isalnum(mem[i]) && mem[i] != '_' && mem[i] != ' ')
			return (-1);
	}
	strncpy(cart->game_title, (char*)mem, 11);
	cart->game_title[11] = '\0';
	mem += 11;

	for (unsigned int i = 0; i < 4; i++)
	{
		if (mem[i] && (mem[i] < 'A' || mem[i] > 'Z') && !isdigit(mem[i]))
			return (-1);
	}
	strncpy(cart->game_code, (char*)mem, 4);
	cart->game_code[4] = 0;
	mem += 4;

	if (*mem != 0x00 && *mem != 0x80 && *mem != 0xc0)
		return (-1);
	cart->cgb_support_code = *(mem++);

	cart->maker_code[0] = *(mem++);
	cart->maker_code[1] = *(mem++);
	cart->maker_code[2] = 0;

	if (*mem == 0x03 && *(start + 0x14b) != 0x33)
		return (-1);
	cart->sgb_support_code = *(mem++);

	cart->type = *(mem++);

	if (*mem > 0x08)
		return (-1);
	cart->rom_size = *(mem++);

	if (*mem > 0x04)
		return (-1);
	cart->extern_ram_size = *(mem++);

	if (*mem > 0x01)
		return (-1);
	cart->destination_code = *mem;

	mem += 2;

	cart->rom_version = *(mem++);

	for (int i = 0x134; i < 0x14d; i++)
	{
		sum += start[i];
	}
	sum += 0x19U + *mem;
	if (sum != 0x0)
		return (-1);
	cart->sum_complement = *(mem++);

	return (cart->type);
}

static void	malloc_blocks(cartridge_t *cart)
{
	// 0x0000 - 0xffff
	g_memmap.complete_block = valloc(0x10000);

	// 0x0000 - 0x3fff
	g_memmap.fixed_rom = NULL;

	// 0x4000 - 0x7fff
	g_memmap.switch_rom = NULL;

	// 0x8000 - 0xa000
	g_memmap.vram = valloc(0x4000);
	g_memmap.vram_banks[0] = g_memmap.vram;
	g_memmap.vram_banks[1] = g_memmap.vram + 0x2000;
	g_memmap.vram_bg[0] = g_memmap.vram + 0x1800;
	g_memmap.vram_bg[1] = g_memmap.vram + 0x3800;

	// 0xa000 - 0xbfff
	if ((g_memmap.save_size = get_external_ram_size(cart)) == 0)
		g_memmap.extern_ram = NULL;
	else
	{
		g_memmap.extern_ram = valloc(g_memmap.save_size);
		g_memmap.extern_ram_banks[0] = g_memmap.extern_ram;
		for (unsigned int i = 1; i < cart->n_ram_banks; i++)
			g_memmap.extern_ram_banks[i] = g_memmap.extern_ram_banks[i - 1] + 0x2000;
	}

	// 0xc000 - 0xcfff (+ 7 banks allocations)
	g_memmap.fixed_ram = valloc(0x8000);
	g_memmap.switch_ram = g_memmap.fixed_ram + 0x1000;
	g_memmap.ram_banks[0] = g_memmap.switch_ram;
	for (uint32_t i = 1; i < 8; i++)
		g_memmap.ram_banks[i] = g_memmap.ram_banks[i - 1] + 0x1000;

	// Bootstrap ??? 0xe000 - 0xfdff
	g_memmap.redzone = g_memmap.complete_block + 0xe000;
	// 0xfe00 - 0xfe9f
	g_memmap.oam = g_memmap.complete_block + 0xfe00;
	// 0xfea0 - 0xfeff
	g_memmap.cpu_redzone = g_memmap.complete_block + 0xfea0;
	// 0xff00 - 0xff7f
	g_memmap.hardware_regs = g_memmap.complete_block + 0xff00;
	// 0xff80 - 0xfffe
	g_memmap.stack_ram = g_memmap.complete_block + 0xff80;
	// 0xffff
	g_memmap.int_flags = g_memmap.complete_block + 0xffff;

	// convertion virtual addresse to real addresse
	g_get_real_addr[8] = g_memmap.vram;				//0x8000
	g_get_real_addr[9] = g_memmap.vram + 0x1000;		//0x9000
	if (g_memmap.extern_ram == NULL)
	{
		g_get_real_addr[10] = g_memmap.complete_block;	//0xa000
		g_get_real_addr[11] = g_memmap.complete_block;	//0xb000
	}
	else
	{
		g_get_real_addr[10] = g_memmap.extern_ram;			//0xa000
		g_get_real_addr[11] = g_memmap.extern_ram + 0x1000;//0xb000
	}
	g_get_real_addr[12] = g_memmap.fixed_ram;			//0xc000
	g_get_real_addr[13] = g_memmap.switch_ram;			//0xd000
}

static void	load_cartridge_on_memory(uint8_t *mem, cartridge_t *cart, uint32_t type)
{
	uint32_t	cartsize = 0x8000;
	uint8_t		*rom;

	if (cart->rom_size) {
		cartsize = 0x8000 << cart->rom_size;
		cart->n_rom_banks = 2 << cart->rom_size;
	}
	else {
		cartsize = 0x8000;
		cart->n_rom_banks = 0;
	}

	if (cart->size != cartsize) {
		fprintf(stderr, "\e[0;31minvalid cartridge size\e[0m\n");
		exit(1);
	}

	rom = valloc(cartsize);
	memcpy(rom, mem, cartsize);
	g_memmap.fixed_rom = rom;
	rom += 0x4000;
	for (uint32_t i = 0; i < cart->n_rom_banks; i++)
	{
		g_memmap.rom_banks[i] = rom;
		rom += 0x4000;
	}

	g_memmap.switch_rom = g_memmap.fixed_rom + 0x4000;

	g_get_real_addr[0] = g_memmap.fixed_rom;			//0x0000
	g_get_real_addr[1] = g_memmap.fixed_rom + 0x1000;	//0x1000
	g_get_real_addr[2] = g_memmap.fixed_rom + 0x2000;	//0x2000
	g_get_real_addr[3] = g_memmap.fixed_rom + 0x3000;	//0x3000
	g_get_real_addr[4] = g_memmap.switch_rom;			//0x4000
	g_get_real_addr[5] = g_memmap.switch_rom + 0x1000;	//0x5000
	g_get_real_addr[6] = g_memmap.switch_rom + 0x2000;	//0x6000
	g_get_real_addr[7] = g_memmap.switch_rom + 0x3000;	//0x7000
}

static void	load_rom_only_cartridge(uint8_t *mem, cartridge_t *cart)
{
	cart->mbc = ROM_ONLY;
	g_memmap.mbc = 0;
	malloc_blocks(cart);
	load_cartridge_on_memory(mem, cart, ROM_ONLY);
	puts("\e[0;32mCARTRIDGE LOADED WITH SUCCESS\e[0m");
}

static void	load_MBC1_cartridge(uint8_t *mem, cartridge_t *cart)
{
	cart->mbc = MBC1;
	g_memmap.mbc = 1;
	malloc_blocks(cart);
	load_cartridge_on_memory(mem, cart, MBC1);
	puts("\e[0;32mCARTRIDGE LOADED WITH SUCCESS\e[0m");
}

static void	load_MBC2_cartridge(uint8_t *mem, cartridge_t *cart)
{
	cart->mbc = MBC2;
	g_memmap.mbc = 2;
	malloc_blocks(cart);
	load_cartridge_on_memory(mem, cart, MBC2);
	puts("\e[0;32mCARTRIDGE LOADED WITH SUCCESS\e[0m");
}

static void	load_MBC3_cartridge(uint8_t *mem, cartridge_t *cart)
{
	cart->mbc = MBC3;
	g_memmap.mbc = 3;
	malloc_blocks(cart);
	load_cartridge_on_memory(mem, cart, MBC3);
	puts("\e[0;32mCARTRIDGE LOADED WITH SUCCESS\e[0m");
}

static void	load_MBC5_cartridge(uint8_t *mem, cartridge_t *cart)
{
	cart->mbc = MBC5;
	g_memmap.mbc = 4;
	malloc_blocks(cart);
	load_cartridge_on_memory(mem, cart, MBC5);
	puts("\e[0;32mCARTRIDGE LOADED WITH SUCCESS\e[0m");
}

static void	load_saved_external_ram(cartridge_t *cart, const char *path)
{
	char		*save_name = valloc(strlen(path) + 2);
	char		*p;
	uint8_t		*save;
	uint32_t	length;

	strcpy(save_name, path);
	if ((p = strrchr(save_name, '.')) == NULL || (strcmp(".gb", p) && strcmp(".gbc", p)))
	{
		fprintf(stderr, "\e[0;31mcartridge extension not recognized: can't load saved game\n\e[0m");
		g_memmap.save_name = NULL;
		return;
	}
	strcpy(p + 1, "sav");
	g_memmap.save_name = save_name;
	if ((g_memmap.save_size = get_external_ram_size(cart)) == 0)
	{
		free(g_memmap.save_name);
		g_memmap.save_name = NULL;
		return;
	}

	if ((save = get_file_contents(save_name, &length)) == NULL)
	{
		bzero(g_memmap.extern_ram, g_memmap.save_size);
		return ;
	}
	if (length != g_memmap.save_size)
	{
		fprintf(stderr, "%s: wrong size, abort loading\n", save_name);
		free(save);
		return;
	}
	memcpy(g_memmap.extern_ram, save, g_memmap.save_size);
	free(save);
}

static void	load_cartridge(uint8_t *mem, cartridge_t *cart, const char *path)
{
	if (set_cartridge_info(mem, cart) == -1)
	{
		fprintf(stderr, "Invalid cartridge\n");
		exit (1);
	}

	switch (cart->type)
	{
		case 0x00:	load_rom_only_cartridge(mem, cart); break;
		case 0x01:
		case 0x02:
		case 0x03:	load_MBC1_cartridge(mem, cart); break;
		case 0x05:
		case 0x06:	load_MBC2_cartridge(mem, cart); break;
		case 0x0f:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:	load_MBC3_cartridge(mem, cart); break;
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:	load_MBC5_cartridge(mem, cart); break;
		default:	fprintf(stderr, "Not supported cartridge type\n");
					exit(1);
	}

	g_memmap.cart_reg[0] = 0;
	g_memmap.cart_reg[1] = 0;
	g_memmap.cart_reg[2] = 0;
	g_memmap.cart_reg[3] = 0;
	g_memmap.cart_reg[4] = 0;
	g_memmap.cart_reg[5] = 0;
	g_memmap.cart_reg[6] = 0;
	g_memmap.cart_reg[7] = 0;

	switch (cart->type)
	{
		case 0x03:
		case 0x06:
		case 0x09:
		case 0x10:
		case 0x13:
		case 0x1b:
		case 0x1e: load_saved_external_ram(cart, path);
	}
	g_memmap.cur_extern_ram = 0;
	g_memmap.cart_reg[0] = 0;
	g_memmap.cart_reg[1] = 0;
	g_memmap.cart_reg[2] = 0;
	g_memmap.cart_reg[3] = 0;
	g_memmap.cart_reg[4] = 0;
}

extern void		open_cartridge(const char *path)
{
	cartridge_t		cartridge = {0, {0}};
	uint8_t			*content;

	if ((content = get_file_contents(path, &cartridge.size)) == NULL)
		exit(1);
	load_cartridge(content, &cartridge, path);
	free(content);
}
