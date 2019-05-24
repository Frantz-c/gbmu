/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   read_file.c                                      .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/24 14:40:04 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/05/24 23:09:33 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>

#define NITENDO_LOGO		"\xCE\xED\x66\x66\xCC\x0D\x00\x0B\x03\x73\x00\x83"\
							"\x00\x0C\x00\x0D\x00\x08\x11\x1F\x88\x89\x00\x0E"\
							"\xDC\xCC\x6E\xE6\xDD\xDD\xD9\x99\xBB\xBB\x67\x63"\
							"\x6E\x0E\xEC\xCC\xDD\xDC\x99\x9F\xBB\xB9\x33\x3E"

unsigned char	*get_file_contents(const char *file, unsigned int *length)
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

void	print_help(const char *prog)
{
	fprintf(stderr, "%s \"file\"\n\n", prog);
}

char	*cgb_support(int c)
{
	switch (c)
	{
		case 0x00: return ("CGB Incompatible");
		case 0x80: return ("CGB Compatible");
		case 0xc0: return ("CGB Exclusive");
	}
	return ("ERROR");
}

char	*sgb_support(int c)
{
	switch (c)
	{
		case 0x00: return ("Gameboy");
		case 0x03: return ("Uses Super Gameboy Functions");
	}
	return ("ERROR");
}

char	*cartridge_type(int c)
{
	switch (c)
	{
		case 0x00: return ("ROM");
		case 0x01: return ("ROM MBC-1");
		case 0x02: return ("ROM MBC-1 SRAM");
		case 0x03: return ("ROM MBC-1 SRAM BATTERY");
		case 0x04: return ("0x04");
		case 0x05: return ("ROM MBC-2");
		case 0x06: return ("ROM MBC-2 BATTERY");
		case 0x07: return ("0x07");
		case 0x08: return ("ROM SRAM");
		case 0x09: return ("ROM SRAM BATTERY");
		case 0x0f: return ("ROM MBC-3_W/RTC BATTERY");
		case 0x10: return ("ROM MBC-3_W/RTC SRAM BATTERY");
		case 0x11: return ("ROM MBC-3_No_RTC");
		case 0x12: return ("ROM MBC-3_No_RTC SRAM");
		case 0x13: return ("ROM MBC-3_No_RTC SRAM BATTERY");
		case 0x19: return ("ROM MBC-5_No_Rumble");
		case 0x1a: return ("ROM MBC-5_No_Rumble SRAM");
		case 0x1b: return ("ROM MBC-5_No_Rumble SRAM BATTERY");
		case 0x29: return ("ROM MBC-5_W/Rumble");
		case 0x2a: return ("ROM MBC-5_W/Rumble SRAM");
		case 0x2b: return ("ROM MBC-5_W/Rumble SRAM BATTERY");
	}
	return ("ERROR");
}

char	*rom_size(int c)
{
	switch (c)
	{
		case 0x00: return ("256 KBit (32 KByte)");
		case 0x01: return ("512 KBit (64 KByte)");
		case 0x02: return ("1 MBit (128 KByte)");
		case 0x03: return ("2 MBit (256 KByte)");
		case 0x04: return ("4 MBit (512 KByte)");
		case 0x05: return ("8 MBit (1 MByte)");
		case 0x06: return ("16 MBit (2 MByte)");
		case 0x07: return ("32 MBit (4 MByte)");
		case 0x08: return ("64 Mbit (8 MByte)");
	}
	return ("ERROR");
}

char	*ram_size(int c)
{
	switch (c)
	{
		case 0x00: return ("No RAM or MBC-2");
		case 0x01: return ("-------");
		case 0x02: return ("64 KBit (8 KByte)");
		case 0x03: return ("256 KBit (32 KByte)");
		case 0x04: return ("1 MBit (128 KByte)");
	}
	return ("ERROR");
}

char	*dest_code(int c)
{
	switch (c)
	{
		case 0x00: return ("Japan");
		case 0x01: return ("Other than Japan");
	}
	return ("ERROR");
}


