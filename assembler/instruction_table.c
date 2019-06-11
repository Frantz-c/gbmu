/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   instruction_table.c                              .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/21 16:29:29 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/06/10 16:49:50 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

// http://bgb.bircd.org/pandocs.htm#thecartridgeheader

#define	GB_CLOCK			4190000
#define GB_MACHINE_CYCLE	1050000	
#define GBC_CLOCK			8000000
#define GBC_MACHINE_CYCLE	2000000 //???
#define NITENDO_LOGO		"\xCE\xED\x66\x66\xCC\x0D\x00\x0B\x03\x73\x00\x83"\
							"\x00\x0C\x00\x0D\x00\x08\x11\x1F\x88\x89\x00\x0E"\
							"\xDC\xCC\x6E\xE6\xDD\xDD\xD9\x99\xBB\xBB\x67\x63"\
							"\x6E\x0E\xEC\xCC\xDD\xDC\x99\x9F\xBB\xB9\x33\x3E"
//#define GB

enum	e_param
{
	NONE, A, B, C, D, E, F, H, L,
	SP, PC,
	AF, BC, DE, HL,
	IMM8, IMM16,
	ADDR8, ADDR16,
	FLAG,
	HL_ADDR, BC_ADDR, DE_ADDR,
	C_FF00,
	__NZ__, __Z__, __NC__, __C__,
	__00H__, __08H__, __10H__, __18H__, __20H__, __28H__, __30H__, __38H__,
	____
};

struct	s_spec
{
	int			opcode;
	t_param		param1;
	t_param		param2;
	int			cycles;
};

struct	s_inst
{
	const char			*lname; // lowercase
	const char			*uname; // uppercase
	struct s_inst_info	*spec;	// t_spec array
};

typedef struct s_inst	t_inst;
typedef struct s_spec	t_spec;
typedef enum e_param	t_param;

// ADDR16 = little endian order 0xff00 = ($00ff)
// X_FF00 = (X)
// HL_ADDR = "(HL)"

t_inst	str_inst_info[] = {
	{"ld", "LD", ld_spec},
	{"ldd", "LDD", ldd_spec},
	{"ldi", "LDI", ldi_spec},
	{"ldhl", "LDHL", ldhl_spec},
	{"push", "PUSH", push_spec},
	{"pop", "POP", pop_spec},
	{"add", "ADD", add_spec},
	{"adc", "ADC", adc_spec},
	{"sub", "SUB", sub_spec},
	{"sbc", "SBC", sbc_spec},
	{"and", "AND", and_spec},
	{"or", "OR", or_spec},
	{"xor", "XOR", xor_spec},
	{"cp", "CP", cp_spec},
	{"inc", "INC", inc_spec},
	{"dec", "DEC", dec_spec},
	{"swap", "SWAP", swap_spec},
	{"daa", "DAA", daa_spec}, //??? decimal adjust A
	{"cpl", "CPL", cpl_spec}, //??? complement A register
	{"ccf", "CCF", ccf_spec},
	{"scf", "SCF", scf_spec},
	{"nop", "NOP", nop_spec},
	{"halt", "HALT", halt_spec},
	{"stop", "STOP", stop_spec},
	{"di", "DI", di_spec},
	{"ei", "EI", ei_spec},
	{"rlca", "RLCA", rlca_spec},
	{"rla", "RLA", rla_spec},
	{"rrca", "RRCA", rrca_spec},
	{"rra", "RRA", rra_spec},
	{"rlc", "RLC", rlc_spec},
	{"rl", "RL", rl_spec},
	{"rrc", "RRC", rrc_spec},
	{"rr", "RR", rr_spec},
	{"sla", "SLA", sla_spec},
	{"sra", "SRA", sra_spec},
	{"srl", "SRL", srl_spec},
	{"bit", "BIT", bit_spec},
	{"set", "SET", set_spec},
	{"res", "RES", res_spec},
	{"jp", "JP", jp_spec},
	{"jr", "JR", jr_spec},
	{"call", "CALL", call_spec},
	{"rst", "RST", rst_spec},
	{"ret", "RET", ret_spec},
	{"reti", "RETI", reti_spec},
	// customs
	{"jpnz", "JPNZ", custom_jpnz_spec},
	{"jpz", "JPZ", custom_jpz_spec},
	{"jpnc", "JPNC", custom_jpnc_spec},
	{"jpc", "JPC", custom_jpc_spec},
	{"jrnz", "JRNZ", custom_jrnz_spec},
	{"jrz", "JRZ", custom_jrz_spec},
	{"jrnc", "JRNC", custom_jrnc_spec},
	{"jrc", "JRC", custom_jrc_spec},
	{"cmp", "CMP", cp_spec},
	{"mov", "MOV", ld_spec},
	{"callnz", "CALLNZ", custom_callnz_spec},
	{"callz", "CALLZ", custom_callz_spec},
	{"callnc", "CALLNC", custom_callnc_spec},
	{"callc", "CALLC", custom_callc_spec},
	{"retnz", "RETNZ", custom_ret_spec},
	{"retz", "RETZ", custom__spec},
	{"retc", "RETC", custom__spec},
	{"retnc", "RETNC", custom__spec},
	{NULL, NULL, NULL},
};

