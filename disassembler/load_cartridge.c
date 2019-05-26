#include <stdint.h>
#include <stdio.h>
#include <stlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include "memory_map.h"

/* en cours */


typedef struct
{
	int		jump_addr;				//0x102 - 0x104
	char	game_title[16];			//0x134 - 0x13e
	char	game_code[8];			//0x13f - 0x142
	int		cgb_support_code;		//0x143
	char	maker_code[8];			//0x144-0x145
	int		sgb_support_code;		//0x146
	int		type;					//0x147
	int		rom_size;				//0x148
	int		extern_ram_size;		//0x149
	int		destination_code;		//0x14a
	int		rom_version;			//0x14c
	int		sum_complement;			//0x14d
	int		hi_check_sum;			//0x14e
	int		lo_check_sum;			//0x14f
}
t_cartridge;

unsigned char	*get_file_contents(const char *file, uint32_t *length)
{
	unsigned char		*content;
	const int	fd = open(file, O_RDONLY);
	struct stat	filestat;

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
	if ((content = malloc(filestat.st_size + 1)) != NULL)
	{
		*length = read(fd, content, filestat.st_size);
		close(fd);
	}
	return (content);
}

int		get_cartridge_type(uint8_t *mem, t_cartridge *cart)
{
	unsigned char	*start = mem;
	unsigned char	sum = 0;
	
	mem += 0x100;
	if (*mem != 0x00)
		add_error(cart, "[0x100] \e[0;31m0x%hhx\e[0m, 0x00 expected\n", mem);
	if (mem[1] != 0xc3)
		add_error(cart, "[0x101] \e[0;31m0x%hhx\e[0m, 0xc3 expected\n", mem + 1);
	mem += 2;

	cart->jump_addr = *(unsigned short*)mem;
	mem += 2;

	if (memcmp(mem, NITENDO_LOGO, 48))
		add_error(cart, "[0x104-0x133] \e[0;31mnintendo logo error\e[0m\n", NULL);
	mem += 48;

	for (unsigned int i = 0; i < 11; i++)
	{
		if (mem[i] && !isalnum(mem[i]) && mem[i] != '_' && mem[i] != ' ')
			add_error(cart, "[title] \e[0;31m0x%hhx is an invalid char\e[0m\n", mem + i);
	}
	strncpy(cart->game_title, (char*)mem, 11);
	cart->game_title[11] = '\0';
	mem += 11;

	for (unsigned int i = 0; i < 4; i++)
	{
		if (mem[i] && (mem[i] < 'A' || mem[i] > 'Z') && !isdigit(mem[i]))
			add_error(cart, "[game_code] \e[0;31m0x%hhx is an invalid char\e[0m\n", mem + i);
	}
	strncpy(cart->game_code, (char*)mem, 4);
	cart->game_code[4] = 0;
	mem += 4;

	if (*mem != 0x00 && *mem != 0x80 && *mem != 0xc0)
		add_error(cart, "[cgb_support] \e[0;31m0x%hhx is an invalid support code\e[0m, {0x00, 0x80, 0xc0} expected\n", mem);
	cart->cgb_support_code = *(mem++);

	cart->maker_code[0] = *(mem++);
	cart->maker_code[1] = *(mem++);
	cart->maker_code[2] = 0;

	if (*mem != 0x00 && *mem != 0x03)
		add_error(cart, "[sgb_support] \e[0;31m0x%hhx is an invalid support code\e[0m, {0x00, 0x03} expected\n", mem);
	cart->sgb_support_code = *(mem++);

	cart->type = *(mem++);

	if (*mem > 0x08)
		add_error(cart, "[ROM size] \e[0;31m0x%hhx is an invalid code\e[0m, {0x00 - 0x08} expected\n", mem);
	cart->rom_size = *(mem++);

	if (*mem > 0x04)
		add_error(cart, "[RAM size] \e[0;31m0x%hhx is an invalid code\e[0m, {0x00 - 0x04} expected\n", mem);
	cart->extern_ram_size = *(mem++);

	if (*mem > 0x01)
		add_error(cart, "[destination code] \e[0;31m0x%hhx is an invalid code\e[0m, {0x00, 0x01} expected\n", mem);
	cart->destination_code = *(mem++);

	if (*mem != 0x33)
		add_error(cart, "[0x14b] \e[0;31m0x%hhx is an invalid value\e[0m, 0x33 expected\n", mem);
	mem++;

	cart->rom_version = *(mem++);

	for (int i = 0x134; i < 0x14d; i++)
	{
		sum += start[i];
	}
	sum += 0x19U + *mem;
	if (sum != 0x0)
		add_error(cart, "[complement check] \e[0;31m0x%hhx is an invalid sum\e[0m, 0x0 expected\n", &sum);
	cart->sum_complement = *(mem++);


}

void	load_rom_only_cartridge(memmap, cart)
{
	
}

void	load_MBC1_cartridge(memmap, cart)
{
	
}

void	load_MBC2_cartridge(memmap, cart)
{
	fprintf(stderr, "Not implemented...\n");
	return (1);
}

void	load_MBC3_cartridge(memmap, cart)
{
	fprintf(stderr, "Not implemented...\n");
	return (1);
}

void	load_MBC5_cartridge(memmap, cart)
{
	fprintf(stderr, "Not implemented...\n");
	return (1);
}


void	load_cartridge(memory_map_t *memmap, uint8_t *mem, t_cartridge *cart)
{
	int		type;

	if ((type = get_cartridge_type(mem, cart)) == -1)
	{
		fprintf(stderr, "Invalid cartridge\n");
		exit (1);
	}

	switch (type)
	{
		case ROM_ONLY:	load_rom_only_cartridge(memmap, mem); break;
		case MBC_1:		load_MBC1_cartridge(memmap, mem); break;
		case MBC_2:		load_MBC2_cartridge(memmap, mem); break;
		case MBC_3:		load_MBC3_cartridge(memmap, mem); break;
		case MBC_5:		load_MBC5_cartridge(memmap, mem); break;
		default:		fprintf(stderr, "Not supported cartridge type\n");
						exit(1);
	}
}

int		main(int ac, char *av[])
{
	memory_map_t	memmap = {0};
	t_cartridge		cartridge = {0};
	uint8_t			*content;
	uint32_t		size;

	if (ac != 2)
	{
		fprintf(stderr, "%s \"cartridge path\"\n", av[0]);
		return (1);
	}

	if ((content = get_file_contents(av[1], &size)) == NULL)
		return (1);

	load_cartridge(&memmap, content, &cartridge);
	return (0);
}
