/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   execute.c                                        .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/06/12 18:09:06 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/06/20 16:27:47 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "memory_map.h"
#include "execute.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../disassembler/disassemble_table.c"

//extern uint8_t		*g_get_real_addr[16];
//extern uint8_t		*g_get_real_write_addr[16];
//extern memory_map_t	g_memmap;


#define ADD_PC(offset)	regs->reg_pc += (offset)
#define SET_PC(value)	regs->reg_pc = value

#define SET_LOW_ROM_NUMBER_MBC5()	\
	CART_REG[1] = value;\
	SWITCH_ROM = ROM_BANK [ CART_REG[1] | (CART_REG[2] << 8) ];\
	g_get_real_addr[4] = SWITCH_ROM;\
	g_get_real_addr[5] = SWITCH_ROM + 0x1000;\
	g_get_real_addr[6] = SWITCH_ROM + 0x2000;\
	g_get_real_addr[7] = SWITCH_ROM + 0x3000;\

#define SET_HI_ROM_NUMBER_MBC5()	\
	CART_REG[2] = (value & 0x01);\
	SWITCH_ROM = ROM_BANK [ CART_REG[1] | (CART_REG[2] << 8) ];\
	g_get_real_addr[4] = SWITCH_ROM;\
	g_get_real_addr[5] = SWITCH_ROM + 0x1000;\
	g_get_real_addr[6] = SWITCH_ROM + 0x2000;\
	g_get_real_addr[7] = SWITCH_ROM + 0x3000;\

#define SET_RAM_NUMBER_MBC5()		\
	CUR_RAM = (value & 0x0f); /*CART_REG[3]*/\
	EXTERN_RAM = RAM_BANK[CUR_RAM];\
	g_get_real_addr[10] = EXTERN_RAM;\
	g_get_real_addr[11] = EXTERN_RAM + 0x1000;\


#define SET_MBC1_MODE_0_ROM_ADDR()	\
	do\
	{/* if file contain empty banks at 0x20, 0x40, 0x60 */\
		if ((CART_REG[1] | (CART_REG[2] << 5)) == 0x20)\
			SWITCH_ROM = ROM_BANK [0x20];\
		else if ((CART_REG[1] | (CART_REG[2] << 5)) == 0x40)\
			SWITCH_ROM = ROM_BANK [0x40];\
		else if ((CART_REG[1] | (CART_REG[2] << 5)) == 0x60)\
			SWITCH_ROM = ROM_BANK [0x60];\
		else if ((CART_REG[1] | (CART_REG[2] << 5)) > 0)\
			SWITCH_ROM = ROM_BANK [ (CART_REG[1] | (CART_REG[2] << 5)) - 1];\
		else\
			SWITCH_ROM = ROM_BANK [0];\
		g_get_real_addr[4] = SWITCH_ROM;\
		g_get_real_addr[5] = SWITCH_ROM + 0x1000;\
		g_get_real_addr[6] = SWITCH_ROM + 0x2000;\
		g_get_real_addr[7] = SWITCH_ROM + 0x3000;\
	} while (0)

#define SET_MBC1_MODE_1_ROM_ADDR()	\
	do\
	{\
		if (CART_REG [MBC1_ROM_NUM] > 0)\
			SWITCH_ROM = ROM_BANK [ CART_REG [MBC1_ROM_NUM] - 1 ];\
		else\
			SWITCH_ROM = ROM_BANK [0];\
		g_get_real_addr[4] = SWITCH_ROM;\
		g_get_real_addr[5] = SWITCH_ROM + 0x1000;\
		g_get_real_addr[6] = SWITCH_ROM + 0x2000;\
		g_get_real_addr[7] = SWITCH_ROM + 0x3000;\
	} while (0)

#define SET_MBC1_MODE_0_RAM_ADDR()	\
	do\
	{\
		if (CART_REG[0])\
		{\
			EXTERN_RAM = RAM_BANK[0];\
			g_get_real_addr[10] = EXTERN_RAM;\
			g_get_real_addr[11] = EXTERN_RAM + 0x1000;\
		}\
		CUR_RAM = 0;\
	} while (0)

#define SET_MBC1_MODE_1_RAM_ADDR()	\
	do\
	{\
		if (CART_REG[0])\
		{\
			EXTERN_RAM = RAM_BANK [ CART_REG[2] ];\
			g_get_real_addr[10] = EXTERN_RAM;\
			g_get_real_addr[11] = EXTERN_RAM + 0x1000;\
		}\
		CUR_RAM = CART_REG[2];\
	} while (0)

#define	SWITCH_RAM_ROM_MBC1()	\
	do\
	{\
		if (g_memmap.cart_reg[MBC1_MODE]) {\
			SET_MBC1_MODE_1_ROM_ADDR();\
			SET_MBC1_MODE_1_RAM_ADDR();\
		}\
		else {\
			plog("MODE = 0 (addr = cart_reg[0] | cart_reg[1] << 5\n");\
			SET_MBC1_MODE_0_ROM_ADDR();\
			SET_MBC1_MODE_0_RAM_ADDR();\
		}\
	} while (0)

#define ENABLE_EXTERNAL_RAM_MBC5()
// if write on disabled ram is a fatal error,
// please remove 0x2000 offset
#define ENABLE_EXTERNAL_RAM_MBC1()	\
	if (value == 0x0a)\
	{\
		g_memmap.cart_reg[0] = 1;\
		EXTERN_RAM = RAM_BANK[CUR_RAM];\
	}\
	else\
	{\
		g_memmap.cart_reg[0] = 0;\
		EXTERN_RAM = g_memmap.complete_block + 0x2000;\
	}\
	g_get_real_addr[10] = EXTERN_RAM;\
	g_get_real_addr[11] = EXTERN_RAM + 0x1000;\

static char	*get_bin(unsigned char n)
{
	static char					buf[16][9] = {{0}};
	static uint32_t				cur = 0;
	unsigned char				curs;
	unsigned int				i = 0;

	if (cur == 16)
		cur = 0;
	curs = 1UL << 7;
	while (curs)
	{
		buf[cur][i++] = (curs & n) ? '1' : '0';
		curs >>= 1;
	}
	buf[cur++][i] = 0;
	return (buf[cur - 1]);
}

static inline const char		*left_trim(const char *s)
{
	while (*s == ' ' || *s == '\t')
		s++;
	if (*s == '0' && s[1] == 'x')
	{
		s += 2;
		while (*s == '0')
			s++;
		return (s);
	}
	return (NULL);
}

static unsigned int				get_base_value(char c)
{
	if (c >= 'a' && c <= 'f')
		return (c - ('a' - 10));
	if (c >= 'A' && c <= 'F')
		return (c - ('a' - 10));
	return (c - '0');
}

static inline unsigned int		ft_strtoi(const char *s)
{
	unsigned int	n;

	n = 0;
	while (1)
	{
		if (*s > 'f' || (*s > 'F' && *s < 'a')
				|| (*s > '9' && *s < 'A') || *s < '0')
			break ;
		n *= 16;
		n += get_base_value(*(s++));
	}
	return (n);
}

extern inline unsigned int		atoi_hexa(const char *s)
{
	if ((s = left_trim(s)) == NULL)
		return (0);
	return (ft_strtoi(s));
}


static char		*itoazx(int n, int size)
{
	static char	s[8];
	int			i = 6;
	int			negative = 0;

	if (n < 0)
	{
		negative++;
		n = 0 - n;
	}

	s[7] = '\0';
	while (n)
	{
		s[i] = ((n % 16) + '0');
		if (s[i] > '9')
			s[i] += 39;
		i--;
		n /= 16;
	}
	while (i > 4 - size)
	{
		s[i--] = '0';
	}
	s[i + 2] = 'x';
	if (negative)
	{
		s[i] = '-';
		return (s + i);
	}
	return (s + i + 1);
}

int				get_int16_from_little_endian(void *memory)
{
#if BYTE_ORDER == LITTLE_ENDIAN
	return (*(int16_t*)memory);
#else
	return ((int16_t)(*(unsigned char*)memory + *((unsigned char*)(memory + 1)) << 8));
#endif
}

unsigned int	get_uint16_from_little_endian(void *memory)
{
#if BYTE_ORDER == LITTLE_ENDIAN
	return (*(uint16_t*)memory);
#else
	return (*(unsigned char*)memory + *((unsigned char*)(memory + 1)) << 8);
#endif
}

static char		*fmt_strcpy(char *src,
							enum e_operand_type optype, void *bin)
{
	static char buf[512];
	char		*dst = buf;
	int			value;
	char		*numeric = NULL;

	while (*src != '*' && (*(dst++) = *(src)))
		src++;
	if (*src == 0)
		return (buf);
	src++;

	if (optype == IMM8)
	{
		value = *((int8_t*)bin);
		numeric = itoazx(value, 2);
	}
	else if (optype == IMM16)
	{
		value = get_int16_from_little_endian(bin);
		numeric = itoazx(value, 4);
	}
	else if (optype == ADDR8)
	{
		value = *((uint8_t*)bin);
		numeric = itoazx(value, 2);
	}
	else if (optype == ADDR16)
	{
		value = get_uint16_from_little_endian(bin);
		numeric = itoazx(value, 4);
	}
	else
	{
		fprintf(stderr, "\e[0;31mFATAL ERROR\e[0m\n");
		exit(1);
	}
	strcpy(dst, numeric);
	dst += strlen(numeric);
	while ((*(dst++) = *(src++)));
	return (buf);
}

cycle_count_t	totalcycles;