// AJOUTER les alternatives a ldi dans ld (pareil pour ldd)

/*
 *  CUSTOM SPECS START
 */

t_spec	custom_jrnz_spec[] = {
	{0x20, IMM8, 0, 12},
	{-1, 0, 0, 0};
};

t_spec	custom_jrz_spec[] = {
	{0x28, IMM8, 0, 12},
	{-1, 0, 0, 0};
};

t_spec	custom_jrnc_spec[] = {
	{0x30, IMM8, 0, 12},
	{-1, 0, 0, 0};
};

t_spec	custom_jrc_spec[] = {
	{0x38, IMM8, 0, 12},
	{-1, 0, 0, 0};
};

t_spec	custom_jpnz_spec[] = {
	{0xc2, IMM6, 0, 12},
	{-1, 0, 0, 0};
};

t_spec	custom_jpz_spec[] = {
	{0xca, IMM16, 0, 12},
	{-1, 0, 0, 0};
};

t_spec	custom_jpnc_spec[] = {
	{0xd2, IMM16, 0, 12},
	{-1, 0, 0, 0};
};

t_spec	custom_jpc_spec[] = {
	{0xda, IMM16, 0, 12},
	{-1, 0, 0, 0};
};

t_spec	custom_retnz_spec[] = {
	{0xc0, 0, 0, 8},
	{-1, 0, 0, 0}
};

t_spec	custom_retz_spec[] = {
	{0xc8, 0, 0, 8},
	{-1, 0, 0, 0}
};

t_spec	custom_retnc_spec[] = {
	{0xd0, 0, 0, 8},
	{-1, 0, 0, 0}
};

t_spec	custom_retc_spec[] = {
	{0xd8, 0, 0, 8},
	{-1, 0, 0, 0}
};

t_spec	custom_callnz_spec[] = {
	{0xc4, 0, 0, 12},
	{-1, 0, 0, 0}
};

t_spec	custom_callz_spec[] = {
	{0xcc, 0, 0, 12},
	{-1, 0, 0, 0}
};

t_spec	custom_callnc_spec[] = {
	{0xd4, 0, 0, 12},
	{-1, 0, 0, 0}
};

t_spec	custom_callc_spec[] = {
	{0xdc, 0, 0, 12},
	{-1, 0, 0, 0}
};

/*
 * CUSTOM SPECS END
 */



t_spec	reti_spec[] = {
	{0xd9, 0, 0, 8},
	{-1, 0, 0, 0}
};

t_spec	ret_spec[] = {
	{0xc9, 0, 0, 8},
	{0xc0, __NZ__, 0, 8},
	{0xc8, __Z__, 0, 8},
	{0xd0, __NC__, 0, 8},
	{0xd8, __C__, 0, 8},
	{-1, 0, 0, 0}
};

t_spec	rst_spec[] = {
	{0xc7, __00H__, 0, 32},
	{0xcf, __08H__, 0, 32},
	{0xd7, __10H__, 0, 32},
	{0xdf, __18H__, 0, 32},
	{0xe7, __20H__, 0, 32},
	{0xef, __28H__, 0, 32},
	{0xf7, __30H__, 0, 32},
	{0xff, __38H__, 0, 32},
	{-1, 0, 0, 0}
};

