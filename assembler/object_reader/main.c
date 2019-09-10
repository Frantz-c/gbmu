/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   main.c                                           .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <fcordon@le-101.fr>                +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/08/31 21:59:27 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/10 20:08:19 by fcordon     ###    #+. /#+    ###.fr     */
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

char	*op[] = {
	"nop", //0x00
	"ld   BC, *",
	"ld   (BC), A",
	"inc  BC",
	"inc  B",
	"dec  B",
	"ld   B, *",
	"rlca",
	"ld   (*), SP", //0x08
	"add  HL, BC",
	"ld   A, (BC)",
	"dec  BC",
	"inc  C",
	"dec  C",
	"ld   C, *",
	"rrca",
	"stop", //0x10
	"ld   DE, *",
	"ld   (DE), A",
	"inc  DE",
	"inc  D",
	"dec  D",
	"ld   D, *",
	"rla",
	"jr   *", //0x18
	"add  HL, DE",
	"ld   A, (DE)",
	"dec  DE",
	"inc  E",
	"dec  E",
	"ld   E, *",
	"rra",
	"jr   NZ, *", //0x20   alt="jrnz   *"
	"ld   HL, *",
	"ld   (HL++), A",
	"inc  HL",
	"inc  H",
	"dec  H",
	"ld   H, *",
	"daa",
	"jr   Z, *", //0x28   alt="jrz   *"
	"add  HL, HL",
	"ld   A, (HL++)",
	"dec  HL",
	"inc  L",
	"dec  L",
	"ld   L, *",
	"cpl",
	"jr   NC, *", //0x30   alt="jrnc   *"
	"ld   SP, *",
	"ld   (HL--), A",
	"inc  SP",
	"inc  (HL)",
	"dec  (HL)",
	"ld   (HL), *",
	"scf",
	"jr   C, *", //0x38   alt="jrc   *"
	"add  HL, SP",
	"ld   A, (HL--)",
	"dec  SP",
	"inc  A",
	"dec  A",
	"ld   A, *",
	"ccf",
	"ld   B, B", //0x40
	"ld   B, C",
	"ld   B, D",
	"ld   B, E",
	"ld   B, H",
	"ld   B, L",
	"ld   B, (HL)",
	"ld   B, A",
	"ld   C, B", //0x48
	"ld   C, C",
	"ld   C, D",
	"ld   C, E",
	"ld   C, H",
	"ld   C, L",
	"ld   C, (HL)",
	"ld   C, A",
	"ld   D, B", //0x50
	"ld   D, C",
	"ld   D, D",
	"ld   D, E",
	"ld   D, H",
	"ld   D, L",
	"ld   D, (HL)",
	"ld   D, A",
	"ld   E, B", //0x58
	"ld   E, C",
	"ld   E, D",
	"ld   E, E",
	"ld   E, H",
	"ld   E, L",
	"ld   E, (HL)",
	"ld   E, A",
	"ld   H, B", //0x60
	"ld   H, C",
	"ld   H, D",
	"ld   H, E",
	"ld   H, H",
	"ld   H, L",
	"ld   H, (HL)",
	"ld   H, A",
	"ld   L, B", //0x68
	"ld   L, C",
	"ld   L, D",
	"ld   L, E",
	"ld   L, H",
	"ld   L, L",
	"ld   L, (HL)",
	"ld   L, A",
	"ld   (HL), B", //0x70
	"ld   (HL), C",
	"ld   (HL), D",
	"ld   (HL), E",
	"ld   (HL), H",
	"ld   (HL), L",
	"halt",
	"ld   (HL), A",
	"ld   A, B", //0x78
	"ld   A, C",
	"ld   A, D",
	"ld   A, E",
	"ld   A, H",
	"ld   A, L",
	"ld   A, (HL)",
	"ld   A, A",
	"add  B", //0x80
	"add  C",
	"add  D",
	"add  E",
	"add  H",
	"add  L",
	"add  (HL)",
	"add  A",
	"adc  B", //0x88
	"adc  C",
	"adc  D",
	"adc  E",
	"adc  H",
	"adc  L",
	"adc  (HL)",
	"adc  A",
	"sub  B", //0x90
	"sub  C",
	"sub  D",
	"sub  E",
	"sub  H",
	"sub  L",
	"sub  (HL)",
	"sub  A",
	"sbc  B", //0x98
	"sbc  C",
	"sbc  D",
	"sbc  E",
	"sbc  H",
	"sbc  L",
	"sbc  (HL)",
	"sbc  A",
	"and  B", //0xa0
	"and  C",
	"and  D",
	"and  E",
	"and  H",
	"and  L",
	"and  (HL)",
	"and  A",
	"xor  B", //0xa8
	"xor  C",
	"xor  D",
	"xor  E",
	"xor  H",
	"xor  L",
	"xor  (HL)",
	"xor  A",
	"or   B", //0xb0
	"or   C",
	"or   D",
	"or   E",
	"or   H",
	"or   L",
	"or   (HL)",
	"or   A",
	"cp   B", //0xb8   alt="cmp   reg"
	"cp   C",
	"cp   D",
	"cp   E",
	"cp   H",
	"cp   L",
	"cp   (HL)",
	"cp   A",
	"ret  NZ", //0xc0
	"pop  BC",
	"jp   NZ, *", // alt="jpnz   *"
	"jp   *",
	"call NZ, *", // alt="callnz   *"
	"push BC",
	"add  A, *",
	"rst  0x00",
	"ret  Z", //0xc8   alt="retz"
	"ret",
	"jp   Z, *", // alt="jpz   *"
	NULL, // PREFIX 0xCB
	"call Z, *", // alt="callz   *"
	"call *",
	"adc  A, *",
	"rst  0x08",
	"ret  NC", //0xd0   alt="retnc"
	"pop  DE",
	"jp   NC, *", // alt="jpnc   *"
	NULL, //unused
	"call NC, *", // alt="callnc   *"
	"push DE",
	"sub  *",
	"rst  0x10",
	"ret  C", //0xd8   alt="retc"
	"reti",
	"jp   C, *", // alt="jpc   *"
	NULL, //unused
	"call C, *", // alt="callc   *"
	NULL, //unused
	"sbc  A, *",
	"rst  0x18",
	"ld   (0xff00+*), A", //0xe0
	"pop  HL",
	"ld   (0xff00+C), A", /**** 2 bytes ?????  */
	NULL,
	NULL,
	"push HL",
	"and  *",
	"rst  0x20",
	"add  SP, *", //0xe8
	"jp   (HL)",
	"ld   (*), A",
	NULL,
	NULL,
	NULL,
	"xor  *",
	"rst  0x28",
	"ld   A, (0xff00+*)", //0xf0
	"pop  AF",
	"ld   A, (0xff00+C)", /**** 2 bytes ?????  */
	"di",
	NULL,
	"push AF",
	"or   *",
	"rst  0x30",
	"ldhl SP, *", //0xf8  (== "ld  HL, SP+*")
	"ld   SP, HL",
	"ld   A, (*)",
	"ei",
	NULL,
	NULL,
	"cp   *", // alt="cmp   *"
	"rst  0x38"
};

