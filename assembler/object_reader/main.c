/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   main.c                                           .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <fcordon@le-101.fr>                +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/08/31 21:59:27 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/31 23:33:27 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

typedef struct	cart_info_s
{
	uint16_t	program_start:1;
	uint16_t	game_title:1;
	uint16_t	game_code:1;
	uint16_t	cgb_support:1;
	uint16_t	maker_code:1;
	uint16_t	sgb_support:1;
	uint16_t	cart_type:1;
	uint16_t	rom_size:1;
	uint16_t	ram_size:1;
	uint16_t	destination:1;
	uint16_t	version:1;
}
cart_info_t;


const char	*get_cgb_support(uint8_t c)
{
	switch (c)
	{
		case 0x00: return ("incompatible");
		case 0x80: return ("compatible");
		case 0xc0: return ("exclusive");
	}
	return ("ERROR");
}

const char	*get_sgb_support(uint8_t c)
{
	switch (c)
	{
		case 0x00: return ("supported");
		case 0x03: return ("not supported");
	}
	return ("ERROR");
}
/*
00h  ROM ONLY                 19h  MBC5
01h  MBC1                     1Ah  MBC5+RAM
02h  MBC1+RAM                 1Bh  MBC5+RAM+BATTERY
03h  MBC1+RAM+BATTERY         1Ch  MBC5+RUMBLE
05h  MBC2                     1Dh  MBC5+RUMBLE+RAM
06h  MBC2+BATTERY             1Eh  MBC5+RUMBLE+RAM+BATTERY
08h  ROM+RAM                  20h  MBC6
09h  ROM+RAM+BATTERY          22h  MBC7+SENSOR+RUMBLE+RAM+BATTERY
0Bh  MMM01
0Ch  MMM01+RAM
0Dh  MMM01+RAM+BATTERY
0Fh  MBC3+TIMER+BATTERY
10h  MBC3+TIMER+RAM+BATTERY   FCh  POCKET CAMERA
11h  MBC3                     FDh  BANDAI TAMA5
12h  MBC3+RAM                 FEh  HuC3
13h  MBC3+RAM+BATTERY         FFh  HuC1+RAM+BATTERY
*/
const char	*get_cart_type(uint8_t c)
{
	switch (c)
	{
		case 0x00: return ("ROM ONLY");
		case 0x01: return ("MBC-1");
		case 0x02: return ("MBC-1 SRAM");
		case 0x03: return ("MBC-1 SRAM BATTERY");
		case 0x04: return ("0x04");
		case 0x05: return ("MBC-2");
		case 0x06: return ("MBC-2 BATTERY");
		case 0x07: return ("0x07");
		case 0x08: return ("SRAM");
		case 0x09: return ("SRAM BATTERY");
		case 0x0b: return ("MMM01");
		case 0x0c: return ("MMM01 SRAM");
		case 0x0d: return ("MMM01 SRAM BATTERY");
		case 0x0f: return ("MBC-3 RTC BATTERY");
		case 0x10: return ("MBC-3 RTC SRAM BATTERY");
		case 0x11: return ("MBC-3");
		case 0x12: return ("MBC-3 SRAM");
		case 0x13: return ("MBC-3 SRAM BATTERY");
		case 0x19: return ("MBC-5");
		case 0x1a: return ("MBC-5 SRAM");
		case 0x1b: return ("MBC-5 SRAM BATTERY");
		case 0x1c: return ("MBC-5 Rumble");
		case 0x1d: return ("MBC-5 Rumble SRAM");
		case 0x1e: return ("MBC-5 Rumble SRAM BATTERY");
	}
	return ("ERROR");
}
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
const char	*get_rom_size(uint8_t c, uint8_t cart_type)
{
	int		mbc1 = (cart_type && cart_type < 4);

	switch (c)
	{
		case 0x00: return ("256 KBits (32 KBytes : 0 banks)");
		case 0x01: return ("512 KBits (64 KBytes : 4 banks)");
		case 0x02: return ("1 MBit (128 KBytes : 8 banks)");
		case 0x03: return ("2 MBits (256 KBytes : 16 banks)");
		case 0x04: return ("4 MBits (512 KBytes : 32 banks)");
		case 0x05: return (!mbc1 ? "8 MBits (1 MByte : 64 banks)" : "8 MBits (1 MByte : 63 banks)");
		case 0x06: return (!mbc1 ? "16 MBits (2 MBytes : 128 banks)" : "16 MBits (2 MBytes : 125 banks)");
		case 0x07: return ("32 MBits (4 MBytes : 256 banks)");
		case 0x08: return ("64 MBits (8 MBytes : 512 banks)");
	}
	return ("ERROR");
}
/*
00h - None
01h - 2 KBytes ??
02h - 8 Kbytes
03h - 32 KBytes (4 banks of 8KBytes each)
04h - 128 KBytes (16 banks of 8KBytes each)
05h - 64 KBytes (8 banks of 8KBytes each)
*/
const char	*get_ram_size(uint8_t c)
{
	switch (c)
	{
		case 0x00: return ("No RAM");
		//case 0x01: return ("16 KBits (2 Kbyte)");
		case 0x02: return ("64 KBits (8 KBytes)");
		case 0x03: return ("256 KBits (32 KBytes : 4 banks of 8 KBytes)");
		case 0x04: return ("1 MBit (128 KBytes : 16 banks of 8 KBytes)");
		case 0x05: return ("512 KBits (64 KBytes : 8 banks of 8 Kbytes)");
	}
	return ("ERROR");
}