t_spec	call_spec[] = {
	{0xcd, IMM16, 0, 12},
	{0xc4, __NZ__, IMM16, 12},
	{0xcc, __Z__, IMM16, 12},
	{0xd4, __NC__, IMM16, 12},
	{0xdc, __C__, IMM16, 12},
	{-1, 0, 0, 0}
};

t_spec	jr_spec[] = {
	{0x18, IMM8, 0, 8},
	{0x20, __NZ__, IMM8, 8},
	{0x28, __Z__, IMM8, 8},
	{0x30, __NC__, IMM8, 8},
	{0x38, __C__, IMM8, 8},
	{-1, 0, 0, 0}
};

t_spec	jp_spec[] = {
	{0xc3, IMM16, 0, 12},
	{0xc2, __NZ__, IMM16, 12},
	{0xca, __Z__, IMM16, 12},
	{0xd2, __NC__, IMM16, 12},
	{0xda, __C__, IMM16, 12},
	{0xe9, HL_ADDR, 0, 4},
	{-1, 0, 0, 0}
};

t_spec	res_spec[] = {
	{0xcb87, IMM8_BIT, A, 8},
	{0xcb80, IMM8_BIT, B, 8},
	{0xcb81, IMM8_BIT, C, 8},
	{0xcb82, IMM8_BIT, D, 8},
	{0xcb83, IMM8_BIT, E, 8},
	{0xcb84, IMM8_BIT, H, 8},
	{0xcb85, IMM8_BIT, L, 8},
	{0xcb86, IMM8_BIT, HL_ADDR, 16},
	{-1, 0, 0, 0}
};

t_spec	set_spec[] = {
	{0xcbc7, IMM8_BIT, A, 8},
	{0xcbc0, IMM8_BIT, B, 8},
	{0xcbc1, IMM8_BIT, C, 8},
	{0xcbc2, IMM8_BIT, D, 8},
	{0xcbc3, IMM8_BIT, E, 8},
	{0xcbc4, IMM8_BIT, H, 8},
	{0xcbc5, IMM8_BIT, L, 8},
	{0xcbc6, IMM8_BIT, HL_ADDR, 16},
	{-1, 0, 0, 0}
};

t_spec	bit_spec[] = {
	{0xcb47, IMM8_BIT, A, 8},
	{0xcb40, IMM8_BIT, B, 8},
	{0xcb41, IMM8_BIT, C, 8},
	{0xcb42, IMM8_BIT, D, 8},
	{0xcb43, IMM8_BIT, E, 8},
	{0xcb44, IMM8_BIT, H, 8},
	{0xcb45, IMM8_BIT, L, 8},
	{0xcb46, IMM8_BIT, HL_ADDR, 16},
	{-1, 0, 0, 0}
};

t_spec	srl_spec[] = {
	{0xcb3f, A, 0, 8},
	{0xcb38, B, 0, 8},
	{0xcb39, C, 0, 8},
	{0xcb3a, D, 0, 8},
	{0xcb3b, E, 0, 8},
	{0xcb3c, H, 0, 8},
	{0xcb3d, L, 0, 8},
	{0xcb3e, HL_ADDR, 0, 16},
	{-1, 0, 0, 0}
};

t_spec	sra_spec[] = {
	{0xcb2f, A, 0, 8},
	{0xcb28, B, 0, 8},
	{0xcb29, C, 0, 8},
	{0xcb2a, D, 0, 8},
	{0xcb2b, E, 0, 8},
	{0xcb2c, H, 0, 8},
	{0xcb2d, L, 0, 8},
	{0xcb2e, HL_ADDR, 0, 16},
	{-1, 0, 0, 0}
};

t_spec	sla_spec[] = {
	{0xcb27, A, 0, 8},
	{0xcb20, B, 0, 8},
	{0xcb21, C, 0, 8},
	{0xcb22, D, 0, 8},
	{0xcb23, E, 0, 8},
	{0xcb24, H, 0, 8},
	{0xcb25, L, 0, 8},
	{0xcb26, HL_ADDR, 0, 16},
	{-1, 0, 0, 0}
};