char	*cb_op[] = {
	"rlc  B\n", //0x00
	"rlc  C\n",
	"rlc  D\n",
	"rlc  E\n",
	"rlc  H\n",
	"rlc  L\n",
	"rlc  (HL)\n",
	"rlc  A\n",
	"rrc  B\n", //0x08
	"rrc  C\n",
	"rrc  D\n",
	"rrc  E\n",
	"rrc  H\n",
	"rrc  L\n",
	"rrc  (HL)\n",
	"rrc  A\n",
	"rl   B\n", //0x10
	"rl   C\n",
	"rl   D\n",
	"rl   E\n",
	"rl   H\n",
	"rl   L\n",
	"rl   (HL)\n",
	"rl   A\n",
	"rr   B\n", //0x18
	"rr   C\n",
	"rr   D\n",
	"rr   E\n",
	"rr   H\n",
	"rr   L\n",
	"rr   (HL)\n",
	"rr   A\n",
	"sla  B\n", //0x20
	"sla  C\n",
	"sla  D\n",
	"sla  E\n",
	"sla  H\n",
	"sla  L\n",
	"sla  (HL)\n",
	"sla  A\n",
	"sra  B\n", //0x28
	"sra  C\n",
	"sra  D\n",
	"sra  E\n",
	"sra  H\n",
	"sra  L\n",
	"sra  (HL)\n",
	"sra  A\n",
	"swap B\n", //0x30
	"swap C\n",
	"swap D\n",
	"swap E\n",
	"swap H\n",
	"swap L\n",
	"swap (HL)\n",
	"swap A\n",
	"srl  B\n", //0x38
	"srl  C\n",
	"srl  D\n",
	"srl  E\n",
	"srl  H\n",
	"srl  L\n",
	"srl  (HL)\n",
	"srl  A\n",
	"bit  0, B\n", //0x40
	"bit  0, C\n",
	"bit  0, D\n",
	"bit  0, E\n",
	"bit  0, H\n",
	"bit  0, L\n",
	"bit  0, (HL)\n",
	"bit  0, A\n",
	"bit  1, B\n", //0x48
	"bit  1, C\n",
	"bit  1, D\n",
	"bit  1, E\n",
	"bit  1, H\n",
	"bit  1, L\n",
	"bit  1, (HL)\n",
	"bit  1, A\n",
	"bit  2, B\n", //0x50
	"bit  2, C\n",
	"bit  2, D\n",
	"bit  2, E\n",
	"bit  2, H\n",
	"bit  2, L\n",
	"bit  2, (HL)\n",
	"bit  2, A\n",
	"bit  3, B\n", //0x58
	"bit  3, C\n",
	"bit  3, D\n",
	"bit  3, E\n",
	"bit  3, H\n",
	"bit  3, L\n",
	"bit  3, (HL)\n",
	"bit  3, A\n",
	"bit  4, B\n", //0x60
	"bit  4, C\n",
	"bit  4, D\n",
	"bit  4, E\n",
	"bit  4, H\n",
	"bit  4, L\n",
	"bit  4, (HL)\n",
	"bit  4, A\n",
	"bit  5, B\n", //0x68
	"bit  5, C\n",
	"bit  5, D\n",
	"bit  5, E\n",
	"bit  5, H\n",
	"bit  5, L\n",
	"bit  5, (HL)\n",
	"bit  5, A\n",
	"bit  6, B\n", //0x70
	"bit  6, C\n",
	"bit  6, D\n",
	"bit  6, E\n",
	"bit  6, H\n",
	"bit  6, L\n",
	"bit  6, (HL)\n",
	"bit  6, A\n",
	"bit  7, B\n", //0x78
	"bit  7, C\n",
	"bit  7, D\n",
	"bit  7, E\n",
	"bit  7, H\n",
	"bit  7, L\n",
	"bit  7, (HL)\n",
	"bit  7, A\n",
	"res  0, B\n", //0x80
	"res  0, C\n",
	"res  0, D\n",
	"res  0, E\n",
	"res  0, H\n",
	"res  0, L\n",
	"res  0, (HL)\n",
	"res  0, A\n",
	"res  1, B\n", //0x88
	"res  1, C\n",
	"res  1, D\n",
	"res  1, E\n",
	"res  1, H\n",
	"res  1, L\n",
	"res  1, (HL)\n",
	"res  1, A\n",
	"res  2, B\n", //0x90
	"res  2, C\n",
	"res  2, D\n",
	"res  2, E\n",
	"res  2, H\n",
	"res  2, L\n",
	"res  2, (HL)\n",
	"res  2, A\n",
	"res  3, B\n", //0x98
	"res  3, C\n",
	"res  3, D\n",
	"res  3, E\n",
	"res  3, H\n",
	"res  3, L\n",
	"res  3, (HL)\n",
	"res  3, A\n",
	"res  4, B\n", //0xa0
	"res  4, C\n",
	"res  4, D\n",
	"res  4, E\n",
	"res  4, H\n",
	"res  4, L\n",
	"res  4, (HL)\n",
	"res  4, A\n",
	"res  5, B\n", //0xa8
	"res  5, C\n",
	"res  5, D\n",
	"res  5, E\n",
	"res  5, H\n",
	"res  5, L\n",
	"res  5, (HL)\n",
	"res  5, A\n",
	"res  6, B\n", //0xb0
	"res  6, C\n",
	"res  6, D\n",
	"res  6, E\n",
	"res  6, H\n",
	"res  6, L\n",
	"res  6, (HL)\n",
	"res  6, A\n",
	"res  7, B\n", //0xb8
	"res  7, C\n",
	"res  7, D\n",
	"res  7, E\n",
	"res  7, H\n",
	"res  7, L\n",
	"res  7, (HL)\n",
	"res  7, A\n",
	"set  0, B\n", //0xc0
	"set  0, C\n",
	"set  0, D\n",
	"set  0, E\n",
	"set  0, H\n",
	"set  0, L\n",
	"set  0, (HL)\n",
	"set  0, A\n",
	"set  1, B\n", //0xc8
	"set  1, C\n",
	"set  1, D\n",
	"set  1, E\n",
	"set  1, H\n",
	"set  1, L\n",
	"set  1, (HL)\n",
	"set  1, A\n",
	"set  2, B\n", //0xd0
	"set  2, C\n",
	"set  2, D\n",
	"set  2, E\n",
	"set  2, H\n",
	"set  2, L\n",
	"set  2, (HL)\n",
	"set  2, A\n",
	"set  3, B\n", //0xd8
	"set  3, C\n",
	"set  3, D\n",
	"set  3, E\n",
	"set  3, H\n",
	"set  3, L\n",
	"set  3, (HL)\n",
	"set  3, A\n",
	"set  4, B\n", //0xe0
	"set  4, C\n",
	"set  4, D\n",
	"set  4, E\n",
	"set  4, H\n",
	"set  4, L\n",
	"set  4, (HL)\n",
	"set  4, A\n",
	"set  5, B\n", //0xe8
	"set  5, C\n",
	"set  5, D\n",
	"set  5, E\n",
	"set  5, H\n",
	"set  5, L\n",
	"set  5, (HL)\n",
	"set  5, A\n",
	"set  6, B\n", //0xf0
	"set  6, C\n",
	"set  6, D\n",
	"set  6, E\n",
	"set  6, H\n",
	"set  6, L\n",
	"set  6, (HL)\n",
	"set  6, A\n",
	"set  7, B\n", //0xf8
	"set  7, C\n",
	"set  7, D\n",
	"set  7, E\n",
	"set  7, H\n",
	"set  7, L\n",
	"set  7, (HL)\n",
	"set  7, A\n"
};

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
		case 1:			return ("var or label");
	}
	return ("ERROR");
}