const char	*get_destination(uint8_t c)
{
	switch (c)
	{
		case 0x00: return ("Japan");
		case 0x01: return ("Other than Japan");
	}
	return ("ERROR");
}


static void read_cartridge_info(FILE *file)
{
	cart_info_t	info;
	uint8_t		buf[16];
	uint8_t		cart_type = 0;

	fread(&info, 2, 1, file);

	puts("\ncartridge:");

	if (info.program_start)
	{
		fread(buf, 1, 2, file);
		printf("  > program_start = 0x%x%x\n", buf[1], buf[0]);
	}
	if (info.game_title)
	{
		fread(buf, 1, 11, file);
		printf("  > game_title = \"%.11s\"\n", buf);
	}
	if (info.game_code)
	{
		fread(buf, 1, 4, file);
		printf("  > game_code = \"%.4s\"\n", buf);
	}
	if (info.cgb_support)
	{
		fread(buf, 1, 1, file);
		printf("  > CGB support = \"%s\"\n", get_cgb_support(*buf));
	}
	if (info.maker_code)
	{
		fread(buf, 1, 2, file);
		printf("  > game_code = \"%.2s\"\n", buf);
	}
	if (info.sgb_support)
	{
		fread(buf, 1, 1, file);
		printf("  > SGB support = \"%s\"\n", get_sgb_support(*buf));
	}
	if (info.cart_type)
	{
		fread(&cart_type, 1, 1, file);
		printf("  > cartridge_type = \"%s\"\n", get_cart_type(cart_type));
	}
	if (info.rom_size)
	{
		fread(buf, 1, 1, file);
		printf("  > rom_size = \"%s\"\n", get_rom_size(*buf, cart_type));
	}
	if (info.ram_size)
	{
		fread(buf, 1, 1, file);
		printf("  > ram_size = \"%s\"\n", get_ram_size(*buf));
	}
	if (info.destination)
	{
		fread(buf, 1, 1, file);
		printf("  > destination = \"%s\"\n", get_destination(*buf));
	}
	if (info.version)
	{
		fread(buf, 1, 1, file);
		printf("  > version = %hhu\n", *buf);
	}
}

#define VAR			0x11
#define LABEL		0x21
#define MEMBLOCK	0x02

static const char	*get_symbol_type(uint32_t type)
{
	switch (type)
	{
		case LABEL:		return ("label");
		case VAR:		return ("variable");
		case MEMBLOCK:	return ("memblock");
	}
	return ("ERROR");
}

static void	read_internal_symbols(FILE *file)
{
	char		buf[128];
	uint32_t	tmp;
	uint32_t	i = 0;

	puts("\ninternal symbols");
	while (1)
	{
		fread(buf + i, 1, 1, file);
		if (buf[i] == 0)
			break;
		if (i == 127) {
			fprintf(stderr, "name length > 128: exit()\n");
			exit(1);
		}
		i++;
	}
	fread(&tmp, sizeof(uint32_t), 1, file);
	printf("  > name = \"%s\", type = %s, ", buf, get_symbol_type(tmp));

	if (tmp == VAR)
	{
		uint32_t	quantity;
		uint32_t	*values;

		fread(&quantity, sizeof(uint32_t), 1, file);
		values = malloc(quantity * sizeof(uint32_t));
		fread(values, sizeof(uint32_t), quantity, file);
		i = 0;
		while (1)
		{
			fread(buf + i, 1, 1, file);
			if (buf[i] == 0)
				break;
			if (i == 127) {
				fprintf(stderr, "name length > 128: exit()\n");
				exit(1);
			}
			i++;
		}
		fread(&tmp, sizeof(uint32_t), 1, file);
		printf("size = %u, block = \"%s\", pos = 0x%x(%u)", tmp, buf, );
	}
	else if (tmp == LABEL)
	{
		fread(&tmp, sizeof(uint32_t), 1, file);
		printf("addr = 0x%x\n", tmp);
	}
	else if (tmp == MEMBLOCK)
	{
	}
	else
	{
		fprintf(stderr, "unknon type: exit()\n");
		exit(1);
	}

}
static void	read_external_symbols(FILE *file)
{
}
static void	print_code(FILE *file, uint32_t length)
{
}

int		main(int argc, char *argv[])
{
	uint32_t	header[4];
	FILE		*file;
	
	if (argc != 2)
	{
		fprintf(stderr, "%s \"file_name\"\n", argv[0]);
		return (1);
	}

	file = fopen(argv[1], "r");
	fread(header, sizeof(uint32_t), 4, file);

	printf("total length = %u bytes (+ header 16 bytes)\n"
			"cartridge_info length   = %u bytes\n"
			"internal symbols length = %u bytes\n"
			"external symbols length = %u bytes\n"
			"code length             = %u bytes\n",
			header[0],
			header[1],
			header[2],
			header[0] - (header[1] + header[2]),
			header[3]
	);

	if (header[1])
		read_cartridge_info(file);
	if (header[2])
		read_internal_symbols(file);
	if (header[0] - (header[1] + header[2]))
		read_external_symbols(file);
	if (header[3])
		print_code(file, header[3]);

	fclose(file);
	return (0);
}