t_spec	rr_spec[] = {
	{0xcb1f, A, 0, 8},
	{0xcb18, B, 0, 8},
	{0xcb19, C, 0, 8},
	{0xcb1a, D, 0, 8},
	{0xcb1b, E, 0, 8},
	{0xcb1c, H, 0, 8},
	{0xcb1d, L, 0, 8},
	{0xcb1e, HL_ADDR, 0, 16},
	{-1, 0, 0, 0}
};

t_spec	rrc_spec[] = {
	{0xcb0f, A, 0, 8},
	{0xcb08, B, 0, 8},
	{0xcb09, C, 0, 8},
	{0xcb0a, D, 0, 8},
	{0xcb0b, E, 0, 8},
	{0xcb0c, H, 0, 8},
	{0xcb0d, L, 0, 8},
	{0xcb0e, HL_ADDR, 0, 16},
	{-1, 0, 0, 0}
};

t_spec	rl_spec[] = {
	{0xcb17, A, 0, 8},
	{0xcb10, B, 0, 8},
	{0xcb11, C, 0, 8},
	{0xcb12, D, 0, 8},
	{0xcb13, E, 0, 8},
	{0xcb14, H, 0, 8},
	{0xcb15, L, 0, 8},
	{0xcb16, HL_ADDR, 0, 16},
	{-1, 0, 0, 0}
};

t_spec	rlc_spec[] = {
	{0xcb07, A, 0, 8},
	{0xcb00, B, 0, 8},
	{0xcb01, C, 0, 8},
	{0xcb02, D, 0, 8},
	{0xcb03, E, 0, 8},
	{0xcb04, H, 0, 8},
	{0xcb05, L, 0, 8},
	{0xcb06, HL_ADDR, 0, 16},
	{-1, 0, 0, 0}
};

t_spec	rra_spec[] = {
	{0x1f, 0, 0, 4},
	{-1, 0, 0, 0}
};

t_spec	rrca_spec[] = {
	{0x0f, 0, 0, 4},
	{-1, 0, 0, 0}
};

t_spec	rla_spec[] = {
	{0x17, 0, 0, 4},
	{-1, 0, 0, 0}
};

t_spec	rlca_spec[] = {
	{0x07, 0, 0, 4},
	{-1, 0, 0, 0}
};

t_spec	ei_spec[] = {
	{0xfb, 0, 0, 4},
	{-1, 0, 0, 0}
};

t_spec	di_spec[] = {
	{0xf3, 0, 0, 4},
	{-1, 0, 0, 0}
};

t_spec	stop_spec[] = {
	{0x1000, 0, 0, 4},
	{-1, 0, 0, 0}
};

t_spec	halt_spec[] = {
	{0x76, 0, 0, 4},
	{-1, 0, 0, 0}
};

t_spec	nop_spec[] = {
	{0x00, 0, 0, 4},
	{-1, 0, 0, 0}
};

t_spec	scf_spec[] = {
	{0x37, 0, 0, 4},
	{-1, 0, 0, 0}
};

t_spec	ccf_spec[] = {
	{0x3f, 0, 0, 4},
	{-1, 0, 0, 0}
};

t_spec	cpl_spec[] = {
	{0x2f, 0, 0, 4},
	{-1, 0, 0, 0}
};

t_spec	daa_spec[] = {
	{0x27, 0, 0, 4},
	{-1, 0, 0, 0}
};

t_spec	swap_spec[] = {
	{0x37, A, 0, 8},
	{0x30, B, 0, 8},
	{0x31, C, 0, 8},
	{0x32, D, 0, 8},
	{0x33, E, 0, 8},
	{0x34, H, 0, 8},
	{0x35, L, 0, 8},
	{0x36, HL_ADDR, 0, 8},
	{-1, 0, 0, 0}
};