static void	read_internal_symbols(FILE *file, uint32_t size)
{
	char		buf[128];
	uint32_t	tmp;
	uint32_t	i;
	uint32_t	cur = 0;

	puts("\nlocal symbols");
	do
	{
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
		cur += (i + 5);
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
			cur += (i + 9 + (quantity * 4));
			printf("size = %u, block = \"%s\", pos = {", tmp, buf);

			for (i = 0; i < quantity - 1; i++)
				printf("%u,", values[i]);
			printf("%u}\n", values[i]);
			free(values);
		}
		else if (tmp == LABEL)
		{
			fread(&tmp, sizeof(uint32_t), 1, file);
			cur += 4;
			printf("addr = 0x%x\n", tmp);
		}
		else if (tmp == MEMBLOCK)
		{
			uint32_t	end;

			fread(&tmp, sizeof(uint32_t), 1, file);
			fread(&end, sizeof(uint32_t), 1, file);
			cur += 8;
			printf("start = 0x%x, end = 0x%x\n", tmp, end);
		}
		else
		{
			fprintf(stderr, "unknown type: exit()\n");
			exit(1);
		}
	}
	while (cur < size);
}

static void	read_external_symbols(FILE *file, uint32_t size)
{
	char		buf[128];
	uint32_t	type, quantity, *values;
	uint32_t	i = 0;
	uint32_t	cur = 0;

	puts("\nextern symbols");
	do
	{
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
		cur += (i + 1);
		fread(&type, sizeof(uint32_t), 1, file);
		fread(&quantity, sizeof(uint32_t), 1, file);
		values = malloc(quantity * sizeof(uint32_t));
		fread(values, sizeof(uint32_t), quantity, file);
		printf("  > name = \"%s\", type = %s, pos = {", buf, get_symbol_type(type));

		for (i = 0; i < quantity - 1; i++)
			printf("%u,", values[i]);
		printf("%u}\n", values[i]);
		cur += 8 + (quantity * 4);
		free(values);
		printf("cur = %u, size = %u\n", cur, size);
	}
	while (cur < size);

}