cycle_count_t	execute(registers_t *regs)
{
	static const void *const	jump_to_mbcx[5][8] =
	{
		{
			&&redzone_ret,	&&redzone_ret,	&&redzone_ret,	&&redzone_ret,
			&&redzone_ret,	&&redzone_ret,	&&redzone_ret,	&&redzone_ret
		},
		{
			&&mbc1_0,		&&mbc1_0,		&&mbc1_1,		&&mbc1_1,
			&&mbc1_2,		&&mbc1_2,		&&mbc1_3,		&&mbc1_3
		},
		{
			&&mbc2_0,		&&mbc2_0,		&&mbc2_1,		&&mbc2_1,
			&&redzone_ret,	&&redzone_ret,	&&redzone_ret,	&&redzone_ret
		},
		{
			&&mbc3_0,		&&mbc3_0,		&&mbc3_1,		&&mbc3_1,
			&&mbc3_2,		&&mbc3_2,		&&mbc3_3,		&&mbc3_3
		},
		{
			&&mbc1_0,		&&mbc1_0,		&&mbc5_1,		&&mbc5_2,
			&&mbc5_3,		&&mbc5_3,		&&redzone_ret,	&&redzone_ret
		}
	};

	static const void *const	instruction_jumps[256] =
	{
		&&nop,				&&ld_bc_imm16,		&&ld_bc_a,			&&inc_bc,
		&&inc_b,			&&dec_b,			&&ld_b_imm8,		&&rlca,
		&&ld_addr16_sp,		&&add_hl_bc,		&&ld_a_bc,			&&dec_bc,
		&&inc_c,			&&dec_c,			&&ld_c_imm8,		&&rrca,
		&&stop,				&&ld_de_imm16,		&&ld_de_a,			&&inc_de,
		&&inc_d,			&&dec_d,			&&ld_d_imm8,		&&rla,
		&&jr_imm8,			&&add_hl_de,		&&ld_a_de,			&&dec_de,
		&&inc_e,			&&dec_e,			&&ld_e_imm8,		&&rra,
		&&jrnz_imm8,		&&ld_hl_imm16,		&&ld_hli_a,			&&inc_hl,
		&&inc_h,			&&dec_h,			&&ld_h_imm8,		&&daa,
		&&jrz_imm8,			&&add_hl_hl,		&&ld_a_hli,			&&dec_hl,
		&&inc_l,			&&dec_l,			&&ld_l_imm8,		&&cpl,
		&&jrnc_imm8,		&&ld_sp_imm16,		&&ld_hld_a,			&&inc_sp,
		&&inc_ahl,			&&dec_ahl,			&&ld_hl_imm8,		&&scf,
		&&jrc_imm8,			&&add_hl_sp,		&&ld_a_hld,			&&dec_sp,
		&&inc_a,			&&dec_a,			&&ld_a_imm8,		&&ccf,
		&&ld_b_b,			&&ld_b_c,			&&ld_b_d,			&&ld_b_e,
		&&ld_b_h,			&&ld_b_l,			&&ld_b_hl,			&&ld_b_a,
		&&ld_c_b,			&&ld_c_c,			&&ld_c_d,			&&ld_c_e,
		&&ld_c_h,			&&ld_c_l,			&&ld_c_hl,			&&ld_c_a,
		&&ld_d_b,			&&ld_d_c,			&&ld_d_d,			&&ld_d_e,
		&&ld_d_h,			&&ld_d_l,			&&ld_d_hl,			&&ld_d_a,
		&&ld_e_b,			&&ld_e_c,			&&ld_e_d,			&&ld_e_e,
		&&ld_e_h,			&&ld_e_l,			&&ld_e_hl,			&&ld_e_a,
		&&ld_h_b,			&&ld_h_c,			&&ld_h_d,			&&ld_h_e,
		&&ld_h_h,			&&ld_h_l,			&&ld_h_hl,			&&ld_h_a,
		&&ld_l_b,			&&ld_l_c,			&&ld_l_d,			&&ld_l_e,
		&&ld_l_h,			&&ld_l_l,			&&ld_l_hl,			&&ld_l_a,
		&&ld_hl_b,			&&ld_hl_c,			&&ld_hl_d,			&&ld_hl_e,
		&&ld_hl_h,			&&ld_hl_l,			&&halt,				&&ld_hl_a,
		&&ld_a_b,			&&ld_a_c,			&&ld_a_d,			&&ld_a_e,
		&&ld_a_h,			&&ld_a_l,			&&ld_a_hl,			&&ld_a_a,
		&&add_a_b,			&&add_a_c,			&&add_a_d,			&&add_a_e,
		&&add_a_h,			&&add_a_l,			&&add_a_hl,			&&add_a_a,
		&&adc_a_b,			&&adc_a_c,			&&adc_a_d,			&&adc_a_e,
		&&adc_a_h,			&&adc_a_l,			&&adc_a_hl,			&&adc_a_a,
		&&sub_a_b,			&&sub_a_c,			&&sub_a_d,			&&sub_a_e,
		&&sub_a_h,			&&sub_a_l,			&&sub_a_hl,			&&sub_a_a,
		&&sbc_a_b,			&&sbc_a_c,			&&sbc_a_d,			&&sbc_a_e,
		&&sbc_a_h,			&&sbc_a_l,			&&sbc_a_hl,			&&sbc_a_a,
		&&and_a_b,			&&and_a_c,			&&and_a_d,			&&and_a_e,
		&&and_a_h,			&&and_a_l,			&&and_a_hl,			&&and_a_a,
		&&xor_a_b,			&&xor_a_c,			&&xor_a_d,			&&xor_a_e,
		&&xor_a_h,			&&xor_a_l,			&&xor_a_hl,			&&xor_a_a,
		&&or_a_b,			&&or_a_c,			&&or_a_d,			&&or_a_e,
		&&or_a_h,			&&or_a_l,			&&or_a_hl,			&&or_a_a,
		&&cp_a_b,			&&cp_a_c,			&&cp_a_d,			&&cp_a_e,
		&&cp_a_h,			&&cp_a_l,			&&cp_a_hl,			&&cp_a_a,
		&&retnz,			&&pop_bc,			&&jpnz_imm16,		&&jp_imm16,
		&&callnz_imm16,		&&push_bc,			&&add_a_imm8,		&&rst_00h,
		&&retz,				&&ret,				&&jpz_imm16,		&&prefix_cb,
		&&callz_imm16,		&&call_imm16,		&&adc_a_imm8,		&&rst_08h,
		&&retnc,			&&pop_de,			&&jpnc_imm16,		&&illegal,
		&&callnc_imm16,		&&push_de,			&&sub_a_imm8,		&&rst_10h,
		&&retc,				&&reti,				&&jpc_imm16,		&&illegal,
		&&callc_imm16,		&&illegal,			&&sbc_a_imm8,		&&rst_18h,
		&&ldff_imm8_a,		&&pop_hl,			&&ldff_c_a,			&&illegal,
		&&illegal,			&&push_hl,			&&and_a_imm8,		&&rst_20h,
		&&add_sp_imm8,		&&jp_hl,			&&ld_imm16_a,		&&illegal,
		&&illegal,			&&illegal,			&&xor_a_imm8,		&&rst_28h,
		&&ldff_a_imm8,		&&pop_af,			&&ldff_a_c,			&&di,
		&&illegal,			&&push_af,			&&or_a_imm8,		&&rst_30h,
		&&ld_hl_sp_imm8,	&&ld_sp_hl,			&&ld_a_imm16,		&&ei,
		&&illegal,			&&illegal,			&&cp_a_imm8,		&&rst_38h
	};
	extern int		log_file;
	cycle_count_t	cycles;
	uint8_t			value;

	uint8_t			*address;

	address = GET_REAL_ADDR(regs->reg_pc);
	uint8_t				opcode = address[0];
	register uint8_t	imm_8 = address[1];
	register uint16_t	imm_16 = (uint16_t)address[1] | ((uint16_t)address[2] << 8);

#if (_CPU_LOG == true)

	dprintf(log_file,
			"\n\n\n\n\nA = %2hhx,    F = %2hhx(%c%c%c%c), B = %2hhx,    C = %2hhx\n"
			"D = %2hhx,    E = %2hhx,       H = %2hhx,    L = %2hhx\n"
			"AF = %4hx, BC = %4hx,    DE = %4hx, HL = %4hx\n"
			"PC = %4hx, ADDR = %p,   SP = %4hx\n\n"
			"LCDC = %c%c%c%c%c%c%c%c, STAT = %c%c%c%c%c%hhx\n\n"
			"SCY = %2hhx, SCX = %2hhx, LY = %2hhx, LYC = %2hhx\n\n"
			"DIV = %2hhx, TIMA = %3hhu, TMA = %3hhu, TAC = %2hhx\n\n"
			"IF = %c%c%c%c%c, IE = %c%c%c%c%c, IME = %s\n",
			regs->reg_a, regs->reg_f, (regs->reg_f & FLAG_Z) ? 'Z' : '.',
			(regs->reg_f & FLAG_N) ? 'N' : '.', (regs->reg_f & FLAG_H) ? 'H' : '.',
			(regs->reg_f & FLAG_CY) ? 'C' : '.', regs->reg_b, regs->reg_c,
			regs->reg_d, regs->reg_e, regs->reg_h, regs->reg_l, regs->reg_af,
			regs->reg_bc, regs->reg_de, regs->reg_hl, regs->reg_pc, address,
			regs->reg_sp, 
			(LCDC_REGISTER & BIT_7) ? 'L' : '.', (LCDC_REGISTER & BIT_6) ? '1' : '0',
			(LCDC_REGISTER & BIT_5) ? 'W' : '.', (LCDC_REGISTER & BIT_4) ? '1' : '0',
			(LCDC_REGISTER & BIT_3) ? '1' : '0', (LCDC_REGISTER & BIT_2) ? 'D' : 'S',
			(LCDC_REGISTER & BIT_1) ? 'O' : '.', (LCDC_REGISTER & BIT_0) ? 'B' : '.',
			(STAT_REGISTER & BIT_6) ? 'L' : '.', (STAT_REGISTER & BIT_5) ? 'O' : '.',
			(STAT_REGISTER & BIT_4) ? 'V' : '.', (STAT_REGISTER & BIT_3) ? 'H' : '.',
			(STAT_REGISTER & BIT_2) ? 'Y' : '.', (STAT_REGISTER & (BIT_0 | BIT_1)),
			SCY_REGISTER, SCX_REGISTER, LY_REGISTER, LYC_REGISTER,
			DIV_REGISTER, TIMA_REGISTER, TMA_REGISTER, TAC_REGISTER,
			(IF_REGISTER & BIT_4) ? 'J' : '.', (IF_REGISTER & BIT_3) ? 'S' : '.',
			(IF_REGISTER & BIT_2) ? 'T' : '.', (IF_REGISTER & BIT_1) ? 'L' : '.',
			(IF_REGISTER & BIT_0) ? 'V' : '.',
			(IE_REGISTER & BIT_4) ? 'J' : '.', (IE_REGISTER & BIT_3) ? 'S' : '.',
			(IE_REGISTER & BIT_2) ? 'T' : '.', (IE_REGISTER & BIT_1) ? 'L' : '.',
			(IE_REGISTER & BIT_0) ? 'V' : '.',
			g_memmap.ime ? "enabled" : "disabled");

	if (opcode == 0xcb)
		plog(cb_opcodes[address[1]].inst);
	else
		plog(fmt_strcpy(opcodes[opcode].inst, opcodes[opcode].optype, address + 1));

#endif

	goto *instruction_jumps[opcode];

#undef plog
#define plog(x)

nop:
plog("nop\n");
	ADD_PC(1);
	return (4);

ld_bc_imm16:
plog("ld_bc_imm16\n");
	ADD_PC(3);
	regs->reg_bc = imm_16;
	return (12);

ld_bc_a:
plog("ld_bc_a\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_bc);
	WRITE_REGISTER_IF_ROM_AREA(regs->reg_bc, regs->reg_a, 8);
	*address = regs->reg_a;
	return (8);

inc_bc:
plog("inc_bc\n");
	ADD_PC(1);
	regs->reg_bc += 1;
	return (8);

inc_b:
plog("inc_b\n");
	ADD_PC(1);
	regs->reg_f &= FLAG_CY;
	regs->reg_b += 1;
	if (regs->reg_b == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_b & 0xFu) == 0)
		regs->reg_f |= FLAG_H;
	return (4);

dec_b:
plog("dec_b\n");
	ADD_PC(1);
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_N;
	regs->reg_b -= 1;
	if (regs->reg_b == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_b & 0xFu) == 0xFu)
		regs->reg_f |= FLAG_H;
	return (4);

ld_b_imm8:
plog("ld_b_imm8\n");
	ADD_PC(2);
	regs->reg_b = imm_8;
	return (8);

// left-shift a, save bit 7 in carry flag and bit 0 of a

rlca:
plog("rlca\n");
	ADD_PC(1);
	imm_8 = ((regs->reg_a & BIT_7) == BIT_7) ? BIT_0 : 0;
	regs->reg_f = imm_8 << 4;
	regs->reg_a <<= 1;
	regs->reg_a |= imm_8;
	return (4);

// [16-bit immediate] = stack pointer

ld_addr16_sp:
plog("ld_addr16_sp\n");
	ADD_PC(3);
	address = GET_REAL_ADDR(imm_16);
	address[0] = regs->reg_sl;
	address[1] = regs->reg_sh;
	return (20);

add_hl_bc:
plog("add_hl_bc\n");
	ADD_PC(1);
	regs->reg_f &= FLAG_Z;
	imm_16 = regs->reg_hl + regs->reg_bc;
	if (imm_16 < regs->reg_hl)
		regs->reg_f |= FLAG_CY;
	if ((imm_16 & 0xFFFu) < (regs->reg_hl & 0xFFFu))
		regs->reg_f |= FLAG_H;
	regs->reg_hl = imm_16;
	return (8);

ld_a_bc:
plog("ld_a_bc\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_bc);
	regs->reg_a = *address;
	return (8);

dec_bc:
plog("dec_bc\n");
	ADD_PC(1);
	regs->reg_bc -= 1;
	return (8);

inc_c:
plog("inc_c\n");
	ADD_PC(1);
	regs->reg_f &= FLAG_CY;
	regs->reg_c += 1;
	if (regs->reg_c == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_c & 0xFu) == 0)
		regs->reg_f |= FLAG_H;
	return (4);

// c--

dec_c:
plog("dec_c\n");
	ADD_PC(1);
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_N;
	regs->reg_c -= 1;
	if (regs->reg_c == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_c & 0xFu) == 0xFu)
		regs->reg_f |= FLAG_H;
	return (4);

ld_c_imm8:
plog("ld_c_imm8\n");
	ADD_PC(2);
	regs->reg_c = imm_8;
	return (8);

// right-shift a, save bit 0 in carry flag and bit 7 of a

rrca:
plog("rrca\n");
	ADD_PC(1);
	imm_8 = ((regs->reg_a & BIT_0) == BIT_0) ? BIT_7 : 0;
	regs->reg_f = imm_8 >> 3;
	regs->reg_a >>= 1;
	regs->reg_a |= imm_8;
	return (4);

// stop processor AND internal clock until interrupt flag is ok

stop:
plog("stop\n");
	ADD_PC(1);
	GAMEBOY = STOP_MODE;
	return (4);

ld_de_imm16:
plog("ld_de_imm16\n");
	ADD_PC(3);
	regs->reg_de = imm_16;
	return (12);

ld_de_a:
plog("ld_de_a\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_de);
	WRITE_REGISTER_IF_ROM_AREA(regs->reg_de, regs->reg_a, 8);
	*address = regs->reg_a;
	return (8);

inc_de:
plog("inc_de\n");
	ADD_PC(1);
	regs->reg_de += 1;
	return (8);

inc_d:
plog("inc_d\n");
	ADD_PC(1);
	regs->reg_f &= FLAG_CY;
	regs->reg_d += 1;
	if (regs->reg_d == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_d & 0xFu) == 0)
		regs->reg_f |= FLAG_H;
	return (4);

dec_d:
plog("dec_d\n");
	ADD_PC(1);
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_N;
	regs->reg_d -= 1;
	if (regs->reg_d == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_d & 0xFu) == 0xFu)
		regs->reg_f |= FLAG_H;
	return (4);

ld_d_imm8:
plog("ld_d_imm8\n");
	ADD_PC(2);
	regs->reg_d = imm_8;
	return (8);

// left-shift a, save bit 7 in carry flag and carry flag in bit 0 of a

rla:
plog("rla\n");
	ADD_PC(1);
	imm_8 = ((regs->reg_f & FLAG_CY) == FLAG_CY) ? BIT_0 : 0;
	regs->reg_f = ((regs->reg_a & BIT_7) == BIT_7) ? FLAG_CY : 0;
	regs->reg_a <<= 1;
	regs->reg_a |= imm_8;
	return (4);

// increment pc with 8-bit immediate value

jr_imm8:
plog("jr_imm8\n");
	ADD_PC(2);
	ADD_PC((int8_t)imm_8);
	return (12);

add_hl_de:
plog("add_hl_de\n");
	ADD_PC(1);
	regs->reg_f &= FLAG_Z;
	imm_16 = regs->reg_hl + regs->reg_de;
	if (imm_16 < regs->reg_hl)
		regs->reg_f |= FLAG_CY;
	if ((imm_16 & 0xFFFu) < (regs->reg_hl & 0xFFFu))
		regs->reg_f |= FLAG_H;
	regs->reg_hl = imm_16;
	return (8);

ld_a_de:
plog("ld_a_de\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_de);
	regs->reg_a = *address;
	return (8);

dec_de:
plog("dec_de\n");
	ADD_PC(1);
	regs->reg_de -= 1;
	return (8);

inc_e:
plog("inc_e\n");
	ADD_PC(1);
	regs->reg_f &= FLAG_CY;
	regs->reg_e += 1;
	if (regs->reg_e == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_e & 0xFu) == 0)
		regs->reg_f |= FLAG_H;
	return (4);

dec_e:
plog("dec_e\n");
	ADD_PC(1);
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_N;
	regs->reg_e -= 1;
	if (regs->reg_e == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_e & 0xFu) == 0xFu)
		regs->reg_f |= FLAG_H;
	return (4);

ld_e_imm8:
plog("ld_e_imm8\n");
	ADD_PC(2);
	regs->reg_e = imm_8;
	return (8);

// right-shift a, save bit 0 in carry flag and carry flag in bit 7 of a

rra:
plog("rra\n");
	ADD_PC(1);
	imm_8 = ((regs->reg_f & FLAG_CY) == FLAG_CY) ? BIT_7 : 0;
	regs->reg_f = ((regs->reg_a & BIT_0) == BIT_0) ? FLAG_CY : 0;
	regs->reg_a >>= 1;
	regs->reg_a |= imm_8;
	return (4);

// increment pc with 8-bit immediate value, if zero flag is not set

jrnz_imm8:
plog("jrnz_imm8\n");
	if ((regs->reg_f & FLAG_Z) == 0) {
		ADD_PC(2);
		ADD_PC((int8_t)imm_8);
		return (12);
	}
	else {
		ADD_PC(2);
		return (8);
	}

ld_hl_imm16:
plog("ld_hl_imm16\n");
	ADD_PC(3);
	regs->reg_hl = imm_16;
	return (12);

ld_hli_a:
plog("ld_hli_a\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_hl += 1;
	WRITE_REGISTER_IF_ROM_AREA(regs->reg_hl - 1, regs->reg_a, 8);
	*address = regs->reg_a;
	return (8);

inc_hl:
plog("inc_hl\n");
	ADD_PC(1);
	regs->reg_hl += 1;
	return (8);

inc_h:
plog("inc_h\n");
	ADD_PC(1);
	regs->reg_f &= FLAG_CY;
	regs->reg_h += 1;
	if (regs->reg_h == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_h & 0xFu) == 0)
		regs->reg_f |= FLAG_H;
	return (4);

dec_h:
plog("dec_h\n");
	ADD_PC(1);
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_N;
	regs->reg_h -= 1;
	if (regs->reg_h == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_h & 0xFu) == 0xFu)
		regs->reg_f |= FLAG_H;
	return (4);