t_spec	dec_spec[] = {
	{0x3d, A, 0, 4},
	{0x05, B, 0, 4},
	{0x0d, C, 0, 4},
	{0x15, D, 0, 4},
	{0x1d, E, 0, 4},
	{0x25, H, 0, 4},
	{0x2d, L, 0, 4},
	{0x35, HL_ADDR, 0, 12},
	{0x0b, BC, 0, 8},
	{0x1b, DE, 0, 8},
	{0x2b, HL, 0, 8},
	{0x3b, SP, 0, 8},
	{-1, 0, 0, 0}
};

t_spec	inc_spec[] = {
	{0x3c, A, 0, 4},
	{0x04, B, 0, 4},
	{0x0c, C, 0, 4},
	{0x14, D, 0, 4},
	{0x1c, E, 0, 4},
	{0x24, H, 0, 4},
	{0x2c, L, 0, 4},
	{0x34, HL_ADDR, 0, 12},
	{0x03, BC, 0, 8},
	{0x13, DE, 0, 8},
	{0x23, HL, 0, 8},
	{0x33, SP, 0, 8},
	{-1, 0, 0, 0}
};

// cp B	--> cp A, B (A - B)
// flags = {Z, N, H, C = set}
t_spec	cp_spec[] = {
	{0xbf, A, 0, 4},
	{0xb8, B, 0, 4},
	{0xb9, C, 0, 4},
	{0xba, D, 0, 4},
	{0xbb, E, 0, 4},
	{0xbc, H, 0, 4},
	{0xbd, L, 0, 4},
	{0xbe, HL_ADDR, 0, 8},
	{0xfe, ____, 0, 8},
	{-1, 0, 0, 0}
};


// logical OR with A register
// flags {Z = set; N, C, H = reset}
// OR x --> A = A & x
t_spec	or_spec[] = {
	{0xb7, A, 0, 4},
	{0xb0, B, 0, 4},
	{0xb1, C, 0, 4},
	{0xb2, D, 0, 4},
	{0xb3, E, 0, 4},
	{0xb4, H, 0, 4},
	{0xb5, L, 0, 4},
	{0xb6, HL_ADDR, 0, 8},
	{0xf6, ____, 0, 8},
	{-1, 0, 0, 0}
};

// logical XOR with A register
// flags {Z = set; N, C, H = reset}
// XOR x --> A = A & x
t_spec	xor_spec[] = {
	{0xaf, A, 0, 4},
	{0xa8, B, 0, 4},
	{0xa9, C, 0, 4},
	{0xaa, D, 0, 4},
	{0xab, E, 0, 4},
	{0xac, H, 0, 4},
	{0xad, L, 0, 4},
	{0xae, HL_ADDR, 0, 8},
	{0xee, ____, 0, 8},
	{-1, 0, 0, 0}
};

// logical AND with A register
// flags {Z = set; N, C = reset; H = set}
// AND x --> A = A & x
t_spec	and_spec[] = {
	{0xa7, A, 0, 4},
	{0xa0, B, 0, 4},
	{0xa1, C, 0, 4},
	{0xa2, D, 0, 4},
	{0xa3, E, 0, 4},
	{0xa4, H, 0, 4},
	{0xa5, L, 0, 4},
	{0xa6, HL_ADDR, 0, 8},
	{0xe6, ____, 0, 8},
	{-1, 0, 0, 0}
};


// carry flag to A
t_spec	sbc_spec[] = {
	{0x9f, A, 0, 4},
	{0x98, B, 0, 4},
	{0x99, C, 0, 4},
	{0x9a, D, 0, 4},
	{0x9b, E, 0, 4},
	{0x9c, H, 0, 4},
	{0x9d, L, 0, 4},
	{0x9e, HL_ADDR, 0, 8},
	{????, ____, 0, ?},
	{-1, 0, 0, 0}
};

// sub to A
t_spec	sub_spec[] = {
	{0x97, A, 0, 4},
	{0x90, B, 0, 4},
	{0x91, C, 0, 4},
	{0x92, D, 0, 4},
	{0x93, E, 0, 4},
	{0x94, H, 0, 4},
	{0x95, L, 0, 4},
	{0x06, HL_ADDR, 0, 8},
	{0xd6, ____, 0, 8},
	{-1, 0, 0, 0}
};

