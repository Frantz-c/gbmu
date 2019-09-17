/* ************************************************************************** */
/* LE - / */
/* / */
/* get_inst_str.c .:: .:/ . .:: */
/* +:+:+ +: +: +:+:+ */
/* By: fcordon <mhouppin@le-101.fr> +:+ +: +: +:+ */
/* #+# #+ #+ #+# */
/* Created: 2019/09/17 16:27:39 by fcordon #+# ## ## #+# */
/* Updated: 2019/09/17 16:31:48 by fcordon ### #+. /#+ ###.fr */
/* / */
/* / */
/* ************************************************************************** */

#include "std_includes.h"

static const char *op[] =
{
	"nop", //0x00
	"ld BC, *",
	"ld [BC], A",
	"inc BC",
	"inc B",
	"dec B",
	"ld B, *",
	"rlca",
	"ld [*], SP", //0x08
	"add HL, BC",
	"ld A, [BC]",
	"dec BC",
	"inc C",
	"dec C",
	"ld C, *",
	"rrca",
	"stop", //0x10
	"ld DE, *",
	"ld [DE], A",
	"inc DE",
	"inc D",
	"dec D",
	"ld D, *",
	"rla",
	"jr *", //0x18
	"add HL, DE",
	"ld A, [DE]",
	"dec DE",
	"inc E",
	"dec E",
	"ld E, *",
	"rra",
	"jr NZ, *", //0x20 alt="jrnz *"
	"ld HL, *",
	"ld [HL++], A",
	"inc HL",
	"inc H",
	"dec H",
	"ld H, *",
	"daa",
	"jr Z, *", //0x28 alt="jrz *"
	"add HL, HL",
	"ld A, [HL++]",
	"dec HL",
	"inc L",
	"dec L",
	"ld L, *",
	"cpl",
	"jr NC, *", //0x30 alt="jrnc *"
	"ld SP, *",
	"ld [HL--], A",
	"inc SP",
	"inc [HL]",
	"dec [HL]",
	"ld [HL], *",
	"scf",
	"jr C, *", //0x38 alt="jrc *"
	"add HL, SP",
	"ld A, [HL--]",
	"dec SP",
	"inc A",
	"dec A",
	"ld A, *",
	"ccf",
	"ld B, B", //0x40
	"ld B, C",
	"ld B, D",
	"ld B, E",
	"ld B, H",
	"ld B, L",
	"ld B, [HL]",
	"ld B, A",
	"ld C, B", //0x48
	"ld C, C",
	"ld C, D",
	"ld C, E",
	"ld C, H",
	"ld C, L",
	"ld C, [HL]",
	"ld C, A",
	"ld D, B", //0x50
	"ld D, C",
	"ld D, D",
	"ld D, E",
	"ld D, H",
	"ld D, L",
	"ld D, [HL]",
	"ld D, A",
	"ld E, B", //0x58
	"ld E, C",
	"ld E, D",
	"ld E, E",
	"ld E, H",
	"ld E, L",
	"ld E, [HL]",
	"ld E, A",
	"ld H, B", //0x60
	"ld H, C",
	"ld H, D",
	"ld H, E",
	"ld H, H",
	"ld H, L",
	"ld H, [HL]",
	"ld H, A",
	"ld L, B", //0x68
	"ld L, C",
	"ld L, D",
	"ld L, E",
	"ld L, H",
	"ld L, L",
	"ld L, [HL]",
	"ld L, A",
	"ld [HL], B", //0x70
	"ld [HL], C",
	"ld [HL], D",
	"ld [HL], E",
	"ld [HL], H",
	"ld [HL], L",
	"halt",
	"ld [HL], A",
	"ld A, B", //0x78
	"ld A, C",
	"ld A, D",
	"ld A, E",
	"ld A, H",
	"ld A, L",
	"ld A, [HL]",
	"ld A, A",
	"add B", //0x80
	"add C",
	"add D",
	"add E",
	"add H",
	"add L",
	"add [HL]",
	"add A",
	"adc B", //0x88
	"adc C",
	"adc D",
	"adc E",
	"adc H",
	"adc L",
	"adc [HL]",
	"adc A",
	"sub B", //0x90
	"sub C",
	"sub D",
	"sub E",
	"sub H",
	"sub L",
	"sub [HL]",
	"sub A",
	"sbc B", //0x98
	"sbc C",
	"sbc D",
	"sbc E",
	"sbc H",
	"sbc L",
	"sbc [HL]",
	"sbc A",
	"and B", //0xa0
	"and C",
	"and D",
	"and E",
	"and H",
	"and L",
	"and [HL]",
	"and A",
	"xor B", //0xa8
	"xor C",
	"xor D",
	"xor E",
	"xor H",
	"xor L",
	"xor [HL]",
	"xor A",
	"or B", //0xb0
	"or C",
	"or D",
	"or E",
	"or H",
	"or L",
	"or [HL]",
	"or A",
	"cp B", //0xb8 alt="cmp reg"
	"cp C",
	"cp D",
	"cp E",
	"cp H",
	"cp L",
	"cp [HL]",
	"cp A",
	"ret NZ", //0xc0
	"pop BC",
	"jp NZ, *", // alt="jpnz *"
	"jp *",
	"call NZ, *", // alt="callnz *"
	"push BC",
	"add A, *",
	"rst 0x00",
	"ret Z", //0xc8 alt="retz"
	"ret",
	"jp Z, *", // alt="jpz *"
	NULL, // PREFIX 0xCB
	"call Z, *", // alt="callz *"
	"call *",
	"adc A, *",
	"rst 0x08",
	"ret NC", //0xd0 alt="retnc"
	"pop DE",
	"jp NC, *", // alt="jpnc *"
	NULL, //unused
	"call NC, *", // alt="callnc *"
	"push DE",
	"sub *",
	"rst 0x10",
	"ret C", //0xd8 alt="retc"
	"reti",
	"jp C, *", // alt="jpc *"
	NULL, //unused
	"call C, *", // alt="callc *"
	NULL, //unused
	"sbc A, *",
	"rst 0x18",
	"ld [0xff00+*], A", //0xe0
	"pop HL",
	"ld [0xff00+C], A", /**** 2 bytes ????? */
	NULL,
	NULL,
	"push HL",
	"and *",
	"rst 0x20",
	"add SP, *", //0xe8
	"jp [HL]",
	"ld [*], A",
	NULL,
	NULL,
	NULL,
	"xor *",
	"rst 0x28",
	"ld A, [0xff00+*]", //0xf0
	"pop AF",
	"ld A, [0xff00+C]", /**** 2 bytes ????? */
	"di",
	NULL,
	"push AF",
	"or *",
	"rst 0x30",
	"ldhl SP, *", //0xf8 [== "ld HL, SP+*"]
	"ld SP, HL",
	"ld A, [*]",
	"ei",
	NULL,
	NULL,
	"cp *", // alt="cmp *"
	"rst 0x38"
};

const char* __attribute__((always_inline))
	get_inst_str(uint8_t opcode)
{
	return (op[opcode]);
}