ld_h_imm8:
plog("ld_h_imm8\n");
	ADD_PC(2);
	regs->reg_h = imm_8;
	return (8);

#define lo_bit opcode
#define hi_bit imm_8

// decimal-adjust a, used to perform decimal additions on registers

daa:
plog("daa\n");
	ADD_PC(1);
	lo_bit = regs->reg_a & 0xFu;
	hi_bit = regs->reg_a >> 4;
	if ((regs->reg_f & FLAG_N) == FLAG_N)
	{
		if ((regs->reg_f & FLAG_H) == FLAG_H)
		{
			if ((regs->reg_f & FLAG_CY) == FLAG_CY || hi_bit >= 0xAu)
			{
				regs->reg_a += 0x66u;
				regs->reg_f |= FLAG_CY;
			}
			else
			{
				regs->reg_a += 0x06u;
			}
		}
		else if (lo_bit >= 0xAu)
		{
			if ((regs->reg_f & FLAG_CY) == FLAG_CY || hi_bit >= 0x9u)
			{
				regs->reg_a += 0x66u;
				regs->reg_f |= FLAG_CY;
			}
			else
			{
				regs->reg_a += 0x06u;
			}
		}
		else if ((regs->reg_f & FLAG_CY) == FLAG_CY || hi_bit >= 0xAu)
		{
			regs->reg_a += 0x60u;
			regs->reg_f |= FLAG_CY;
		}
	}
	else
	{
		if ((regs->reg_f & FLAG_H) == FLAG_H)
		{
			if ((regs->reg_f & FLAG_CY) == FLAG_CY)
			{
				regs->reg_a += 0x9Au;
			}
			else
			{
				regs->reg_a += 0xFAu;
			}
		}
		else if ((regs->reg_f & FLAG_CY) == FLAG_CY)
		{
			regs->reg_a += 0xA0u;
		}
	}
	regs->reg_f &= ~(FLAG_H);
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

#undef lo_bit
#undef hi_bit

// increment pc with 8-bit immediate value, if zero flag is set

jrz_imm8:
plog("jrz_imm8\n");
	if ((regs->reg_f & FLAG_Z) == FLAG_Z)
	{
		ADD_PC(2);
		ADD_PC((int8_t)imm_8);
		return (12);
	}
	else
	{
		ADD_PC(2);
		return (8);
	}

add_hl_hl:
plog("add_hl_hl\n");
	ADD_PC(1);
	regs->reg_f &= FLAG_Z;
	imm_16 = regs->reg_hl + regs->reg_hl;
	if (imm_16 < regs->reg_hl)
		regs->reg_f |= FLAG_CY;
	if ((imm_16 & 0xFFFu) < (regs->reg_hl & 0xFFFu))
		regs->reg_f |= FLAG_H;
	regs->reg_hl = imm_16;
	return (8);

ld_a_hli:
plog("ld_a_hli\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_hl += 1;
	regs->reg_a = *address;
	return (8);

dec_hl:
plog("dec_hl\n");
	ADD_PC(1);
	regs->reg_hl -= 1;
	return (8);

inc_l:
plog("inc_l\n");
	ADD_PC(1);
	regs->reg_f &= FLAG_CY;
	regs->reg_l += 1;
	if (regs->reg_l == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_l & 0xFu) == 0)
		regs->reg_f |= FLAG_H;
	return (4);

dec_l:
plog("dec_l\n");
	ADD_PC(1);
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_N;
	regs->reg_l -= 1;
	if (regs->reg_l == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_l & 0xFu) == 0xFu)
		regs->reg_f |= FLAG_H;
	return (4);

ld_l_imm8:
plog("ld_l_imm8\n");
	ADD_PC(2);
	regs->reg_l = imm_8;
	return (8);

// Take the complements of a, same as "not a"

cpl:
plog("cpl\n");
	ADD_PC(1);
	regs->reg_f |= FLAG_N | FLAG_H;
	regs->reg_a = ~(regs->reg_a);
	return (4);

// increment pc with 8-bit immediate value, if carry flag is not set

jrnc_imm8:
plog("jrnc_imm8\n");
	if ((regs->reg_f & FLAG_CY) == 0)
	{
		ADD_PC(2);
		ADD_PC((int8_t)imm_8);
		return (12);
	}
	else
	{
		ADD_PC(2);
		return (8);
	}

ld_sp_imm16:
plog("ld_sp_imm16\n");
	ADD_PC(3);
	regs->reg_sp = imm_16;
	return (12);

ld_hld_a:
plog("ld_hld_a\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_hl -= 1;
	WRITE_REGISTER_IF_ROM_AREA(regs->reg_hl - 1, regs->reg_a, 8);
	*address = regs->reg_a;
	return (8);

inc_sp:
plog("inc_sp\n");
	ADD_PC(1);
	regs->reg_sp += 1;
	return (8);

inc_ahl:
plog("inc_ahl\n");
	ADD_PC(1);
	regs->reg_f &= FLAG_CY;
	address = GET_REAL_ADDR(regs->reg_hl);
	*address += 1;
	if (*address == 0)
		regs->reg_f |= FLAG_Z;
	if ((*address & 0xFu) == 0)
		regs->reg_f |= FLAG_H;
	return (12);

dec_ahl:
plog("dec_ahl\n");
	ADD_PC(1);
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_N;
	address = GET_REAL_ADDR(regs->reg_hl);
	*address -= 1;
	if (*address == 0)
		regs->reg_f |= FLAG_Z;
	if ((*address & 0xFu) == 0xFu)
		regs->reg_f |= FLAG_H;
	return (12);

ld_hl_imm8:
plog("ld_hl_imm8\n");
	ADD_PC(2);
	address = GET_REAL_ADDR(regs->reg_hl);
	WRITE_REGISTER_IF_ROM_AREA(regs->reg_hl, imm_8, 12);
	*address = imm_8;
	return (12);

// Set carry flag

scf:
plog("scf\n");
	ADD_PC(1);
	regs->reg_f &= FLAG_Z;
	regs->reg_f |= FLAG_CY;
	return (4);

// increment pc with 8-bit immediate value, if carry flag is set

jrc_imm8:
plog("jrc_imm8\n");
	if ((regs->reg_f & FLAG_CY) == FLAG_CY)
	{
		ADD_PC(2);
		ADD_PC((int8_t)imm_8);
		return (12);
	}
	else
	{
		ADD_PC(2);
		return (8);
	}

add_hl_sp:
plog("add_hl_sp\n");
	ADD_PC(1);
	regs->reg_f &= FLAG_Z;
	imm_16 = regs->reg_hl + regs->reg_sp;
	if (imm_16 < regs->reg_hl)
		regs->reg_f |= FLAG_CY;
	if ((imm_16 & 0xFFFu) < (regs->reg_hl & 0xFFFu))
		regs->reg_f |= FLAG_H;
	regs->reg_hl = imm_16;
	return (8);

ld_a_hld:
plog("ld_a_hld\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_hl -= 1;
	regs->reg_a = *address;
	return (8);

dec_sp:
plog("dec_sp\n");
	ADD_PC(1);
	regs->reg_sp -= 1;
	return (8);

inc_a:
plog("inc_a\n");
	ADD_PC(1);
	regs->reg_f &= FLAG_CY;
	regs->reg_a += 1;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_a & 0xFu) == 0)
		regs->reg_f |= FLAG_H;
	return (4);

dec_a:
plog("dec_a\n");
	ADD_PC(1);
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_N;
	regs->reg_a -= 1;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_a & 0xFu) == 0xFu)
		regs->reg_f |= FLAG_H;
	return (4);

ld_a_imm8:
plog("ld_a_imm8\n");
	ADD_PC(2);
	regs->reg_a = imm_8;
	return (8);

// Takes the complements of carry flag, same as "not carry flag"

ccf:
plog("ccf\n");
	ADD_PC(1);
	regs->reg_f ^= FLAG_CY;
	regs->reg_f &= (FLAG_Z | FLAG_CY);
	return (4);

ld_b_b:
plog("ld_b_b\n");
	ADD_PC(1);
	regs->reg_b = regs->reg_b;
	return (4);

ld_b_c:
plog("ld_b_c\n");
	ADD_PC(1);
	regs->reg_b = regs->reg_c;
	return (4);

ld_b_d:
plog("ld_b_d\n");
	ADD_PC(1);
	regs->reg_b = regs->reg_d;
	return (4);

ld_b_e:
plog("ld_b_e\n");
	ADD_PC(1);
	regs->reg_b = regs->reg_e;
	return (4);

ld_b_h:
plog("ld_b_h\n");
	ADD_PC(1);
	regs->reg_b = regs->reg_h;
	return (4);

ld_b_l:
plog("ld_b_l\n");
	ADD_PC(1);
	regs->reg_b = regs->reg_l;
	return (4);