static void	read_code(FILE *file, uint32_t length)
{
		static const uint8_t	inst_length[256] = {
	//	0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F
		1,3,1,1,1,1,2,1,3,1,1,1,1,1,2,1,
		1,3,1,1,1,1,2,1,2,1,1,1,1,1,2,1,
		2,3,1,1,1,1,2,1,2,1,1,1,1,1,2,1,
		2,3,1,1,1,1,2,1,2,1,1,1,1,1,2,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,3,3,3,1,2,1,1,1,3,2,3,3,2,1,
		1,1,3,0,3,1,2,1,1,1,3,0,3,0,2,1,
		2,1,1,0,0,1,2,1,2,1,3,0,0,0,2,1,
		2,1,1,1,0,1,2,1,2,1,3,1,0,0,2,1
	};
	uint8_t		*buf;
	uint32_t	start_addr, section_length;
	uint32_t	readl = 0;

	puts("\ncode section");
	while (readl < length)
	{
		fread(&start_addr, sizeof(uint32_t), 1, file);
		fread(&section_length, sizeof(uint32_t), 1, file);
		buf = malloc(section_length);

		fread(buf, 1, section_length, file);
		readl += 8 + section_length;
		printf("  > addr = 0x%x, length = %u bytes\n  {\n", start_addr, section_length);
		// read buf
		uint32_t i = 0;
		while (i < section_length)
		{
			// .byte
			if (buf[i] == 0xDDu && buf[i + 1] == 0xDDu && buf[i + 2] == 0xDDu)
			{
				printf("    .byte ");
				i += 3;
				uint32_t	byte_length = (buf[i] << 8) | buf[i+1];
				i += 2;
				while (byte_length)
				{
					printf("0x%x ", buf[i++]);
					byte_length--;
				}
				printf("\n");
			}
			else
			{
				if (buf[i] == 0xCBu)
				{
					i++;
					printf("    %s", cb_op[buf[i]]);
					i++;
				}
				else if (op[buf[i]] != NULL)
				{
					uint8_t	instl = inst_length[buf[i]];

					if (instl == 1)
						printf("    %s\n", op[buf[i]]);
					else if (instl == 2)
						printf("    %s (value = 0x%0.2hhx, sign = '%c')\n", op[buf[i]], buf[i+1], buf[i+2]);
					else
						printf("    %s (value = 0x%0.2hhx%0.2hhx, sign = '%c')\n", op[buf[i]], buf[i+2], buf[i+1], buf[i+3]);
					i += instl + (instl != 1);
				}
				else
				{
					printf("    BAD_OPCODE 0x%x\n", buf[i]);
					i++;
				}
			}
		}
		free(buf);
	}
	puts("  }");
	if (fread(&start_addr, 1, 1, file) == 1)
	{
		fprintf(stderr, "too longer file ! exit()\n");
		exit(1);
	}
}

int		main(int argc, char *argv[])
{
	uint32_t	header[4] = {0};
	FILE		*file;
	
	if (argc != 2)
	{
		fprintf(stderr, "%s \"file_name\"\n", argv[0]);
		return (1);
	}

	file = fopen(argv[1], "r");
	if (file == NULL)
	{
		fprintf(stderr, "can't open file...\n");
		return (1);
	}
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
		read_internal_symbols(file, header[2]);
	if (header[0] - (header[1] + header[2]))
		read_external_symbols(file, (header[0] - (header[1] + header[2])));
	if (header[3])
		read_code(file, header[3]);

	fclose(file);
	return (0);
}