// flags: {Z = set if riesult is zero; N = reset;
//			H = set if carry from bit 3;
//			C = set if carry of bit 7}
// carry flag to A
t_spec	adc_spec[] = {
	{0x8f, A, 0, 4},
	{0x88, B, 0, 4},
	{0x89, C, 0, 4},
	{0x8a, D, 0, 4},
	{0x8b, E, 0, 4},
	{0x8c, H, 0, 4},
	{0x8d, L, 0, 4},
	{0x8e, HL_ADDR, 0, 8},
	{0xce, ____, 0, 8},
	{-1, 0, 0, 0}
};

// flags: {Z = set if result is zero; N = reset;
//			H = set if carry from bit 3;
//			C = set if carry of bit 7}
// add x to A
t_spec	add_spec[] = {
	{0x87, A, 0, 4},
	{0x80, B, 0, 4},
	{0x81, C, 0, 4},
	{0x82, D, 0, 4},
	{0x83, E, 0, 4},
	{0x84, H, 0, 4},
	{0x85, L, 0, 4},
	{0x86, HL_ADDR, 0, 8},
	{0xc6, ____, 0, 8},
	{0x09, HL, BC, 8},
	{0x19, HL, DE, 8},
	{0x29, HL, HL, 8},
	{0x39, HL, SP, 8},
	{0xe8, SP, ____, 16},
	{-1, 0, 0, 0}
};

t_spec	pop_spec[] = {
	{0xf1, AF, 0, 12},
	{0xc1, BC, 0, 12},
	{0xd1, DE, 0, 12},
	{0xe1, HL, 0, 12},
	{-1, 0, 0, 0}
};

t_spec	push_spec[] = {
	{0xf5, AF, 0, 16},
	{0xc5, BC, 0, 16},
	{0xd5, DE, 0, 16},
	{0xe5, HL, 0, 16},
	{-1, 0, 0, 0}
};

t_spec	ldhl_spec[] = {
	// lea	(SP+IMM8), HL
	// flags :: {Z, N = reset; H, C = set or reset}
	{0xf8, SP, IMM8, 12},
};

t_spec	ldd_spec[] = {
	// load		reg, (addr)
	// dec		addr
	{0x3a, A, HL_ADDR, 8},
	// load		(addr), reg
	//dec		addr
	{0x32, HL_ADDR, A, 8},
	{-1, 0, 0, 0}
};

t_spec	ldi_spec[] = {
	// load		reg, addr
	// inc		addr
	{0x2a, A, HL_ADDR, 8},
	// load		addr, reg
	// inc		addr
	{0x22, HL_ADDR, A, 8},
	{-1, 0, 0, 0}
}