ld_b_hl:
plog("ld_b_hl\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_b = *address;
	return (8);

ld_b_a:
plog("ld_b_a\n");
	ADD_PC(1);
	regs->reg_b = regs->reg_a;
	return (4);

ld_c_b:
plog("ld_c_b\n");
	ADD_PC(1);
	regs->reg_c = regs->reg_b;
	return (4);

ld_c_c:
plog("ld_c_c\n");
	ADD_PC(1);
	regs->reg_c = regs->reg_c;
	return (4);

ld_c_d:
plog("ld_c_d\n");
	ADD_PC(1);
	regs->reg_c = regs->reg_d;
	return (4);

ld_c_e:
plog("ld_c_e\n");
	ADD_PC(1);
	regs->reg_c = regs->reg_e;
	return (4);

ld_c_h:
plog("ld_c_h\n");
	ADD_PC(1);
	regs->reg_c = regs->reg_h;
	return (4);

ld_c_l:
plog("ld_c_l\n");
	ADD_PC(1);
	regs->reg_c = regs->reg_l;
	return (4);

ld_c_hl:
plog("ld_c_hl\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_c = *address;
	return (8);

ld_c_a:
plog("ld_c_a\n");
	ADD_PC(1);
	regs->reg_c = regs->reg_a;
	return (4);

ld_d_b:
plog("ld_d_b\n");
	ADD_PC(1);
	regs->reg_d = regs->reg_b;
	return (4);

ld_d_c:
plog("ld_d_c\n");
	ADD_PC(1);
	regs->reg_d = regs->reg_c;
	return (4);

ld_d_d:
plog("ld_d_d\n");
	ADD_PC(1);
	regs->reg_d = regs->reg_d;
	return (4);

ld_d_e:
plog("ld_d_e\n");
	ADD_PC(1);
	regs->reg_d = regs->reg_e;
	return (4);

ld_d_h:
plog("ld_d_h\n");
	ADD_PC(1);
	regs->reg_d = regs->reg_h;
	return (4);

ld_d_l:
plog("ld_d_l\n");
	ADD_PC(1);
	regs->reg_d = regs->reg_l;
	return (4);

ld_d_hl:
plog("ld_d_hl\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_d = *address;
	return (8);

ld_d_a:
plog("ld_d_a\n");
	ADD_PC(1);
	regs->reg_d = regs->reg_a;
	return (4);

ld_e_b:
plog("ld_e_b\n");
	ADD_PC(1);
	regs->reg_e = regs->reg_b;
	return (4);

ld_e_c:
plog("ld_e_c\n");
	ADD_PC(1);
	regs->reg_e = regs->reg_c;
	return (4);

ld_e_d:
plog("ld_e_d\n");
	ADD_PC(1);
	regs->reg_e = regs->reg_d;
	return (4);

ld_e_e:
plog("ld_e_e\n");
	ADD_PC(1);
	regs->reg_e = regs->reg_e;
	return (4);

ld_e_h:
plog("ld_e_h\n");
	ADD_PC(1);
	regs->reg_e = regs->reg_h;
	return (4);

ld_e_l:
plog("ld_e_l\n");
	ADD_PC(1);
	regs->reg_e = regs->reg_l;
	return (4);

ld_e_hl:
plog("ld_e_hl\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_e = *address;
	return (8);

ld_e_a:
plog("ld_e_a\n");
	ADD_PC(1);
	regs->reg_e = regs->reg_a;
	return (4);

ld_h_b:
plog("ld_h_b\n");
	ADD_PC(1);
	regs->reg_h = regs->reg_b;
	return (4);

ld_h_c:
plog("ld_h_c\n");
	ADD_PC(1);
	regs->reg_h = regs->reg_c;
	return (4);

ld_h_d:
plog("ld_h_d\n");
	ADD_PC(1);
	regs->reg_h = regs->reg_d;
	return (4);

ld_h_e:
plog("ld_h_e\n");
	ADD_PC(1);
	regs->reg_h = regs->reg_e;
	return (4);

ld_h_h:
plog("ld_h_h\n");
	ADD_PC(1);
	regs->reg_h = regs->reg_h;
	return (4);

ld_h_l:
plog("ld_h_l\n");
	ADD_PC(1);
	regs->reg_h = regs->reg_l;
	return (4);

ld_h_hl:
plog("ld_h_hl\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_h = *address;
	return (8);

ld_h_a:
plog("ld_h_a\n");
	ADD_PC(1);
	regs->reg_h = regs->reg_a;
	return (4);

ld_l_b:
plog("ld_l_b\n");
	ADD_PC(1);
	regs->reg_l = regs->reg_b;
	return (4);

ld_l_c:
plog("ld_l_c\n");
	ADD_PC(1);
	regs->reg_l = regs->reg_c;
	return (4);

ld_l_d:
plog("ld_l_d\n");
	ADD_PC(1);
	regs->reg_l = regs->reg_d;
	return (4);

ld_l_e:
plog("ld_l_e\n");
	ADD_PC(1);
	regs->reg_l = regs->reg_e;
	return (4);

ld_l_h:
plog("ld_l_h\n");
	ADD_PC(1);
	regs->reg_l = regs->reg_h;
	return (4);

ld_l_l:
plog("ld_l_l\n");
	ADD_PC(1);
	regs->reg_l = regs->reg_l;
	return (4);

ld_l_hl:
plog("ld_l_hl\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_l = *address;
	return (8);

ld_l_a:
plog("ld_l_a\n");
	ADD_PC(1);
	regs->reg_l = regs->reg_a;
	return (4);

ld_hl_b:
plog("ld_hl_b\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_hl);
	WRITE_REGISTER_IF_ROM_AREA(regs->reg_hl, regs->reg_b, 8);
	*address = regs->reg_b;
	return (8);

ld_hl_c:
plog("ld_hl_c\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_hl);
	WRITE_REGISTER_IF_ROM_AREA(regs->reg_hl, regs->reg_c, 8);
	*address = regs->reg_c;
	return (8);

ld_hl_d:
plog("ld_hl_d\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_hl);
	WRITE_REGISTER_IF_ROM_AREA(regs->reg_hl, regs->reg_d, 8);
	*address = regs->reg_d;
	return (8);

ld_hl_e:
plog("ld_hl_e\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_hl);
	WRITE_REGISTER_IF_ROM_AREA(regs->reg_hl, regs->reg_e, 8);
	*address = regs->reg_e;
	return (8);

ld_hl_h:
plog("ld_hl_h\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_hl);
	WRITE_REGISTER_IF_ROM_AREA(regs->reg_hl, regs->reg_h, 8);
	*address = regs->reg_h;
	return (8);

ld_hl_l:
plog("ld_hl_l\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_hl);
	WRITE_REGISTER_IF_ROM_AREA(regs->reg_hl, regs->reg_l, 8);
	*address = regs->reg_l;
	return (8);

// stop processor until interrupt flag is ok

halt:
plog("halt\n");
	GAMEBOY = HALT_MODE;
	ADD_PC(1);
	return (4);

ld_hl_a:
plog("ld_hl_a\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_hl);
	WRITE_REGISTER_IF_ROM_AREA(regs->reg_hl, regs->reg_a, 8);
	*address = regs->reg_a;
	return (8);

ld_a_b:
plog("ld_a_b\n");
	ADD_PC(1);
	regs->reg_a = regs->reg_b;
	return (4);

ld_a_c:
plog("ld_a_c\n");
	ADD_PC(1);
	regs->reg_a = regs->reg_c;
	return (4);

ld_a_d:
plog("ld_a_d\n");
	ADD_PC(1);
	regs->reg_a = regs->reg_d;
	return (4);

ld_a_e:
plog("ld_a_e\n");
	ADD_PC(1);
	regs->reg_a = regs->reg_e;
	return (4);

ld_a_h:
plog("ld_a_h\n");
	ADD_PC(1);
	regs->reg_a = regs->reg_h;
	return (4);

ld_a_l:
plog("ld_a_l\n");
	ADD_PC(1);
	regs->reg_a = regs->reg_l;
	return (4);

ld_a_hl:
plog("ld_a_hl\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_a = *address;
	return (8);

ld_a_a:
plog("ld_a_a\n");
	ADD_PC(1);
	regs->reg_a = regs->reg_a;
	return (4);

add_a_b:
plog("add_a_b\n");
	ADD_PC(1);
	regs->reg_f = 0;
	imm_8 = regs->reg_a + regs->reg_b;
	if (imm_8 < regs->reg_a)
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

add_a_c:
plog("add_a_c\n");
	ADD_PC(1);
	regs->reg_f = 0;
	imm_8 = regs->reg_a + regs->reg_c;
	if (imm_8 < regs->reg_a)
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

add_a_d:
plog("add_a_d\n");
	ADD_PC(1);
	regs->reg_f = 0;
	imm_8 = regs->reg_a + regs->reg_d;
	if (imm_8 < regs->reg_a)
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

add_a_e:
plog("add_a_e\n");
	ADD_PC(1);
	regs->reg_f = 0;
	imm_8 = regs->reg_a + regs->reg_e;
	if (imm_8 < regs->reg_a)
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

add_a_h:
plog("add_a_h\n");
	ADD_PC(1);
	regs->reg_f = 0;
	imm_8 = regs->reg_a + regs->reg_h;
	if (imm_8 < regs->reg_a)
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

add_a_l:
plog("add_a_l\n");
	ADD_PC(1);
	regs->reg_f = 0;
	imm_8 = regs->reg_a + regs->reg_l;
	if (imm_8 < regs->reg_a)
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

add_a_hl:
plog("add_a_hl\n");
	ADD_PC(1);
	regs->reg_f = 0;
	address = GET_REAL_ADDR(regs->reg_hl);
	imm_8 = regs->reg_a + *address;
	if (imm_8 < regs->reg_a)
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (8);

add_a_a:
plog("add_a_a\n");
	ADD_PC(1);
	regs->reg_f = 0;
	imm_8 = regs->reg_a + regs->reg_a;
	if (imm_8 < regs->reg_a)
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

#define tmpflag	opcode

adc_a_b:
plog("adc_a_b\n");
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = 0;
	imm_8 = regs->reg_a + regs->reg_b;
	imm_8 += ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 < regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu) ||
		((imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

adc_a_c:
plog("adc_a_c\n");
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = 0;
	imm_8 = regs->reg_a + regs->reg_c;
	imm_8 += ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 < regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu) ||
		((imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

adc_a_d:
plog("adc_a_d\n");
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = 0;
	imm_8 = regs->reg_a + regs->reg_d;
	imm_8 += ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 < regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu) ||
		((imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

adc_a_e:
plog("adc_a_e\n");
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = 0;
	imm_8 = regs->reg_a + regs->reg_e;
	imm_8 += ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 < regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu) ||
		((imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

adc_a_h:
plog("adc_a_h\n");
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = 0;
	imm_8 = regs->reg_a + regs->reg_h;
	imm_8 += ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 < regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu) ||
		((imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

adc_a_l:
plog("adc_a_l\n");
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = 0;
	imm_8 = regs->reg_a + regs->reg_l;
	imm_8 += ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 < regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu) ||
		((imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

adc_a_hl:
plog("adc_a_hl\n");
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = 0;
	address = GET_REAL_ADDR(regs->reg_hl);
	imm_8 = regs->reg_a + *address;
	imm_8 += ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 < regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu) ||
		((imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (8);

adc_a_a:
plog("adc_a_a\n");
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = 0;
	imm_8 = regs->reg_a + regs->reg_a;
	imm_8 += ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 < regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu) ||
		((imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

sub_a_b:
plog("sub_a_b\n");
	ADD_PC(1);
	regs->reg_f = FLAG_N;
	imm_8 = regs->reg_a - regs->reg_b;
	if (imm_8 > regs->reg_a)
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

sub_a_c:
plog("sub_a_c\n");
	ADD_PC(1);
	regs->reg_f = FLAG_N;
	imm_8 = regs->reg_a - regs->reg_c;
	if (imm_8 > regs->reg_a)
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

sub_a_d:
plog("sub_a_d\n");
	ADD_PC(1);
	regs->reg_f = FLAG_N;
	imm_8 = regs->reg_a - regs->reg_d;
	if (imm_8 > regs->reg_a)
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

sub_a_e:
plog("sub_a_e\n");
	ADD_PC(1);
	regs->reg_f = FLAG_N;
	imm_8 = regs->reg_a - regs->reg_e;
	if (imm_8 > regs->reg_a)
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

sub_a_h:
plog("sub_a_h\n");
	ADD_PC(1);
	regs->reg_f = FLAG_N;
	imm_8 = regs->reg_a - regs->reg_h;
	if (imm_8 > regs->reg_a)
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

sub_a_l:
plog("sub_a_l\n");
	ADD_PC(1);
	regs->reg_f = FLAG_N;
	imm_8 = regs->reg_a - regs->reg_l;
	if (imm_8 > regs->reg_a)
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

sub_a_hl:
plog("sub_a_hl\n");
	ADD_PC(1);
	regs->reg_f = FLAG_N;
	address = GET_REAL_ADDR(regs->reg_hl);
	imm_8 = regs->reg_a - *address;
	if (imm_8 > regs->reg_a)
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (8);

sub_a_a:
plog("sub_a_a\n");
	ADD_PC(1);
	regs->reg_f = FLAG_N;
	imm_8 = regs->reg_a - regs->reg_a;
	if (imm_8 > regs->reg_a)
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

sbc_a_b:
plog("sbc_a_b\n");
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = FLAG_N;
	imm_8 = regs->reg_a - regs->reg_b;
	imm_8 -= ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 > regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu) ||
		((imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

sbc_a_c:
plog("sbc_a_c\n");
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = FLAG_N;
	imm_8 = regs->reg_a - regs->reg_c;
	imm_8 -= ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 > regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu) ||
		((imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

sbc_a_d:
plog("sbc_a_d\n");
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = FLAG_N;
	imm_8 = regs->reg_a - regs->reg_d;
	imm_8 -= ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 > regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu) ||
		((imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

sbc_a_e:
plog("sbc_a_e\n");
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = FLAG_N;
	imm_8 = regs->reg_a - regs->reg_e;
	imm_8 -= ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 > regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu) ||
		((imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

sbc_a_h:
plog("sbc_a_h\n");
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = FLAG_N;
	imm_8 = regs->reg_a - regs->reg_h;
	imm_8 -= ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 > regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu) ||
		((imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

sbc_a_l:
plog("sbc_a_l\n");
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = FLAG_N;
	imm_8 = regs->reg_a - regs->reg_l;
	imm_8 -= ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 > regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu) ||
		((imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

sbc_a_hl:
plog("sbc_a_hl\n");
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = FLAG_N;
	address = GET_REAL_ADDR(regs->reg_hl);
	imm_8 = regs->reg_a - *address;
	imm_8 -= ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 > regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu) ||
		((imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (8);

sbc_a_a:
plog("sbc_a_a\n");
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = FLAG_N;
	imm_8 = regs->reg_a - regs->reg_a;
	imm_8 -= ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 > regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu) ||
		((imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

and_a_b:
plog("and_a_b\n");
	ADD_PC(1);
	regs->reg_f = FLAG_H;
	regs->reg_a &= regs->reg_b;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

and_a_c:
plog("and_a_c\n");
	ADD_PC(1);
	regs->reg_f = FLAG_H;
	regs->reg_a &= regs->reg_c;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

and_a_d:
plog("and_a_d\n");
	ADD_PC(1);
	regs->reg_f = FLAG_H;
	regs->reg_a &= regs->reg_d;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

and_a_e:
plog("and_a_e\n");
	ADD_PC(1);
	regs->reg_f = FLAG_H;
	regs->reg_a &= regs->reg_e;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

and_a_h:
plog("and_a_h\n");
	ADD_PC(1);
	regs->reg_f = FLAG_H;
	regs->reg_a &= regs->reg_h;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

and_a_l:
plog("and_a_l\n");
	ADD_PC(1);
	regs->reg_f = FLAG_H;
	regs->reg_a &= regs->reg_l;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

and_a_hl:
plog("and_a_hl\n");
	ADD_PC(1);
	regs->reg_f = FLAG_H;
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_a &= *address;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

and_a_a:
plog("and_a_a\n");
	ADD_PC(1);
	regs->reg_f = FLAG_H;
	regs->reg_a &= regs->reg_a;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

xor_a_b:
plog("xor_a_b\n");
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a ^= regs->reg_b;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

xor_a_c:
plog("xor_a_c\n");
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a ^= regs->reg_c;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

xor_a_d:
plog("xor_a_d\n");
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a ^= regs->reg_d;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

xor_a_e:
plog("xor_a_e\n");
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a ^= regs->reg_e;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

xor_a_h:
plog("xor_a_h\n");
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a ^= regs->reg_h;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

xor_a_l:
plog("xor_a_l\n");
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a ^= regs->reg_l;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

xor_a_hl:
plog("xor_a_hl\n");
	ADD_PC(1);
	regs->reg_f = 0;
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_a ^= *address;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

xor_a_a:
plog("xor_a_a\n");
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a ^= regs->reg_a;
//	if (regs->reg_a == 0)
	regs->reg_f |= FLAG_Z;
	return (4);

or_a_b:
plog("or_a_b\n");
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a |= regs->reg_b;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

or_a_c:
plog("or_a_c\n");
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a |= regs->reg_c;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

or_a_d:
plog("or_a_d\n");
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a |= regs->reg_d;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

or_a_e:
plog("or_a_e\n");
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a |= regs->reg_e;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

or_a_h:
plog("or_a_h\n");
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a |= regs->reg_h;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

or_a_l:
plog("or_a_l\n");
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a |= regs->reg_l;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

or_a_hl:
plog("or_a_hl\n");
	ADD_PC(1);
	regs->reg_f = 0;
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_a |= *address;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

or_a_a:
plog("or_a_a\n");
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a |= regs->reg_a;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

cp_a_b:
plog("cp_a_b\n");
	ADD_PC(1);
	regs->reg_f = FLAG_N;
	if (regs->reg_a == regs->reg_b)
		regs->reg_f |= FLAG_Z;
	if (regs->reg_a < regs->reg_b)
		regs->reg_f |= FLAG_CY;
	if ((regs->reg_a & 0xFu) < (regs->reg_b & 0xFu))
		regs->reg_f |= FLAG_H;
	return (4);

cp_a_c:
plog("cp_a_c\n");
	ADD_PC(1);
	regs->reg_f = FLAG_N;
	if (regs->reg_a == regs->reg_c)
		regs->reg_f |= FLAG_Z;
	if (regs->reg_a < regs->reg_c)
		regs->reg_f |= FLAG_CY;
	if ((regs->reg_a & 0xFu) < (regs->reg_c & 0xFu))
		regs->reg_f |= FLAG_H;
	return (4);

cp_a_d:
plog("cp_a_d\n");
	ADD_PC(1);
	regs->reg_f = FLAG_N;
	if (regs->reg_a == regs->reg_d)
		regs->reg_f |= FLAG_Z;
	if (regs->reg_a < regs->reg_d)
		regs->reg_f |= FLAG_CY;
	if ((regs->reg_a & 0xFu) < (regs->reg_d & 0xFu))
		regs->reg_f |= FLAG_H;
	return (4);

cp_a_e:
plog("cp_a_e\n");
	ADD_PC(1);
	regs->reg_f = FLAG_N;
	if (regs->reg_a == regs->reg_e)
		regs->reg_f |= FLAG_Z;
	if (regs->reg_a < regs->reg_e)
		regs->reg_f |= FLAG_CY;
	if ((regs->reg_a & 0xFu) < (regs->reg_e & 0xFu))
		regs->reg_f |= FLAG_H;
	return (4);

cp_a_h:
plog("cp_a_h\n");
	ADD_PC(1);
	regs->reg_f = FLAG_N;
	if (regs->reg_a == regs->reg_h)
		regs->reg_f |= FLAG_Z;
	if (regs->reg_a < regs->reg_h)
		regs->reg_f |= FLAG_CY;
	if ((regs->reg_a & 0xFu) < (regs->reg_h & 0xFu))
		regs->reg_f |= FLAG_H;
	return (4);

cp_a_l:
plog("cp_a_l\n");
	ADD_PC(1);
	regs->reg_f = FLAG_N;
	if (regs->reg_a == regs->reg_l)
		regs->reg_f |= FLAG_Z;
	if (regs->reg_a < regs->reg_l)
		regs->reg_f |= FLAG_CY;
	if ((regs->reg_a & 0xFu) < (regs->reg_l & 0xFu))
		regs->reg_f |= FLAG_H;
	return (4);

cp_a_hl:
plog("cp_a_hl\n");
	ADD_PC(1);
	regs->reg_f = FLAG_N;
	address = GET_REAL_ADDR(regs->reg_hl);
	if (regs->reg_a == *address)
		regs->reg_f |= FLAG_Z;
	if (regs->reg_a < *address)
		regs->reg_f |= FLAG_CY;
	if ((regs->reg_a & 0xFu) < (*address & 0xFu))
		regs->reg_f |= FLAG_H;
	return (8);

cp_a_a:
plog("cp_a_a\n");
	ADD_PC(1);
	regs->reg_f = FLAG_N;
	if (regs->reg_a == regs->reg_a)
		regs->reg_f |= FLAG_Z;
	if (regs->reg_a < regs->reg_a)
		regs->reg_f |= FLAG_CY;
	if ((regs->reg_a & 0xFu) < (regs->reg_a & 0xFu))
		regs->reg_f |= FLAG_H;
	return (4);

retnz:
plog("retnz\n");
	if ((regs->reg_f & FLAG_Z) == 0)
	{
		address = GET_REAL_ADDR(regs->reg_sp);
		regs->reg_sp += 2;
		SET_PC(((uint16_t)address[1] << 8) | (uint16_t)address[0]);
		return (20);
	}
	else
	{
		ADD_PC(1);
		return (8);
	}

pop_bc:
plog("pop_bc\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_sp);
	regs->reg_sp += 2;
	regs->reg_bc = ((uint16_t)address[1] << 8) | (uint16_t)address[0];
	return (12);

jpnz_imm16:
plog("jpnz_imm16\n");
	if ((regs->reg_f & FLAG_Z) == 0)
	{
		SET_PC(imm_16);
		return (16);
	}
	else
	{
		ADD_PC(3);
		return (12);
	}

jp_imm16:
plog("jp_imm16\n");
	SET_PC(imm_16);
	return (16);

callnz_imm16:
plog("callnz_imm16\n");
	ADD_PC(3);
	if ((regs->reg_f & FLAG_Z) == 0)
	{
		address = GET_REAL_ADDR(regs->reg_sp);
		regs->reg_sp -= 2;
		address[-1] = (uint8_t)(regs->reg_pc >> 8);
		address[-2] = (uint8_t)(regs->reg_pc);
		SET_PC(imm_16);
		return (24);
	}
	else
		return (12);

push_bc:
plog("push_bc\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_sp);
	regs->reg_sp -= 2;
	address[-1] = (uint8_t)(regs->reg_bc >> 8);
	address[-2] = (uint8_t)(regs->reg_bc);
	return (16);

add_a_imm8:
plog("add_a_imm8\n");
	ADD_PC(2);
	regs->reg_f = 0;
	imm_8 += regs->reg_a;
	if (imm_8 < regs->reg_a)
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

rst_00h:
plog("rst_00h\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_sp);
	regs->reg_sp -= 2;
	address[-1] = (uint8_t)(regs->reg_pc >> 8);
	address[-2] = (uint8_t)(regs->reg_pc);
	regs->reg_pc = 0x00u;
	return (16);

retz:
plog("retz\n");
	if ((regs->reg_f & FLAG_Z) == FLAG_Z)
	{
		address = GET_REAL_ADDR(regs->reg_sp);
		regs->reg_sp += 2;
		SET_PC(((uint16_t)address[1] << 8) | (uint16_t)address[0]);
		return (20);
	}
	else
	{
		ADD_PC(1);
		return (8);
	}

ret:
plog("ret\n");
	address = GET_REAL_ADDR(regs->reg_sp);
	regs->reg_sp += 2;
	SET_PC(((uint16_t)address[1] << 8) | (uint16_t)address[0]);
	return (20);

jpz_imm16:
plog("jpz_imm16\n");
	if ((regs->reg_f & FLAG_Z) == FLAG_Z)
	{
		SET_PC(imm_16);
		return (16);
	}
	else
	{
		ADD_PC(3);
		return (12);
	}

callz_imm16:
plog("callz_imm16\n");
	ADD_PC(3);
	if ((regs->reg_f & FLAG_Z) == FLAG_Z)
	{
		address = GET_REAL_ADDR(regs->reg_sp);
		regs->reg_sp -= 2;
		address[-1] = (uint8_t)(regs->reg_pc >> 8);
		address[-2] = (uint8_t)(regs->reg_pc);
		SET_PC(imm_16);
		return (24);
	}
	else
		return (12);

call_imm16:
plog("call_imm16\n");
	ADD_PC(3);
	address = GET_REAL_ADDR(regs->reg_sp);
	regs->reg_sp -= 2;
	address[-1] = (uint8_t)(regs->reg_pc >> 8);
	address[-2] = (uint8_t)(regs->reg_pc);
	SET_PC(imm_16);
	return (24);

adc_a_imm8:
plog("adc_a_imm8\n");
	ADD_PC(2);
	tmpflag = regs->reg_f;
	regs->reg_f = 0;
	imm_8 += regs->reg_a;
	imm_8 += ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 < regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu) ||
		((imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

rst_08h:
plog("rst_08h\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_sp);
	regs->reg_sp -= 2;
	address[-1] = (uint8_t)(regs->reg_pc >> 8);
	address[-2] = (uint8_t)(regs->reg_pc);
	regs->reg_pc = 0x08u;
	return (16);

retnc:
plog("retnc\n");
	if ((regs->reg_f & FLAG_CY) == 0)
	{
		address = GET_REAL_ADDR(regs->reg_sp);
		regs->reg_sp += 2;
		SET_PC(((uint16_t)address[1] << 8) | (uint16_t)address[0]);
		return (20);
	}
	else
	{
		ADD_PC(1);
		return (8);
	}

pop_de:
plog("pop_de\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_sp);
	regs->reg_sp += 2;
	regs->reg_de = ((uint16_t)address[1] << 8) | (uint16_t)address[0];
	return (12);

jpnc_imm16:
plog("jpnc_imm16\n");
	if ((regs->reg_f & FLAG_CY) == 0)
	{
		SET_PC(imm_16);
		return (16);
	}
	else
	{
		ADD_PC(3);
		return (12);
	}

callnc_imm16:
plog("callnc_imm16\n");
	ADD_PC(3);
	if ((regs->reg_f & FLAG_CY) == 0)
	{
		address = GET_REAL_ADDR(regs->reg_sp);
		regs->reg_sp -= 2;
		address[-1] = (uint8_t)(regs->reg_pc >> 8);
		address[-2] = (uint8_t)(regs->reg_pc);
		SET_PC(imm_16);
		return (24);
	}
	else
		return (12);

push_de:
plog("push_de\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_sp);
	regs->reg_sp -= 2;
	address[-1] = (uint8_t)(regs->reg_de >> 8);
	address[-2] = (uint8_t)(regs->reg_de);
	return (16);

sub_a_imm8:
plog("sub_a_imm8\n");
	ADD_PC(2);
	regs->reg_f = FLAG_N;
	imm_8 = regs->reg_a - imm_8;
	if (imm_8 > regs->reg_a)
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

rst_10h:
plog("rst_10h\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_sp);
	regs->reg_sp -= 2;
	address[-1] = (uint8_t)(regs->reg_pc >> 8);
	address[-2] = (uint8_t)(regs->reg_pc);
	regs->reg_pc = 0x10u;
	return (16);

retc:
plog("retc\n");
	if ((regs->reg_f & FLAG_CY) == FLAG_CY)
	{
		address = GET_REAL_ADDR(regs->reg_sp);
		regs->reg_sp += 2;
		SET_PC(((uint16_t)address[1] << 8) | (uint16_t)address[0]);
		return (20);
	}
	else
	{
		ADD_PC(1);
		return (8);
	}

reti:
plog("reti\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_sp);
	regs->reg_sp += 2;
	SET_PC(((uint16_t)address[1] << 8) | (uint16_t)address[0]);
	g_memmap.ime = true;
	return (16);

jpc_imm16:
plog("jpc_imm16\n");
	if ((regs->reg_f & FLAG_CY) == FLAG_CY)
	{
		SET_PC(imm_16);
		return (16);
	}
	else
	{
		ADD_PC(3);
		return (12);
	}

callc_imm16:
plog("callc_imm16\n");
	ADD_PC(3);
	if ((regs->reg_f & FLAG_CY) == FLAG_CY)
	{
		address = GET_REAL_ADDR(regs->reg_sp);
		regs->reg_sp -= 2;
		address[-1] = (uint8_t)(regs->reg_pc >> 8);
		address[-2] = (uint8_t)(regs->reg_pc);
		SET_PC(imm_16);
		return (24);
	}
	else
		return (12);

sbc_a_imm8:
plog("sbc_a_imm8\n");
	ADD_PC(2);
	tmpflag = regs->reg_f;
	regs->reg_f = FLAG_N;
	imm_8 = regs->reg_a - imm_8;
	imm_8 -= ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 > regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu) ||
		((imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

rst_18h:
plog("rst_18h\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_sp);
	regs->reg_sp -= 2;
	address[-1] = (uint8_t)(regs->reg_pc >> 8);
	address[-2] = (uint8_t)(regs->reg_pc);
	regs->reg_pc = 0x18u;
	return (16);

ldff_imm8_a:
plog("ldff_imm8_a\n");
	ADD_PC(2);
	address = GET_REAL_ADDR(0xFF00u + (uint16_t)imm_8);
	*address = regs->reg_a;
	return (8);

pop_hl:
plog("pop_hl\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_sp);
	regs->reg_sp += 2;
	regs->reg_hl = ((uint16_t)address[1] << 8) | (uint16_t)address[0];
	return (12);

ldff_c_a:
plog("ldff_c_a\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(0xFF00u + (uint16_t)regs->reg_c);
	*address = regs->reg_a;
	return (8);

push_hl:
plog("push_hl\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_sp);
	regs->reg_sp -= 2;
	address[-1] = (uint8_t)(regs->reg_hl >> 8);
	address[-2] = (uint8_t)(regs->reg_hl);
	return (16);

and_a_imm8:
plog("and_a_imm8\n");
	ADD_PC(2);
	regs->reg_f = FLAG_H;
	regs->reg_a &= imm_8;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

rst_20h:
plog("rst_20h\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_sp);
	regs->reg_sp -= 2;
	address[-1] = (uint8_t)(regs->reg_pc >> 8);
	address[-2] = (uint8_t)(regs->reg_pc);
	regs->reg_pc = 0x20u;
	return (16);

add_sp_imm8:
plog("add_sp_imm8\n");
	ADD_PC(2);
	regs->reg_f = 0;
	imm_16 = regs->reg_sp + imm_8;
	if (imm_16 < regs->reg_sp)
		regs->reg_f |= FLAG_CY;
	if ((imm_16 & 0xFFFu) < (regs->reg_sp & 0xFFFu))
		regs->reg_f |= FLAG_H;
	regs->reg_sp = imm_16;
	return (16);

jp_hl:
plog("jp_hl\n");
	SET_PC(regs->reg_hl);
	return (4);

ld_imm16_a:
plog("ld_imm16_a\n");
	ADD_PC(3);
	address = GET_REAL_ADDR(imm_16);
	WRITE_REGISTER_IF_ROM_AREA(imm_16, regs->reg_a, 16);
	*address = regs->reg_a;
	return (16);

xor_a_imm8:
plog("xor_a_imm8\n");
	ADD_PC(2);
	regs->reg_f = 0;
	regs->reg_a ^= imm_8;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

rst_28h:
plog("rst_28h\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_sp);
	regs->reg_sp -= 2;
	address[-1] = (uint8_t)(regs->reg_pc >> 8);
	address[-2] = (uint8_t)(regs->reg_pc);
	regs->reg_pc = 0x28u;
	return (16);

ldff_a_imm8:
plog("ldff_a_imm8\n");
	ADD_PC(2);
	address = GET_REAL_ADDR(0xFF00u + (uint16_t)imm_8);
	regs->reg_a = *address;
	return (8);

pop_af:
plog("pop_af\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_sp);
	regs->reg_sp += 2;
	regs->reg_af = ((uint16_t)address[1] << 8) | (uint16_t)address[0];
	return (12);

ldff_a_c:
plog("ldff_a_c\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(0xFF00u + (uint16_t)regs->reg_c);
	regs->reg_a = *address;
	return (8);

di:
plog("di\n");
	ADD_PC(1);
	g_memmap.ime = false;
	return (4);

push_af:
plog("push_af\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_sp);
	regs->reg_sp -= 2;
	address[-1] = (uint8_t)(regs->reg_af >> 8);
	address[-2] = (uint8_t)(regs->reg_af);
	return (16);

or_a_imm8:
plog("or_a_imm8\n");
	ADD_PC(2);
	regs->reg_f = 0;
	regs->reg_a |= imm_8;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

rst_30h:
plog("rst_30h\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_sp);
	regs->reg_sp -= 2;
	address[-1] = (uint8_t)(regs->reg_pc >> 8);
	address[-2] = (uint8_t)(regs->reg_pc);
	regs->reg_pc = 0x30u;
	return (16);

ld_hl_sp_imm8:
plog("ld_hl_sp_imm8\n");
	ADD_PC(2);
	regs->reg_f = 0;
	imm_16 = regs->reg_sp + imm_8;
	if (imm_16 < regs->reg_sp)
		regs->reg_f |= FLAG_CY;
	if ((imm_16 & 0xFFFu) < (regs->reg_sp & 0xFFFu))
		regs->reg_f |= FLAG_H;
	regs->reg_hl = imm_16;
	return (12);

ld_sp_hl:
plog("ld_sp_hl\n");
	ADD_PC(1);
	regs->reg_hl = regs->reg_sp;
	return (8);

ld_a_imm16:
plog("ld_a_imm16\n");
	ADD_PC(3);
	address = GET_REAL_ADDR(imm_16);
	regs->reg_a = *address;
	return (16);

ei:
plog("ei\n");
	ADD_PC(1);
	g_memmap.ime = true;
	return (4);

cp_a_imm8:
plog("cp_a_imm8\n");
	ADD_PC(2);
	regs->reg_f = FLAG_N;
	if (regs->reg_a == imm_8)
		regs->reg_f |= FLAG_Z;
	if (regs->reg_a < imm_8)
		regs->reg_f |= FLAG_CY;
	if ((regs->reg_a & 0xFu) < (imm_8 & 0xFu))
		regs->reg_f |= FLAG_H;
	return (4);

rst_38h:
plog("rst_38h\n");
	ADD_PC(1);
	address = GET_REAL_ADDR(regs->reg_sp);
	regs->reg_sp -= 2;
	address[-1] = (uint8_t)(regs->reg_pc >> 8);
	address[-2] = (uint8_t)(regs->reg_pc);
	regs->reg_pc = 0x38u;
	return (16);

illegal:
plog("illegal\n");
	ADD_PC(1);
	return (0);

prefix_cb:
plog("prefix_cb\n");
	ADD_PC(2);

	static const void *const	bitboard_tables[256] = 
	{
		&&rlc_b,			&&rlc_c,			&&rlc_d,			&&rlc_e,
		&&rlc_h,			&&rlc_l,			&&rlc_hl,			&&rlc_a,
		&&rrc_b,			&&rrc_c,			&&rrc_d,			&&rrc_e,
		&&rrc_h,			&&rrc_l,			&&rrc_hl,			&&rrc_a,
		&&rl_b,				&&rl_c,				&&rl_d,				&&rl_e,
		&&rl_h,				&&rl_l,				&&rl_hl,			&&rl_a,
		&&rr_b,				&&rr_c,				&&rr_d,				&&rr_e,
		&&rr_h,				&&rr_l,				&&rr_hl,			&&rr_a,
		&&sla_b,			&&sla_c,			&&sla_d,			&&sla_e,
		&&sla_h,			&&sla_l,			&&sla_hl,			&&sla_a,
		&&sra_b,			&&sra_c,			&&sra_d,			&&sra_e,
		&&sra_h,			&&sra_l,			&&sra_hl,			&&sra_a,
		&&swap_b,			&&swap_c,			&&swap_d,			&&swap_e,
		&&swap_h,			&&swap_l,			&&swap_hl,			&&swap_a,
		&&srl_b,			&&srl_c,			&&srl_d,			&&srl_e,
		&&srl_h,			&&srl_l,			&&srl_hl,			&&srl_a,
		&&bit_0_b,			&&bit_0_c,			&&bit_0_d,			&&bit_0_e,
		&&bit_0_h,			&&bit_0_l,			&&bit_0_hl,			&&bit_0_a,
		&&bit_1_b,			&&bit_1_c,			&&bit_1_d,			&&bit_1_e,
		&&bit_1_h,			&&bit_1_l,			&&bit_1_hl,			&&bit_1_a,
		&&bit_2_b,			&&bit_2_c,			&&bit_2_d,			&&bit_2_e,
		&&bit_2_h,			&&bit_2_l,			&&bit_2_hl,			&&bit_2_a,
		&&bit_3_b,			&&bit_3_c,			&&bit_3_d,			&&bit_3_e,
		&&bit_3_h,			&&bit_3_l,			&&bit_3_hl,			&&bit_3_a,
		&&bit_4_b,			&&bit_4_c,			&&bit_4_d,			&&bit_4_e,
		&&bit_4_h,			&&bit_4_l,			&&bit_4_hl,			&&bit_4_a,
		&&bit_5_b,			&&bit_5_c,			&&bit_5_d,			&&bit_5_e,
		&&bit_5_h,			&&bit_5_l,			&&bit_5_hl,			&&bit_5_a,
		&&bit_6_b,			&&bit_6_c,			&&bit_6_d,			&&bit_6_e,
		&&bit_6_h,			&&bit_6_l,			&&bit_6_hl,			&&bit_6_a,
		&&bit_7_b,			&&bit_7_c,			&&bit_7_d,			&&bit_7_e,
		&&bit_7_h,			&&bit_7_l,			&&bit_7_hl,			&&bit_7_a,
		&&res_0_b,			&&res_0_c,			&&res_0_d,			&&res_0_e,
		&&res_0_h,			&&res_0_l,			&&res_0_hl,			&&res_0_a,
		&&res_1_b,			&&res_1_c,			&&res_1_d,			&&res_1_e,
		&&res_1_h,			&&res_1_l,			&&res_1_hl,			&&res_1_a,
		&&res_2_b,			&&res_2_c,			&&res_2_d,			&&res_2_e,
		&&res_2_h,			&&res_2_l,			&&res_2_hl,			&&res_2_a,
		&&res_3_b,			&&res_3_c,			&&res_3_d,			&&res_3_e,
		&&res_3_h,			&&res_3_l,			&&res_3_hl,			&&res_3_a,
		&&res_4_b,			&&res_4_c,			&&res_4_d,			&&res_4_e,
		&&res_4_h,			&&res_4_l,			&&res_4_hl,			&&res_4_a,
		&&res_5_b,			&&res_5_c,			&&res_5_d,			&&res_5_e,
		&&res_5_h,			&&res_5_l,			&&res_5_hl,			&&res_5_a,
		&&res_6_b,			&&res_6_c,			&&res_6_d,			&&res_6_e,
		&&res_6_h,			&&res_6_l,			&&res_6_hl,			&&res_6_a,
		&&res_7_b,			&&res_7_c,			&&res_7_d,			&&res_7_e,
		&&res_7_h,			&&res_7_l,			&&res_7_hl,			&&res_7_a,
		&&set_0_b,			&&set_0_c,			&&set_0_d,			&&set_0_e,
		&&set_0_h,			&&set_0_l,			&&set_0_hl,			&&set_0_a,
		&&set_1_b,			&&set_1_c,			&&set_1_d,			&&set_1_e,
		&&set_1_h,			&&set_1_l,			&&set_1_hl,			&&set_1_a,
		&&set_2_b,			&&set_2_c,			&&set_2_d,			&&set_2_e,
		&&set_2_h,			&&set_2_l,			&&set_2_hl,			&&set_2_a,
		&&set_3_b,			&&set_3_c,			&&set_3_d,			&&set_3_e,
		&&set_3_h,			&&set_3_l,			&&set_3_hl,			&&set_3_a,
		&&set_4_b,			&&set_4_c,			&&set_4_d,			&&set_4_e,
		&&set_4_h,			&&set_4_l,			&&set_4_hl,			&&set_4_a,
		&&set_5_b,			&&set_5_c,			&&set_5_d,			&&set_5_e,
		&&set_5_h,			&&set_5_l,			&&set_5_hl,			&&set_5_a,
		&&set_6_b,			&&set_6_c,			&&set_6_d,			&&set_6_e,
		&&set_6_h,			&&set_6_l,			&&set_6_hl,			&&set_6_a,
		&&set_7_b,			&&set_7_c,			&&set_7_d,			&&set_7_e,
		&&set_7_h,			&&set_7_l,			&&set_7_hl,			&&set_7_a,
	};

	goto *bitboard_tables[imm_8];

rlc_b:
plog("rlc_b\n");
	imm_8 = ((regs->reg_b & BIT_7) == BIT_7) ? BIT_0 : 0;
	regs->reg_f = imm_8 << 4;
	if (regs->reg_b == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_b <<= 1;
	regs->reg_b |= imm_8;
	return (8);

rlc_c:
plog("rlc_c\n");
	imm_8 = ((regs->reg_c & BIT_7) == BIT_7) ? BIT_0 : 0;
	regs->reg_f = imm_8 << 4;
	if (regs->reg_c == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_c <<= 1;
	regs->reg_c |= imm_8;
	return (8);

rlc_d:
plog("rlc_d\n");
	imm_8 = ((regs->reg_d & BIT_7) == BIT_7) ? BIT_0 : 0;
	regs->reg_f = imm_8 << 4;
	if (regs->reg_d == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_d <<= 1;
	regs->reg_d |= imm_8;
	return (8);

rlc_e:
plog("rlc_e\n");
	imm_8 = ((regs->reg_e & BIT_7) == BIT_7) ? BIT_0 : 0;
	regs->reg_f = imm_8 << 4;
	if (regs->reg_e == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_e <<= 1;
	regs->reg_e |= imm_8;
	return (8);

rlc_h:
plog("rlc_h\n");
	imm_8 = ((regs->reg_h & BIT_7) == BIT_7) ? BIT_0 : 0;
	regs->reg_f = imm_8 << 4;
	if (regs->reg_h == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_h <<= 1;
	regs->reg_h |= imm_8;
	return (8);

rlc_l:
plog("rlc_l\n");
	imm_8 = ((regs->reg_l & BIT_7) == BIT_7) ? BIT_0 : 0;
	regs->reg_f = imm_8 << 4;
	if (regs->reg_l == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_l <<= 1;
	regs->reg_l |= imm_8;
	return (8);

rlc_hl:
plog("rlc_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	imm_8 = ((*address & BIT_7) == BIT_7) ? BIT_0 : 0;
	regs->reg_f = imm_8 << 4;
	if (*address == 0)
		regs->reg_f |= FLAG_Z;
	*address <<= 1;
	*address |= imm_8;
	return (16);

rlc_a:
plog("rlc_a\n");
	imm_8 = ((regs->reg_a & BIT_7) == BIT_7) ? BIT_0 : 0;
	regs->reg_f = imm_8 << 4;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a <<= 1;
	regs->reg_a |= imm_8;
	return (8);

rrc_b:
plog("rrc_b\n");
	imm_8 = ((regs->reg_b & BIT_0) == BIT_0) ? BIT_7 : 0;
	regs->reg_f = imm_8 >> 3;
	if (regs->reg_b == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_b >>= 1;
	regs->reg_b |= imm_8;
	return (8);

rrc_c:
plog("rrc_c\n");
	imm_8 = ((regs->reg_c & BIT_0) == BIT_0) ? BIT_7 : 0;
	regs->reg_f = imm_8 >> 3;
	if (regs->reg_c == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_c >>= 1;
	regs->reg_c |= imm_8;
	return (8);

rrc_d:
plog("rrc_d\n");
	imm_8 = ((regs->reg_d & BIT_0) == BIT_0) ? BIT_7 : 0;
	regs->reg_f = imm_8 >> 3;
	if (regs->reg_d == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_d >>= 1;
	regs->reg_d |= imm_8;
	return (8);

rrc_e:
plog("rrc_e\n");
	imm_8 = ((regs->reg_e & BIT_0) == BIT_0) ? BIT_7 : 0;
	regs->reg_f = imm_8 >> 3;
	if (regs->reg_e == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_e >>= 1;
	regs->reg_e |= imm_8;
	return (8);

rrc_h:
plog("rrc_h\n");
	imm_8 = ((regs->reg_h & BIT_0) == BIT_0) ? BIT_7 : 0;
	regs->reg_f = imm_8 >> 3;
	if (regs->reg_h == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_h >>= 1;
	regs->reg_h |= imm_8;
	return (8);

rrc_l:
plog("rrc_l\n");
	imm_8 = ((regs->reg_l & BIT_0) == BIT_0) ? BIT_7 : 0;
	regs->reg_f = imm_8 >> 3;
	if (regs->reg_l == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_l >>= 1;
	regs->reg_l |= imm_8;
	return (8);

rrc_hl:
plog("rrc_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	imm_8 = ((*address & BIT_0) == BIT_0) ? BIT_7 : 0;
	regs->reg_f = imm_8 >> 3;
	if (*address == 0)
		regs->reg_f |= FLAG_Z;
	*address >>= 1;
	*address |= imm_8;
	return (16);

rrc_a:
plog("rrc_a\n");
	imm_8 = ((regs->reg_a & BIT_0) == BIT_0) ? BIT_7 : 0;
	regs->reg_f = imm_8 >> 3;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a >>= 1;
	regs->reg_a |= imm_8;
	return (8);

rl_b:
plog("rl_b\n");
	imm_8 = ((regs->reg_f & FLAG_CY) == FLAG_CY) ? BIT_0 : 0;
	regs->reg_f = ((regs->reg_b & BIT_7) == BIT_7) ? FLAG_CY : 0;
	if (regs->reg_b == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_b <<= 1;
	regs->reg_b |= imm_8;
	return (8);

rl_c:
plog("rl_c\n");
	imm_8 = ((regs->reg_f & FLAG_CY) == FLAG_CY) ? BIT_0 : 0;
	regs->reg_f = ((regs->reg_c & BIT_7) == BIT_7) ? FLAG_CY : 0;
	if (regs->reg_c == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_c <<= 1;
	regs->reg_c |= imm_8;
	return (8);

rl_d:
plog("rl_d\n");
	imm_8 = ((regs->reg_f & FLAG_CY) == FLAG_CY) ? BIT_0 : 0;
	regs->reg_f = ((regs->reg_d & BIT_7) == BIT_7) ? FLAG_CY : 0;
	if (regs->reg_d == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_d <<= 1;
	regs->reg_d |= imm_8;
	return (8);

rl_e:
plog("rl_e\n");
	imm_8 = ((regs->reg_f & FLAG_CY) == FLAG_CY) ? BIT_0 : 0;
	regs->reg_f = ((regs->reg_e & BIT_7) == BIT_7) ? FLAG_CY : 0;
	if (regs->reg_e == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_e <<= 1;
	regs->reg_e |= imm_8;
	return (8);

rl_h:
plog("rl_h\n");
	imm_8 = ((regs->reg_f & FLAG_CY) == FLAG_CY) ? BIT_0 : 0;
	regs->reg_f = ((regs->reg_h & BIT_7) == BIT_7) ? FLAG_CY : 0;
	if (regs->reg_h == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_h <<= 1;
	regs->reg_h |= imm_8;
	return (8);

rl_l:
plog("rl_l\n");
	imm_8 = ((regs->reg_f & FLAG_CY) == FLAG_CY) ? BIT_0 : 0;
	regs->reg_f = ((regs->reg_l & BIT_7) == BIT_7) ? FLAG_CY : 0;
	if (regs->reg_l == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_l <<= 1;
	regs->reg_l |= imm_8;
	return (8);

rl_hl:
plog("rl_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	imm_8 = ((regs->reg_f & FLAG_CY) == FLAG_CY) ? BIT_0 : 0;
	regs->reg_f = ((*address & BIT_7) == BIT_7) ? FLAG_CY : 0;
	if (*address == 0)
		regs->reg_f |= FLAG_Z;
	*address <<= 1;
	*address |= imm_8;
	return (16);

rl_a:
plog("rl_a\n");
	imm_8 = ((regs->reg_f & FLAG_CY) == FLAG_CY) ? BIT_0 : 0;
	regs->reg_f = ((regs->reg_a & BIT_7) == BIT_7) ? FLAG_CY : 0;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a <<= 1;
	regs->reg_a |= imm_8;
	return (8);

rr_b:
plog("rr_b\n");
	imm_8 = ((regs->reg_f & FLAG_CY) == FLAG_CY) ? BIT_7 : 0;
	regs->reg_f = ((regs->reg_b & BIT_0) == BIT_0) ? FLAG_CY : 0;
	if (regs->reg_b == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_b >>= 1;
	regs->reg_b |= imm_8;
	return (8);

rr_c:
plog("rr_c\n");
	imm_8 = ((regs->reg_f & FLAG_CY) == FLAG_CY) ? BIT_7 : 0;
	regs->reg_f = ((regs->reg_c & BIT_0) == BIT_0) ? FLAG_CY : 0;
	if (regs->reg_c == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_c >>= 1;
	regs->reg_c |= imm_8;
	return (8);

rr_d:
plog("rr_d\n");
	imm_8 = ((regs->reg_f & FLAG_CY) == FLAG_CY) ? BIT_7 : 0;
	regs->reg_f = ((regs->reg_d & BIT_0) == BIT_0) ? FLAG_CY : 0;
	if (regs->reg_d == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_d >>= 1;
	regs->reg_d |= imm_8;
	return (8);

rr_e:
plog("rr_e\n");
	imm_8 = ((regs->reg_f & FLAG_CY) == FLAG_CY) ? BIT_7 : 0;
	regs->reg_f = ((regs->reg_e & BIT_0) == BIT_0) ? FLAG_CY : 0;
	if (regs->reg_e == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_e >>= 1;
	regs->reg_e |= imm_8;
	return (8);

rr_h:
plog("rr_h\n");
	imm_8 = ((regs->reg_f & FLAG_CY) == FLAG_CY) ? BIT_7 : 0;
	regs->reg_f = ((regs->reg_h & BIT_0) == BIT_0) ? FLAG_CY : 0;
	if (regs->reg_h == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_h >>= 1;
	regs->reg_h |= imm_8;
	return (8);

rr_l:
plog("rr_l\n");
	imm_8 = ((regs->reg_f & FLAG_CY) == FLAG_CY) ? BIT_7 : 0;
	regs->reg_f = ((regs->reg_l & BIT_0) == BIT_0) ? FLAG_CY : 0;
	if (regs->reg_l == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_l >>= 1;
	regs->reg_l |= imm_8;
	return (8);

rr_hl:
plog("rr_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	imm_8 = ((regs->reg_f & FLAG_CY) == FLAG_CY) ? BIT_7 : 0;
	regs->reg_f = ((*address & BIT_0) == BIT_0) ? FLAG_CY : 0;
	if (*address == 0)
		regs->reg_f |= FLAG_Z;
	*address >>= 1;
	*address |= imm_8;
	return (16);

rr_a:
plog("rr_a\n");
	imm_8 = ((regs->reg_f & FLAG_CY) == FLAG_CY) ? BIT_7 : 0;
	regs->reg_f = ((regs->reg_a & BIT_0) == BIT_0) ? FLAG_CY : 0;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a >>= 1;
	regs->reg_a |= imm_8;
	return (8);

sla_b:
plog("sla_b\n");
	regs->reg_f = ((regs->reg_b & BIT_7) == BIT_7) ? FLAG_CY : 0;
	if (regs->reg_b == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_b <<= 1;
	return (8);

sla_c:
plog("sla_c\n");
	regs->reg_f = ((regs->reg_c & BIT_7) == BIT_7) ? FLAG_CY : 0;
	if (regs->reg_c == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_c <<= 1;
	return (8);

sla_d:
plog("sla_d\n");
	regs->reg_f = ((regs->reg_d & BIT_7) == BIT_7) ? FLAG_CY : 0;
	if (regs->reg_d == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_d <<= 1;
	return (8);

sla_e:
plog("sla_e\n");
	regs->reg_f = ((regs->reg_e & BIT_7) == BIT_7) ? FLAG_CY : 0;
	if (regs->reg_e == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_e <<= 1;
	return (8);

sla_h:
plog("sla_h\n");
	regs->reg_f = ((regs->reg_h & BIT_7) == BIT_7) ? FLAG_CY : 0;
	if (regs->reg_h == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_h <<= 1;
	return (8);

sla_l:
plog("sla_l\n");
	regs->reg_f = ((regs->reg_l & BIT_7) == BIT_7) ? FLAG_CY : 0;
	if (regs->reg_l == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_l <<= 1;
	return (8);

sla_hl:
plog("sla_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_f = ((*address & BIT_7) == BIT_7) ? FLAG_CY : 0;
	if (*address == 0)
		regs->reg_f |= FLAG_Z;
	*address <<= 1;
	return (16);

sla_a:
plog("sla_a\n");
	regs->reg_f = ((regs->reg_a & BIT_7) == BIT_7) ? FLAG_CY : 0;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a <<= 1;
	return (8);

sra_b:
plog("sra_b\n");
	regs->reg_f = ((regs->reg_b & BIT_0) == BIT_0) ? FLAG_CY : 0;
	imm_8 = regs->reg_b & BIT_7;
	regs->reg_b >>= 1;
	regs->reg_b |= imm_8;
	return (8);

sra_c:
plog("sra_c\n");
	regs->reg_f = ((regs->reg_c & BIT_0) == BIT_0) ? FLAG_CY : 0;
	imm_8 = regs->reg_c & BIT_7;
	regs->reg_c >>= 1;
	regs->reg_c |= imm_8;
	return (8);

sra_d:
plog("sra_d\n");
	regs->reg_f = ((regs->reg_d & BIT_0) == BIT_0) ? FLAG_CY : 0;
	imm_8 = regs->reg_d & BIT_7;
	regs->reg_d >>= 1;
	regs->reg_d |= imm_8;
	return (8);

sra_e:
plog("sra_e\n");
	regs->reg_f = ((regs->reg_e & BIT_0) == BIT_0) ? FLAG_CY : 0;
	imm_8 = regs->reg_e & BIT_7;
	regs->reg_e >>= 1;
	regs->reg_e |= imm_8;
	return (8);

sra_h:
plog("sra_h\n");
	regs->reg_f = ((regs->reg_h & BIT_0) == BIT_0) ? FLAG_CY : 0;
	imm_8 = regs->reg_h & BIT_7;
	regs->reg_h >>= 1;
	regs->reg_h |= imm_8;
	return (8);

sra_l:
plog("sra_l\n");
	regs->reg_f = ((regs->reg_l & BIT_0) == BIT_0) ? FLAG_CY : 0;
	imm_8 = regs->reg_l & BIT_7;
	regs->reg_l >>= 1;
	regs->reg_l |= imm_8;
	return (8);

sra_hl:
plog("sra_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_f = ((*address & BIT_0) == BIT_0) ? FLAG_CY : 0;
	imm_8 = *address & BIT_7;
	*address >>= 1;
	*address |= imm_8;
	return (16);

sra_a:
plog("sra_a\n");
	regs->reg_f = ((regs->reg_a & BIT_0) == BIT_0) ? FLAG_CY : 0;
	imm_8 = regs->reg_a & BIT_7;
	regs->reg_a >>= 1;
	regs->reg_a |= imm_8;
	return (8);

swap_b:
plog("swap_b\n");
	regs->reg_b = (regs->reg_b >> 4) | (regs->reg_b << 4);
	regs->reg_f = (regs->reg_b == 0) ? FLAG_Z : 0;
	return (8);

swap_c:
plog("swap_c\n");
	regs->reg_c = (regs->reg_c >> 4) | (regs->reg_c << 4);
	regs->reg_f = (regs->reg_c == 0) ? FLAG_Z : 0;
	return (8);

swap_d:
plog("swap_d\n");
	regs->reg_d = (regs->reg_d >> 4) | (regs->reg_d << 4);
	regs->reg_f = (regs->reg_d == 0) ? FLAG_Z : 0;
	return (8);

swap_e:
plog("swap_e\n");
	regs->reg_e = (regs->reg_e >> 4) | (regs->reg_e << 4);
	regs->reg_f = (regs->reg_e == 0) ? FLAG_Z : 0;
	return (8);

swap_h:
plog("swap_h\n");
	regs->reg_h = (regs->reg_h >> 4) | (regs->reg_h << 4);
	regs->reg_f = (regs->reg_h == 0) ? FLAG_Z : 0;
	return (8);

swap_l:
plog("swap_l\n");
	regs->reg_l = (regs->reg_l >> 4) | (regs->reg_l << 4);
	regs->reg_f = (regs->reg_l == 0) ? FLAG_Z : 0;
	return (8);

swap_hl:
plog("swap_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	*address = (*address >> 4) | (*address << 4);
	regs->reg_f = (*address == 0) ? FLAG_Z : 0;
	return (16);

swap_a:
plog("swap_a\n");
	regs->reg_a = (regs->reg_a >> 4) | (regs->reg_a << 4);
	regs->reg_f = (regs->reg_a == 0) ? FLAG_Z : 0;
	return (8);

srl_b:
plog("srl_b\n");
	regs->reg_f = ((regs->reg_b & BIT_0) == BIT_0) ? FLAG_CY : 0;
	if (regs->reg_b == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_b >>= 1;
	return (8);

srl_c:
plog("srl_c\n");
	regs->reg_f = ((regs->reg_c & BIT_0) == BIT_0) ? FLAG_CY : 0;
	if (regs->reg_c == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_c >>= 1;
	return (8);

srl_d:
plog("srl_d\n");
	regs->reg_f = ((regs->reg_d & BIT_0) == BIT_0) ? FLAG_CY : 0;
	if (regs->reg_d == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_d >>= 1;
	return (8);

srl_e:
plog("srl_e\n");
	regs->reg_f = ((regs->reg_e & BIT_0) == BIT_0) ? FLAG_CY : 0;
	if (regs->reg_e == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_e >>= 1;
	return (8);

srl_h:
plog("srl_h\n");
	regs->reg_f = ((regs->reg_h & BIT_0) == BIT_0) ? FLAG_CY : 0;
	if (regs->reg_h == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_h >>= 1;
	return (8);

srl_l:
plog("srl_l\n");
	regs->reg_f = ((regs->reg_l & BIT_0) == BIT_0) ? FLAG_CY : 0;
	if (regs->reg_l == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_l >>= 1;
	return (8);

srl_hl:
plog("srl_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_f = ((*address & BIT_0) == BIT_0) ? FLAG_CY : 0;
	if (*address == 0)
		regs->reg_f |= FLAG_Z;
	*address >>= 1;
	return (16);

srl_a:
plog("srl_a\n");
	regs->reg_f = ((regs->reg_a & BIT_0) == BIT_0) ? FLAG_CY : 0;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a >>= 1;
	return (8);

bit_0_b:
plog("bit_0_b\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_b & BIT_0) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_0_c:
plog("bit_0_c\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_c & BIT_0) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_0_d:
plog("bit_0_d\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_d & BIT_0) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_0_e:
plog("bit_0_e\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_e & BIT_0) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_0_h:
plog("bit_0_h\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_h & BIT_0) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_0_l:
plog("bit_0_l\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_l & BIT_0) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_0_hl:
plog("bit_0_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((*address & BIT_0) == 0)
		regs->reg_f |= FLAG_Z;
	return (16);

bit_0_a:
plog("bit_0_a\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_a & BIT_0) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_1_b:
plog("bit_1_b\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_b & BIT_1) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_1_c:
plog("bit_1_c\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_c & BIT_1) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_1_d:
plog("bit_1_d\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_d & BIT_1) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_1_e:
plog("bit_1_e\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_e & BIT_1) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_1_h:
plog("bit_1_h\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_h & BIT_1) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_1_l:
plog("bit_1_l\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_l & BIT_1) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_1_hl:
plog("bit_1_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((*address & BIT_1) == 0)
		regs->reg_f |= FLAG_Z;
	return (16);

bit_1_a:
plog("bit_1_a\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_a & BIT_1) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_2_b:
plog("bit_2_b\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_b & BIT_2) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_2_c:
plog("bit_2_c\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_c & BIT_2) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_2_d:
plog("bit_2_d\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_d & BIT_2) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_2_e:
plog("bit_2_e\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_e & BIT_2) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_2_h:
plog("bit_2_h\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_h & BIT_2) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_2_l:
plog("bit_2_l\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_l & BIT_2) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_2_hl:
plog("bit_2_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((*address & BIT_2) == 0)
		regs->reg_f |= FLAG_Z;
	return (16);

bit_2_a:
plog("bit_2_a\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_a & BIT_2) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_3_b:
plog("bit_3_b\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_b & BIT_3) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_3_c:
plog("bit_3_c\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_c & BIT_3) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_3_d:
plog("bit_3_d\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_d & BIT_3) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_3_e:
plog("bit_3_e\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_e & BIT_3) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_3_h:
plog("bit_3_h\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_h & BIT_3) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_3_l:
plog("bit_3_l\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_l & BIT_3) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_3_hl:
plog("bit_3_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((*address & BIT_3) == 0)
		regs->reg_f |= FLAG_Z;
	return (16);

bit_3_a:
plog("bit_3_a\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_a & BIT_3) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_4_b:
plog("bit_4_b\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_b & BIT_4) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_4_c:
plog("bit_4_c\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_c & BIT_4) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_4_d:
plog("bit_4_d\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_d & BIT_4) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_4_e:
plog("bit_4_e\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_e & BIT_4) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_4_h:
plog("bit_4_h\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_h & BIT_4) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_4_l:
plog("bit_4_l\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_l & BIT_4) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_4_hl:
plog("bit_4_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((*address & BIT_4) == 0)
		regs->reg_f |= FLAG_Z;
	return (16);

bit_4_a:
plog("bit_4_a\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_a & BIT_4) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_5_b:
plog("bit_5_b\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_b & BIT_5) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_5_c:
plog("bit_5_c\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_c & BIT_5) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_5_d:
plog("bit_5_d\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_d & BIT_5) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_5_e:
plog("bit_5_e\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_e & BIT_5) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_5_h:
plog("bit_5_h\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_h & BIT_5) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_5_l:
plog("bit_5_l\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_l & BIT_5) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_5_hl:
plog("bit_5_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((*address & BIT_5) == 0)
		regs->reg_f |= FLAG_Z;
	return (16);

bit_5_a:
plog("bit_5_a\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_a & BIT_5) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_6_b:
plog("bit_6_b\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_b & BIT_6) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_6_c:
plog("bit_6_c\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_c & BIT_6) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_6_d:
plog("bit_6_d\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_d & BIT_6) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_6_e:
plog("bit_6_e\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_e & BIT_6) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_6_h:
plog("bit_6_h\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_h & BIT_6) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_6_l:
plog("bit_6_l\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_l & BIT_6) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_6_hl:
plog("bit_6_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((*address & BIT_6) == 0)
		regs->reg_f |= FLAG_Z;
	return (16);

bit_6_a:
plog("bit_6_a\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_a & BIT_6) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_7_b:
plog("bit_7_b\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_b & BIT_7) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_7_c:
plog("bit_7_c\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_c & BIT_7) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_7_d:
plog("bit_7_d\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_d & BIT_7) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_7_e:
plog("bit_7_e\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_e & BIT_7) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_7_h:
plog("bit_7_h\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_h & BIT_7) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_7_l:
plog("bit_7_l\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_l & BIT_7) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

bit_7_hl:
plog("bit_7_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((*address & BIT_7) == 0)
		regs->reg_f |= FLAG_Z;
	return (16);

bit_7_a:
plog("bit_7_a\n");
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_H;
	if ((regs->reg_a & BIT_7) == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

res_0_b:
plog("res_0_b\n");
	regs->reg_b &= ~(BIT_0);
	return (8);

res_0_c:
plog("res_0_c\n");
	regs->reg_c &= ~(BIT_0);
	return (8);

res_0_d:
plog("res_0_d\n");
	regs->reg_d &= ~(BIT_0);
	return (8);

res_0_e:
plog("res_0_e\n");
	regs->reg_e &= ~(BIT_0);
	return (8);

res_0_h:
plog("res_0_h\n");
	regs->reg_h &= ~(BIT_0);
	return (8);

res_0_l:
plog("res_0_l\n");
	regs->reg_l &= ~(BIT_0);
	return (8);

res_0_hl:
plog("res_0_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	*address &= ~(BIT_0);
	return (16);

res_0_a:
plog("res_0_a\n");
	regs->reg_a &= ~(BIT_0);
	return (8);

res_1_b:
plog("res_1_b\n");
	regs->reg_b &= ~(BIT_1);
	return (8);

res_1_c:
plog("res_1_c\n");
	regs->reg_c &= ~(BIT_1);
	return (8);

res_1_d:
plog("res_1_d\n");
	regs->reg_d &= ~(BIT_1);
	return (8);

res_1_e:
plog("res_1_e\n");
	regs->reg_e &= ~(BIT_1);
	return (8);

res_1_h:
plog("res_1_h\n");
	regs->reg_h &= ~(BIT_1);
	return (8);

res_1_l:
plog("res_1_l\n");
	regs->reg_l &= ~(BIT_1);
	return (8);

res_1_hl:
plog("res_1_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	*address &= ~(BIT_1);
	return (16);

res_1_a:
plog("res_1_a\n");
	regs->reg_a &= ~(BIT_1);
	return (8);

res_2_b:
plog("res_2_b\n");
	regs->reg_b &= ~(BIT_2);
	return (8);

res_2_c:
plog("res_2_c\n");
	regs->reg_c &= ~(BIT_2);
	return (8);

res_2_d:
plog("res_2_d\n");
	regs->reg_d &= ~(BIT_2);
	return (8);

res_2_e:
plog("res_2_e\n");
	regs->reg_e &= ~(BIT_2);
	return (8);

res_2_h:
plog("res_2_h\n");
	regs->reg_h &= ~(BIT_2);
	return (8);

res_2_l:
plog("res_2_l\n");
	regs->reg_l &= ~(BIT_2);
	return (8);

res_2_hl:
plog("res_2_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	*address &= ~(BIT_2);
	return (16);

res_2_a:
plog("res_2_a\n");
	regs->reg_a &= ~(BIT_2);
	return (8);

res_3_b:
plog("res_3_b\n");
	regs->reg_b &= ~(BIT_3);
	return (8);

res_3_c:
plog("res_3_c\n");
	regs->reg_c &= ~(BIT_3);
	return (8);

res_3_d:
plog("res_3_d\n");
	regs->reg_d &= ~(BIT_3);
	return (8);

res_3_e:
plog("res_3_e\n");
	regs->reg_e &= ~(BIT_3);
	return (8);

res_3_h:
plog("res_3_h\n");
	regs->reg_h &= ~(BIT_3);
	return (8);

res_3_l:
plog("res_3_l\n");
	regs->reg_l &= ~(BIT_3);
	return (8);

res_3_hl:
plog("res_3_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	*address &= ~(BIT_3);
	return (16);

res_3_a:
plog("res_3_a\n");
	regs->reg_a &= ~(BIT_3);
	return (8);

res_4_b:
plog("res_4_b\n");
	regs->reg_b &= ~(BIT_4);
	return (8);

res_4_c:
plog("res_4_c\n");
	regs->reg_c &= ~(BIT_4);
	return (8);

res_4_d:
plog("res_4_d\n");
	regs->reg_d &= ~(BIT_4);
	return (8);

res_4_e:
plog("res_4_e\n");
	regs->reg_e &= ~(BIT_4);
	return (8);

res_4_h:
plog("res_4_h\n");
	regs->reg_h &= ~(BIT_4);
	return (8);

res_4_l:
plog("res_4_l\n");
	regs->reg_l &= ~(BIT_4);
	return (8);

res_4_hl:
plog("res_4_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	*address &= ~(BIT_4);
	return (16);

res_4_a:
plog("res_4_a\n");
	regs->reg_a &= ~(BIT_4);
	return (8);

res_5_b:
plog("res_5_b\n");
	regs->reg_b &= ~(BIT_5);
	return (8);

res_5_c:
plog("res_5_c\n");
	regs->reg_c &= ~(BIT_5);
	return (8);

res_5_d:
plog("res_5_d\n");
	regs->reg_d &= ~(BIT_5);
	return (8);

res_5_e:
plog("res_5_e\n");
	regs->reg_e &= ~(BIT_5);
	return (8);

res_5_h:
plog("res_5_h\n");
	regs->reg_h &= ~(BIT_5);
	return (8);

res_5_l:
plog("res_5_l\n");
	regs->reg_l &= ~(BIT_5);
	return (8);

res_5_hl:
plog("res_5_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	*address &= ~(BIT_5);
	return (16);

res_5_a:
plog("res_5_a\n");
	regs->reg_a &= ~(BIT_5);
	return (8);

res_6_b:
plog("res_6_b\n");
	regs->reg_b &= ~(BIT_6);
	return (8);

res_6_c:
plog("res_6_c\n");
	regs->reg_c &= ~(BIT_6);
	return (8);

res_6_d:
plog("res_6_d\n");
	regs->reg_d &= ~(BIT_6);
	return (8);

res_6_e:
plog("res_6_e\n");
	regs->reg_e &= ~(BIT_6);
	return (8);

res_6_h:
plog("res_6_h\n");
	regs->reg_h &= ~(BIT_6);
	return (8);

res_6_l:
plog("res_6_l\n");
	regs->reg_l &= ~(BIT_6);
	return (8);

res_6_hl:
plog("res_6_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	*address &= ~(BIT_6);
	return (16);

res_6_a:
plog("res_6_a\n");
	regs->reg_a &= ~(BIT_6);
	return (8);

res_7_b:
plog("res_7_b\n");
	regs->reg_b &= ~(BIT_7);
	return (8);

res_7_c:
plog("res_7_c\n");
	regs->reg_c &= ~(BIT_7);
	return (8);

res_7_d:
plog("res_7_d\n");
	regs->reg_d &= ~(BIT_7);
	return (8);

res_7_e:
plog("res_7_e\n");
	regs->reg_e &= ~(BIT_7);
	return (8);

res_7_h:
plog("res_7_h\n");
	regs->reg_h &= ~(BIT_7);
	return (8);

res_7_l:
plog("res_7_l\n");
	regs->reg_l &= ~(BIT_7);
	return (8);

res_7_hl:
plog("res_7_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	*address &= ~(BIT_7);
	return (16);

res_7_a:
plog("res_7_a\n");
	regs->reg_a &= ~(BIT_7);
	return (8);

set_0_b:
plog("set_0_b\n");
	regs->reg_b |= (BIT_0);
	return (8);

set_0_c:
plog("set_0_c\n");
	regs->reg_c |= (BIT_0);
	return (8);

set_0_d:
plog("set_0_d\n");
	regs->reg_d |= (BIT_0);
	return (8);

set_0_e:
plog("set_0_e\n");
	regs->reg_e |= (BIT_0);
	return (8);

set_0_h:
plog("set_0_h\n");
	regs->reg_h |= (BIT_0);
	return (8);

set_0_l:
plog("set_0_l\n");
	regs->reg_l |= (BIT_0);
	return (8);

set_0_hl:
plog("set_0_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	*address |= (BIT_0);
	return (16);

set_0_a:
plog("set_0_a\n");
	regs->reg_a |= (BIT_0);
	return (8);

set_1_b:
plog("set_1_b\n");
	regs->reg_b |= (BIT_1);
	return (8);

set_1_c:
plog("set_1_c\n");
	regs->reg_c |= (BIT_1);
	return (8);

set_1_d:
plog("set_1_d\n");
	regs->reg_d |= (BIT_1);
	return (8);

set_1_e:
plog("set_1_e\n");
	regs->reg_e |= (BIT_1);
	return (8);

set_1_h:
plog("set_1_h\n");
	regs->reg_h |= (BIT_1);
	return (8);

set_1_l:
plog("set_1_l\n");
	regs->reg_l |= (BIT_1);
	return (8);

set_1_hl:
plog("set_1_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	*address |= (BIT_1);
	return (16);

set_1_a:
plog("set_1_a\n");
	regs->reg_a |= (BIT_1);
	return (8);

set_2_b:
plog("set_2_b\n");
	regs->reg_b |= (BIT_2);
	return (8);

set_2_c:
plog("set_2_c\n");
	regs->reg_c |= (BIT_2);
	return (8);

set_2_d:
plog("set_2_d\n");
	regs->reg_d |= (BIT_2);
	return (8);

set_2_e:
plog("set_2_e\n");
	regs->reg_e |= (BIT_2);
	return (8);

set_2_h:
plog("set_2_h\n");
	regs->reg_h |= (BIT_2);
	return (8);

set_2_l:
plog("set_2_l\n");
	regs->reg_l |= (BIT_2);
	return (8);

set_2_hl:
plog("set_2_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	*address |= (BIT_2);
	return (16);

set_2_a:
plog("set_2_a\n");
	regs->reg_a |= (BIT_2);
	return (8);

set_3_b:
plog("set_3_b\n");
	regs->reg_b |= (BIT_3);
	return (8);

set_3_c:
plog("set_3_c\n");
	regs->reg_c |= (BIT_3);
	return (8);

set_3_d:
plog("set_3_d\n");
	regs->reg_d |= (BIT_3);
	return (8);

set_3_e:
plog("set_3_e\n");
	regs->reg_e |= (BIT_3);
	return (8);

set_3_h:
plog("set_3_h\n");
	regs->reg_h |= (BIT_3);
	return (8);

set_3_l:
plog("set_3_l\n");
	regs->reg_l |= (BIT_3);
	return (8);

set_3_hl:
plog("set_3_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	*address |= (BIT_3);
	return (16);

set_3_a:
plog("set_3_a\n");
	regs->reg_a |= (BIT_3);
	return (8);

set_4_b:
plog("set_4_b\n");
	regs->reg_b |= (BIT_4);
	return (8);

set_4_c:
plog("set_4_c\n");
	regs->reg_c |= (BIT_4);
	return (8);

set_4_d:
plog("set_4_d\n");
	regs->reg_d |= (BIT_4);
	return (8);

set_4_e:
plog("set_4_e\n");
	regs->reg_e |= (BIT_4);
	return (8);

set_4_h:
plog("set_4_h\n");
	regs->reg_h |= (BIT_4);
	return (8);

set_4_l:
plog("set_4_l\n");
	regs->reg_l |= (BIT_4);
	return (8);

set_4_hl:
plog("set_4_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	*address |= (BIT_4);
	return (16);

set_4_a:
plog("set_4_a\n");
	regs->reg_a |= (BIT_4);
	return (8);

set_5_b:
plog("set_5_b\n");
	regs->reg_b |= (BIT_5);
	return (8);

set_5_c:
plog("set_5_c\n");
	regs->reg_c |= (BIT_5);
	return (8);

set_5_d:
plog("set_5_d\n");
	regs->reg_d |= (BIT_5);
	return (8);

set_5_e:
plog("set_5_e\n");
	regs->reg_e |= (BIT_5);
	return (8);

set_5_h:
plog("set_5_h\n");
	regs->reg_h |= (BIT_5);
	return (8);

set_5_l:
plog("set_5_l\n");
	regs->reg_l |= (BIT_5);
	return (8);

set_5_hl:
plog("set_5_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	*address |= (BIT_5);
	return (16);

set_5_a:
plog("set_5_a\n");
	regs->reg_a |= (BIT_5);
	return (8);

set_6_b:
plog("set_6_b\n");
	regs->reg_b |= (BIT_6);
	return (8);

set_6_c:
plog("set_6_c\n");
	regs->reg_c |= (BIT_6);
	return (8);

set_6_d:
plog("set_6_d\n");
	regs->reg_d |= (BIT_6);
	return (8);

set_6_e:
plog("set_6_e\n");
	regs->reg_e |= (BIT_6);
	return (8);

set_6_h:
plog("set_6_h\n");
	regs->reg_h |= (BIT_6);
	return (8);

set_6_l:
plog("set_6_l\n");
	regs->reg_l |= (BIT_6);
	return (8);

set_6_hl:
plog("set_6_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	*address |= (BIT_6);
	return (16);

set_6_a:
plog("set_6_a\n");
	regs->reg_a |= (BIT_6);
	return (8);

set_7_b:
plog("set_7_b\n");
	regs->reg_b |= (BIT_7);
	return (8);

set_7_c:
plog("set_7_c\n");
	regs->reg_c |= (BIT_7);
	return (8);

set_7_d:
plog("set_7_d\n");
	regs->reg_d |= (BIT_7);
	return (8);

set_7_e:
plog("set_7_e\n");
	regs->reg_e |= (BIT_7);
	return (8);

set_7_h:
plog("set_7_h\n");
	regs->reg_h |= (BIT_7);
	return (8);

set_7_l:
plog("set_7_l\n");
	regs->reg_l |= (BIT_7);
	return (8);

set_7_hl:
plog("set_7_hl\n");
	address = GET_REAL_ADDR(regs->reg_hl);
	*address |= (BIT_7);
	return (16);

set_7_a:
plog("set_7_a\n");
	regs->reg_a |= (BIT_7);
	return (8);

/****************
**    MBC_1    **
****************/
mbc1_0:
	plog("\nmbc1_0:\n\n");
	ENABLE_EXTERNAL_RAM_MBC1();
	return (cycles);


mbc1_1:
	plog("\nmbc1_1:\n\n");
	/* 5 bits register */
	g_memmap.cart_reg[MBC1_ROM_NUM] = (value & 0x1f);
	SWITCH_RAM_ROM_MBC1();
	return (cycles);

mbc1_2:
	plog("\nmbc1_2:\n\n");
	/* 3 bits register */
	g_memmap.cart_reg[MBC1_RAM_NUM] = (value & 0x03);
	SWITCH_RAM_ROM_MBC1();
	return (cycles);

mbc1_3:
	plog("\nmbc1_3:\n\n");
	/* 1 bit register */
	g_memmap.cart_reg[MBC1_MODE] = (value & 0x01);
	SWITCH_RAM_ROM_MBC1();
	return (cycles);


/****************
**    MBC_2    **
****************/
mbc2_0:
mbc2_1:


/****************
**    MBC_3    **
****************/
mbc3_0:
mbc3_1:
mbc3_2:
mbc3_3:


/****************
**    MBC_5    **
****************/
mbc5_0:
	ENABLE_EXTERNAL_RAM_MBC5();
	return (cycles);

mbc5_1:
	SET_LOW_ROM_NUMBER_MBC5();
	return (cycles);

mbc5_2:
	SET_HI_ROM_NUMBER_MBC5();
	return (cycles);

mbc5_3:
	SET_RAM_NUMBER_MBC5();
	return (cycles);


/****************
**    ERROR    **
****************/
redzone_ret:
	*g_memmap.redzone = 0xff; /* read-only memory is written */
	return (0);
}
