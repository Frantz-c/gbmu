#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include "memory_map2.h"

/* en cours... */	

#define NITENDO_LOGO		"\xCE\xED\x66\x66\xCC\x0D\x00\x0B\x03\x73\x00\x83"\
							"\x00\x0C\x00\x0D\x00\x08\x11\x1F\x88\x89\x00\x0E"\
							"\xDC\xCC\x6E\xE6\xDD\xDD\xD9\x99\xBB\xBB\x67\x63"\
							"\x6E\x0E\xEC\xCC\xDD\xDC\x99\x9F\xBB\xB9\x33\x3E"

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

void	load_rom_only_cartridge(memory_map_t *memmap, uint8_t *mem, t_cartridge *cart)
{
	fprintf(stderr, "soon...\n");
	exit(1);
}

void	load_MBC1_cartridge(memory_map_t *memmap, uint8_t *mem, t_cartridge *cart)
{
	fprintf(stderr, "soon...\n");
	exit(1);
}

void	load_MBC2_cartridge(memory_map_t *memmap, uint8_t *mem, t_cartridge *cart)
{
	fprintf(stderr, "Not implemented...\n");
	exit (1);
}

void	load_MBC3_cartridge(memory_map_t *memmap, uint8_t *mem, t_cartridge *cart)
{
	fprintf(stderr, "Not implemented...\n");
	exit (1);
}

void	load_MBC5_cartridge(memory_map_t *memmap, uint8_t *mem, t_cartridge *cart)
{
	fprintf(stderr, "Not implemented...\n");
	exit (1);
}


void	load_cartridge(memory_map_t *memmap, uint8_t *mem, t_cartridge *cart)
{
	if (get_cartridge_type(mem, cart) == -1)
	{
		fprintf(stderr, "Invalid cartridge\n");
		exit (1);
	}

	switch (cart->type)
	{
		case 0x00:	load_rom_only_cartridge(memmap, mem, cart); break;
		case 0x01:
		case 0x02:
		case 0x03:	load_MBC1_cartridge(memmap, mem, cart); break;
		case 0x05:
		case 0x06:	load_MBC2_cartridge(memmap, mem, cart); break;
		case 0x0f:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:	load_MBC3_cartridge(memmap, mem, cart); break;
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:	load_MBC5_cartridge(memmap, mem, cart); break;
		default:	fprintf(stderr, "Not supported cartridge type\n");
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