t_spec	ld_spec[] = {
	// ld	(ADDR16), SP
	{0x08, ADDR16, SP, 20},

	{0xe0, ADDR8, A, 12}, // LD	($FF00+n), A
	{0xf0}, A, ADDR8, 12, // LD A, ($FF00+n)
	// LD	reg16, imm16
	{0x01, BC, IMM16, 12},
	{0x11, DE, IMM16, 12},
	{0x21, HL, IMM16, 12},
	{0x31, SP, IMM16, 12},
	{0xf9, SP, HL, 8},
	// LD	reg, imm8
	{0x06, B, IMM8, 8},
	{0x0e, C, IMM8, 8},
	{0x16, D, IMM8, 8},
	{0x1e, E, IMM8, 8},
	{0x26, H, IMM8, 8},
	{0x2e, L, IMM8, 8},
	// LD	A, x
	{0x7f, A, A, 4},
	{0x78, A, B, 4},
	{0x79, A, C, 4},
	{0x7a, A, D, 4},
	{0x7b, A, E, 4},
	{0x7c, A, H, 4},
	{0x7d, A, L, 4},
	{0x0a, A, BC_ADDR, 8},
	{0x1a, A, DE_ADDR, 8},
	{0x7e, A, HL_ADDR, 8},
	{0xfa, A, ADDR16, 16},
	{0x3e, A, ____, 8},
	{0xf2, A, C_FF00, 8},
	// LD	x, A
	{0x7f, A, A, 4},
	{0x47, B, A, 4},
	{0x4f, C, A, 4},
	{0x57, D, A, 4},
	{0x5f, E, A, 4},
	{0x67, H, A, 4},
	{0x6f, L, A, 4},
	{0x02, BC_ADDR, A, 8},
	{0x12, DE_ADDR, A, 8},
	{0x77, HL_ADDR, A, 8},
	{0xea, ADDR16, A, 16},
	{0xe2, C_FF00, A, 8},
	{0x40, B, B, 4},
	{0x41, B, C, 4},
	{0x42, B, D, 4},
	{0x43, B, E, 4},
	{0x44, B, H, 4},
	{0x45, B, L, 4},
	{0x46, B, HL_ADDR, 8},
	{0x48, C, B, 4},
	{0x49, C, C, 4},
	{0x4a, C, D, 4},
	{0x4b, C, E, 4},
	{0x4c, C, H, 4},
	{0x4d, C, L, 4},
	{0x4e, C, HL_ADDR, 8},
	{0x50, D, B, 4},
	{0x51, D, C, 4},
	{0x52, D, D, 4},
	{0x53, D, E, 4},
	{0x54, D, H, 4},
	{0x55, D, L, 4},
	{0x56, D, HL_ADDR, 8},
	{0x58, E, B, 4},
	{0x59, E, C, 4},
	{0x5a, E, D, 4},
	{0x5b, E, E, 4},
	{0x5c, E, H, 4},
	{0x5d, E, L, 4},
	{0x5e, E, HL_ADDR, 8},
	{0x60, H, B, 4},
	{0x61, H, C, 4},
	{0x62, H, D, 4},
	{0x63, H, E, 4},
	{0x64, H, H, 4},
	{0x65, H, L, 4},
	{0x66, H, HL_ADDR, 8},
	{0x68, L, B, 4},
	{0x69, L, C, 4},
	{0x6a, L, D, 4},
	{0x6b, L, E, 4},
	{0x6c, L, H, 4},
	{0x6d, L, L, 4},
	{0x6e, L, HL_ADDR, 8},
	{0x70, HL_ADDR, B, 8},
	{0x71, HL_ADDR, C, 8},
	{0x72, HL_ADDR, D, 8},
	{0x73, HL_ADDR, E, 8},
	{0x74, HL_ADDR, H, 8},
	{0x75, HL_ADDR, L, 8},
	{0x36, HL_ADDR, IMM8, 12},
	{-1, 0, 0, 0}
};

/*

	premiere lecture:

		recuperation des directives (.CARTRIDGE, .TITLE, ...)

	deuxieme lecture:

		recuperation des labels

	troisieme lecture:

		compilation(remplacement des defines a la volee, ...)


	const void *const	mnemonic[]; // check mnemoniques
	-> unknown mnemonic:
		directive_t		directive[]; // %define, %undef, %bytes, %include
		-> unknown directive:
			define_t		define[]; // check defines
			-> not a define: ERROR
	const void *const operands[][]; // check operand 1


	%define	ARGUMENT(a,b)	a, b

	ld	ARGUMENT(C, D)

*/