typedef struct
{
	char	error[2048];
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

void		print_cartridge_info(t_cartridge *cart)
{
	if (cart->error[0])
		fprintf(stderr, "%s\n\n", cart->error);
	printf("\n"
			"\e[0;33m[START ADDRESSE]\e[0m -> \e[1;35m0x%x\e[0m\n"
			"\e[0;33m[GAME TITLE]    \e[0m -> \e[0;36m%s\e[0m\n"
			"\e[0;33m[GAME CODE]     \e[0m -> \e[0;36m%s\e[0m\n"
			"\e[0;33m[CGB SUPPORT]   \e[0m -> \e[0;36m%s\e[0m\n"
			"\e[0;33m[MAKER CODE]    \e[0m -> \e[0;36m%s\e[0m\n"
			"\e[0;33m[SGB SUPPORT]   \e[0m -> \e[0;36m%s\e[0m\n"
			"\e[0;33m[CARTRIDGE TYPE]\e[0m -> \e[0;36m%s\e[0m\n"
			"\e[0;33m[ROM SIZE]      \e[0m -> \e[0;36m%s\e[0m\n"
			"\e[0;33m[RAM SIZE]      \e[0m -> \e[0;36m%s\e[0m\n"
			"\e[0;33m[DESTINATION]   \e[0m -> \e[0;36m%s\e[0m\n"
			"\e[0;33m[GAME VERSION]  \e[0m -> \e[1;35m%d\e[0m\n"
			"\e[0;33m[CPLT CHECK]    \e[0m -> \e[1;35m%d\e[0m\n\n",
			cart->jump_addr, cart->game_title, cart->game_code,
			cgb_support(cart->cgb_support_code), cart->maker_code,
			sgb_support(cart->sgb_support_code), cartridge_type(cart->type),
			rom_size(cart->rom_size), ram_size(cart->extern_ram_size),
			dest_code(cart->destination_code), cart->rom_version, cart->sum_complement);
}

void		add_error(t_cartridge *cart, const char *fmt, unsigned char *byte)
{
	static int	p = 0;

	if (!byte)
		p += sprintf(cart->error + p, "%s", fmt);
	else
		p += sprintf(cart->error + p, fmt, *byte);
}

t_cartridge	*get_cartridge_info(unsigned char *file)
{
	t_cartridge		*cart = calloc(1, sizeof(t_cartridge));
	unsigned char	*start = file;
	unsigned char	sum = 0;
	
	file += 0x100;
	if (*file != 0x00)
		add_error(cart, "[0x100] \e[0;31m0x%hhx\e[0m, 0x00 expected\n", file);
	if (file[1] != 0xc3)
		add_error(cart, "[0x101] \e[0;31m0x%hhx\e[0m, 0xc3 expected\n", file + 1);
	file += 2;

	cart->jump_addr = *(unsigned short*)file;
	file += 2;

	if (memcmp(file, NITENDO_LOGO, 48))
		add_error(cart, "[0x104-0x133] \e[0;31mnintendo logo error\e[0m\n", NULL);
	file += 48;

	for (unsigned int i = 0; i < 11; i++)
	{
		if (file[i] && !isalnum(file[i]) && file[i] != '_' && file[i] != ' ')
			add_error(cart, "[title] \e[0;31m0x%hhx is an invalid char\e[0m\n", file + i);
	}
	strncpy(cart->game_title, (char*)file, 11);
	cart->game_title[11] = '\0';
	file += 11;

	for (unsigned int i = 0; i < 4; i++)
	{
		if (file[i] && (file[i] < 'A' || file[i] > 'Z') && !isdigit(file[i]))
			add_error(cart, "[game_code] \e[0;31m0x%hhx is an invalid char\e[0m\n", file + i);
	}
	strncpy(cart->game_code, (char*)file, 4);
	cart->game_code[4] = 0;
	file += 4;

	if (*file != 0x00 && *file != 0x80 && *file != 0xc0)
		add_error(cart, "[cgb_support] \e[0;31m0x%hhx is an invalid support code\e[0m, {0x00, 0x80, 0xc0} expected\n", file);
	cart->cgb_support_code = *(file++);

	cart->maker_code[0] = *(file++);
	cart->maker_code[1] = *(file++);
	cart->maker_code[2] = 0;

	if (*file != 0x00 && *file != 0x03)
		add_error(cart, "[sgb_support] \e[0;31m0x%hhx is an invalid support code\e[0m, {0x00, 0x03} expected\n", file);
	cart->sgb_support_code = *(file++);

	cart->type = *(file++);

	if (*file > 0x08)
		add_error(cart, "[ROM size] \e[0;31m0x%hhx is an invalid code\e[0m, {0x00 - 0x08} expected\n", file);
	cart->rom_size = *(file++);

	if (*file > 0x04)
		add_error(cart, "[RAM size] \e[0;31m0x%hhx is an invalid code\e[0m, {0x00 - 0x04} expected\n", file);
	cart->extern_ram_size = *(file++);

	if (*file > 0x01)
		add_error(cart, "[destination code] \e[0;31m0x%hhx is an invalid code\e[0m, {0x00, 0x01} expected\n", file);
	cart->destination_code = *(file++);

	if (*file != 0x33)
		add_error(cart, "[0x14b] \e[0;31m0x%hhx is an invalid value\e[0m, 0x33 expected\n", file);
	file++;

	cart->rom_version = *(file++);

	for (int i = 0x134; i < 0x14d; i++)
	{
		sum += start[i];
	}
	sum += 0x19U + *file;
	if (sum != 0x0)
		add_error(cart, "[complement check] \e[0;31m0x%hhx is an invalid sum\e[0m, 0x0 expected\n", &sum);
	cart->sum_complement = *(file++);

	// ????
	return (cart);
}

int		main(int ac, char *av[])
{
	unsigned char	*file;
	unsigned int	len;
	t_cartridge		*cartridge_info;

	if (ac != 2)
	{
		print_help(av[0]);
		return (1);
	}

	if ((file = get_file_contents(av[1], &len)) == NULL)
		return (1);
	cartridge_info = get_cartridge_info(file);
	free(file);
	print_cartridge_info(cartridge_info);
	free(cartridge_info);

	return (0);
}