/*

	file1:

	.bank	1, 0

	ld	A, 0x3
	...

	.bank	1, 0x400


*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>

char	*get_file_contents(const char *file, unsigned int *length)
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

typedef struct		ll_s
{
	char			*name;		// file name
	char			*content;
	unsigned int	len;		// file length
	unsigned int	pos;		// file start position (bank number * 0x4000 + offset)
	struct ll_s		*next;
}					ll_t;

typedef struct		error_s
{
	char			*display;
	struct error_s	*next;
}					error_t;

typedef struct		parsed_s
{
	unsigned int	type; // MNEMONIC, OPERAND, LABEL, DIRECTIVE
	char			*data;
	struct parsed_s	*next;
	unsigned int	hash;
}					parsed_t;

typedef struct		line_s
{
	
	struct parsed_s		*code;
	struct line_s		*next;
	unsigned int		line;
}					line_t;

typedef struct		section_s
{
	struct line_s		*line;
	struct section_s	*next;
	char				*file;
}					section_t;

void	push_file_name(ll_t **p, char *filename)
{
	static ll_t	*ptr = NULL;

	if (ptr == NULL)
	{
		ptr = malloc(sizeof(ll_t));
		*p = ptr;
		ptr->name = filename;
		ptr->pos = 0;
		ptr->next = NULL;
		return ;
	}
	ptr->next = malloc(sizeof(ll_t));
	ptr = ptr->next;
	ptr->name = filname;
	ptr->pos = 0;
	ptr->next = NULL;
}

void	set_input_output_file_name(char *v[], ll_t **src, char **dst)
{
	unsigned int	gbc = 0;

	for (; *v; v++)
	{
		// if (v == "-o") *dst = strdup(v + 1);
		if (**v == '-' && (*v)[1] == 'o' && (*v)[2] == '\0')
		{
			if (v[1] == NULL || v[1][0] == '\0')
			{
				fprintf(stderr, "file name expected after -o\n");
				exit(1);
			}
			v++;
			*dst = strdup(*v);
		}
		else if (strcmp(*v, "--gbc"))
		{
			if (gbc)
			{
				fprintf(stderr, "too many --gbc arguments");
				exit(1);
			}
			gbc = 1;
		}
		else
		{
			push_file_name(src);
		}
	}
	if (*dst == NULL)
	{
		*dst = (gbc) ?
			strdup("no_name.gbc") :
			strdup("no_name.gb");
	}
}

void	load_files(ll_t *src)
{
	int		error;

	while (src)
	{
		if ((src->content = get_file_contents(src->name, &src->len)) == NULL)
			error = 1;
		src = src->next;
	}
	if (error)
		exit(1);
}

#define MNEMONIC_EXISTS(hash)	check_mnemonic[hash]
#define DIRECTIVE_EXISTS(hash)	check_directive[hash]


void	push_mnemonic(parsed_t *p, )
{
	static const void *const	opcode[] = {
		
	};

	if (strcmp("push", p->data) != 0)
	{
		push_error(filename, line->line, "Unknown directive \"%s\"\n", line->code->data);
		return ;
	}
}

char	*assemble(char *bin, line_t *line, char *filename, unsigned int end)
{
	void	(*check_mnemonic[])() = {
		//
	};
	void	(*check_directive[])() = {
		//
	};

	for (line = code->line; line; line = line->next)
	{
		if (line->code->type == MNEMONIC)
		{
			if (MNEMONIC_EXISTS(line->code->hash))
				check_mnemonic[code->hash](line->code);
			else
				push_error(filename, line->line, "Unknown mnemonic \"%s\"\n", line->code->data);
		}
		else if (line->code->type == DIRECTIVE)
		{
			if (DIRECTIVE_EXISTS(code->hash))
				check_directive[code->hash](line->code);
			else
				push_error(filename, line->line, "Unknown directive \"%s\"\n", line->code->data);
		}
		else
		{
			fprintf(stderr, "Fatal error\n");
			exit(1);
		}
	}
}

char	*assemble_sections(section_t *sect, cartridge_t *cart)
{
	char			*bin = malloc(cart->rom_size);
	char			*cur = bin;
	unsigned int	end;

	for (; sect; sect = sect->next)
	{
		end = sect->next ? sect->next->start_pos : ((sect->start_pos / 0x4000) + 1) * 0x4000;
		bin = assemble(bin, sect->line, sect->file, end);
	}
}

int		main(unsigned int argc, char *argv[])
{
	cartridge_t		cart;
	labels_t		label;
	ll_t			*src = NULL;
	char			*dst = NULL;
	char			*content;
	section_t		*sections;
	char			*binary;

	if (argc == 1)
	{
		print_usage(argv[0]);
		return (1);
	}
	set_input_output_file_name(argv + 1, &src, &dst);
	load_files(src);
	set_cartridge_header(&cart, src);	// get cartridge informations in all files (.cartridge_type, .ram_size, .rom_size, ...), at the begining of the files
	content = merge_files(src, &cart);	// merge all files into content.
	free_list(src);

	sections = get_sections(content, &label);	// cut content into sections (consecutive code areas). sect->code = words table
	free(content);

	binary = assemble_sections(sections, &cart);
	put_file_contents(binary, cart->rom_size, dst);
	free(dst);

	return (0);
}
