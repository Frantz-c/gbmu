/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   execute.c                                        .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/29 09:59:24 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/05/30 11:05:38 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "execute.h"

extern uint8_t		*g_get_real_read_addr[16];
extern uint8_t		*g_get_real_write_addr[16];
extern memory_map_t	g_memmap;

#define ADD_PC(offset)	regs->reg_pc += (offset);
#define SET_PC(value)	regs->reg_pc = (value);

cycle_count_t	execute(registers_t *regs)
{
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

	uint8_t			*address;

	address = GET_REAL_READ_ADDR(regs->reg_pc);

	uint8_t				opcode = address[0];
	register uint8_t	imm_8 = address[1];
	register uint16_t	imm_16 = (uint16_t)address[1] | ((uint16_t)address[2] << 8);

	goto *instruction_jumps[opcode];

nop:
	ADD_PC(1);
	return (4);

ld_bc_imm16:
	ADD_PC(3);
	regs->reg_bc = imm_16;
	return (12);

ld_bc_a:
	ADD_PC(1);
	address = GET_REAL_WRITE_ADDR(regs->reg_bc);
	*address = regs->reg_a;
	return (8);

inc_bc:
	ADD_PC(1);
	regs->reg_bc += 1;
	return (8);

inc_b:
	ADD_PC(1);
	regs->reg_f &= FLAG_CY;
	regs->reg_b += 1;
	if (regs->reg_b == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_b & 0xFu) == 0)
		regs->reg_f |= FLAG_H;
	return (4);

dec_b:
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
	ADD_PC(2);
	regs->reg_b = imm_8;
	return (8);

rlca:
	ADD_PC(1);
	imm_8 = ((regs->reg_a & BIT_7) == BIT_7) ? BIT_0 : 0;
	regs->reg_f = imm_8 << 4;
	regs->reg_a <<= 1;
	regs->reg_a |= imm_8;
	return (4);

ld_addr16_sp:
	ADD_PC(3);
	address = GET_REAL_WRITE_ADDR(imm_16);
	address[0] = regs->reg_sl;
	address[1] = regs->reg_sh;
	return (20);

add_hl_bc:
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
	ADD_PC(1);
	address = GET_REAL_READ_ADDR(regs->reg_bc);
	regs->reg_a = *address;
	return (8);

dec_bc:
	ADD_PC(1);
	regs->reg_bc -= 1;
	return (8);

inc_c:
	ADD_PC(1);
	regs->reg_f &= FLAG_CY;
	regs->reg_c += 1;
	if (regs->reg_c == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_c & 0xFu) == 0)
		regs->reg_f |= FLAG_H;
	return (4);

dec_c:
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
	ADD_PC(2);
	regs->reg_c = imm_8;
	return (8);

rrca:
	ADD_PC(1);
	imm_8 = ((regs->reg_a & BIT_0) == BIT_0) ? BIT_7 : 0;
	regs->reg_f = imm_8 >> 3;
	regs->reg_a >>= 1;
	regs->reg_a |= imm_8;
	return (4);

stop:
	ADD_PC(1);

	// Va te faire ...

	return (4);

ld_de_imm16:
	ADD_PC(3);
	regs->reg_de = imm_16;
	return (12);

ld_de_a:
	ADD_PC(1);
	address = GET_REAL_WRITE_ADDR(regs->reg_de);
	*address = regs->reg_a;
	return (8);

inc_de:
	ADD_PC(1);
	regs->reg_de += 1;
	return (8);

inc_d:
	ADD_PC(1);
	regs->reg_f &= FLAG_CY;
	regs->reg_d += 1;
	if (regs->reg_d == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_d & 0xFu) == 0)
		regs->reg_f |= FLAG_H;
	return (4);

dec_d:
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
	ADD_PC(2);
	regs->reg_d = imm_8;
	return (8);

rla:
	ADD_PC(1);
	imm_8 = ((regs->reg_f & FLAG_CY) == FLAG_CY) ? BIT_0 : 0;
	regs->reg_f = ((regs->reg_a & BIT_7) == BIT_7) ? FLAG_CY : 0;
	regs->reg_a <<= 1;
	regs->reg_a |= imm_8;
	return (4);

jr_imm8:
	ADD_PC(imm_8);
	return (12);

add_hl_de:
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
	ADD_PC(1);
	address = GET_REAL_READ_ADDR(regs->reg_de);
	regs->reg_a = *address;
	return (8);

dec_de:
	ADD_PC(1);
	regs->reg_de -= 1;
	return (8);

inc_e:
	ADD_PC(1);
	regs->reg_f &= FLAG_CY;
	regs->reg_e += 1;
	if (regs->reg_e == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_e & 0xFu) == 0)
		regs->reg_f |= FLAG_H;
	return (4);

dec_e:
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
	ADD_PC(2);
	regs->reg_d = imm_8;
	return (8);

rra:
	ADD_PC(1);
	imm_8 = ((regs->reg_f & FLAG_CY) == FLAG_CY) ? BIT_7 : 0;
	regs->reg_f = ((regs->reg_a & BIT_0) == BIT_0) ? FLAG_CY : 0;
	regs->reg_a >>= 1;
	regs->reg_a |= imm_8;
	return (4);

jrnz_imm8:
	if ((regs->reg_f & FLAG_Z) == 0)
	{
		ADD_PC(imm_8);
		return (12);
	}
	else
	{
		ADD_PC(2);
		return (8);
	}

ld_hl_imm16:
	ADD_PC(3);
	regs->reg_hl = imm_16;
	return (12);

ld_hli_a:
	ADD_PC(1);
	address = GET_REAL_WRITE_ADDR(regs->reg_hl);
	regs->reg_hl += 1;
	*address = regs->reg_a;
	return (8);

inc_hl:
	ADD_PC(1);
	regs->reg_hl += 1;
	return (8);

inc_h:
	ADD_PC(1);
	regs->reg_f &= FLAG_CY;
	regs->reg_h += 1;
	if (regs->reg_h == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_h & 0xFu) == 0)
		regs->reg_f |= FLAG_H;
	return (4);

dec_h:
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
	ADD_PC(2);
	regs->reg_h = imm_8;
	return (8);

daa:
	ADD_PC(1);

	// Va te faire ...

	return (4);

jrz_imm8:
	if ((regs->reg_f & FLAG_Z) == FLAG_Z)
	{
		ADD_PC(imm_8);
		return (12);
	}
	else
	{
		ADD_PC(2);
		return (8);
	}

add_hl_hl:
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
	ADD_PC(1);
	address = GET_REAL_READ_ADDR(regs->reg_hl);
	regs->reg_hl += 1;
	regs->reg_a = *address;
	return (8);

dec_hl:
	ADD_PC(1);
	regs->reg_hl -= 1;
	return (8);

inc_l:
	ADD_PC(1);
	regs->reg_f &= FLAG_CY;
	regs->reg_l += 1;
	if (regs->reg_l == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_l & 0xFu) == 0)
		regs->reg_f |= FLAG_H;
	return (4);

dec_l:
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
	ADD_PC(2);
	regs->reg_l = imm_8;
	return (8);

cpl:
	ADD_PC(1);
	regs->reg_f |= FLAG_N | FLAG_H;
	regs->reg_a = ~(regs->reg_a);
	return (4);

jrnc_imm8:
	if ((regs->reg_f & FLAG_CY) == 0)
	{
		ADD_PC(imm_8);
		return (12);
	}
	else
	{
		ADD_PC(2);
		return (8);
	}

ld_sp_imm16:
	ADD_PC(3);
	regs->reg_sp = imm_16;
	return (12);

ld_hld_a:
	ADD_PC(1);
	address = GET_REAL_WRITE_ADDR(regs->reg_hl);
	regs->reg_hl -= 1;
	*address = regs->reg_a;
	return (8);

inc_sp:
	ADD_PC(1);
	regs->reg_sp += 1;
	return (8);

inc_ahl:
	ADD_PC(1);
	regs->reg_f &= FLAG_CY;
	address = GET_REAL_WRITE_ADDR(regs->reg_hl);
	*address += 1;
	if (*address == 0)
		regs->reg_f |= FLAG_Z;
	if ((*address & 0xFu) == 0)
		regs->reg_f |= FLAG_H;
	return (12);

dec_ahl:
	ADD_PC(1);
	regs->reg_f &= FLAG_CY;
	regs->reg_f |= FLAG_N;
	address = GET_REAL_WRITE_ADDR(regs->reg_hl);
	*address -= 1;
	if (*address == 0)
		regs->reg_f |= FLAG_Z;
	if ((*address & 0xFu) == 0xFu)
		regs->reg_f |= FLAG_H;
	return (12);

ld_hl_imm8:
	ADD_PC(2);
	address = GET_REAL_WRITE_ADDR(regs->reg_hl);
	*address = imm_8;
	return (12);

scf:
	ADD_PC(1);
	regs->reg_f &= FLAG_Z;
	regs->reg_f |= FLAG_CY;
	return (4);

jrc_imm8:
	if ((regs->reg_f & FLAG_CY) == FLAG_CY)
	{
		ADD_PC(imm_8);
		return (12);
	}
	else
	{
		ADD_PC(2);
		return (8);
	}

add_hl_sp:
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
	ADD_PC(1);
	address = GET_REAL_READ_ADDR(regs->reg_hl);
	regs->reg_hl -= 1;
	regs->reg_a = *address;
	return (8);

dec_sp:
	ADD_PC(1);
	regs->reg_sp -= 1;
	return (8);

inc_a:
	ADD_PC(1);
	regs->reg_f &= FLAG_CY;
	regs->reg_a += 1;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_a & 0xFu) == 0)
		regs->reg_f |= FLAG_H;
	return (4);

dec_a:
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
	ADD_PC(2);
	regs->reg_a = imm_8;
	return (8);

ccf:
	ADD_PC(1);
	regs->reg_f ^= FLAG_CY;
	regs->reg_f &= (FLAG_Z | FLAG_CY);
	return (4);

ld_b_b:
	ADD_PC(1);
	regs->reg_b = regs->reg_b;
	return (4);

ld_b_c:
	ADD_PC(1);
	regs->reg_b = regs->reg_c;
	return (4);

ld_b_d:
	ADD_PC(1);
	regs->reg_b = regs->reg_d;
	return (4);

ld_b_e:
	ADD_PC(1);
	regs->reg_b = regs->reg_e;
	return (4);

ld_b_h:
	ADD_PC(1);
	regs->reg_b = regs->reg_h;
	return (4);

ld_b_l:
	ADD_PC(1);
	regs->reg_b = regs->reg_l;
	return (4);

ld_b_hl:
	ADD_PC(1);
	address = GET_REAL_READ_ADDR(regs->reg_hl);
	regs->reg_b = *address;
	return (8);

ld_b_a:
	ADD_PC(1);
	regs->reg_b = regs->reg_a;
	return (4);

ld_c_b:
	ADD_PC(1);
	regs->reg_c = regs->reg_b;
	return (4);

ld_c_c:
	ADD_PC(1);
	regs->reg_c = regs->reg_c;
	return (4);

ld_c_d:
	ADD_PC(1);
	regs->reg_c = regs->reg_d;
	return (4);

ld_c_e:
	ADD_PC(1);
	regs->reg_c = regs->reg_e;
	return (4);

ld_c_h:
	ADD_PC(1);
	regs->reg_c = regs->reg_h;
	return (4);

ld_c_l:
	ADD_PC(1);
	regs->reg_c = regs->reg_l;
	return (4);

ld_c_hl:
	ADD_PC(1);
	address = GET_REAL_READ_ADDR(regs->reg_hl);
	regs->reg_c = *address;
	return (8);

ld_c_a:
	ADD_PC(1);
	regs->reg_c = regs->reg_a;
	return (4);

ld_d_b:
	ADD_PC(1);
	regs->reg_d = regs->reg_b;
	return (4);

ld_d_c:
	ADD_PC(1);
	regs->reg_d = regs->reg_c;
	return (4);

ld_d_d:
	ADD_PC(1);
	regs->reg_d = regs->reg_d;
	return (4);

ld_d_e:
	ADD_PC(1);
	regs->reg_d = regs->reg_e;
	return (4);

ld_d_h:
	ADD_PC(1);
	regs->reg_d = regs->reg_h;
	return (4);

ld_d_l:
	ADD_PC(1);
	regs->reg_d = regs->reg_l;
	return (4);

ld_d_hl:
	ADD_PC(1);
	address = GET_REAL_READ_ADDR(regs->reg_hl);
	regs->reg_d = *address;
	return (8);

ld_d_a:
	ADD_PC(1);
	regs->reg_d = regs->reg_a;
	return (4);

ld_e_b:
	ADD_PC(1);
	regs->reg_e = regs->reg_b;
	return (4);

ld_e_c:
	ADD_PC(1);
	regs->reg_e = regs->reg_c;
	return (4);

ld_e_d:
	ADD_PC(1);
	regs->reg_e = regs->reg_d;
	return (4);

ld_e_e:
	ADD_PC(1);
	regs->reg_e = regs->reg_e;
	return (4);

ld_e_h:
	ADD_PC(1);
	regs->reg_e = regs->reg_h;
	return (4);

ld_e_l:
	ADD_PC(1);
	regs->reg_e = regs->reg_l;
	return (4);

ld_e_hl:
	ADD_PC(1);
	address = GET_REAL_READ_ADDR(regs->reg_hl);
	regs->reg_e = *address;
	return (8);

ld_e_a:
	ADD_PC(1);
	regs->reg_e = regs->reg_a;
	return (4);

ld_h_b:
	ADD_PC(1);
	regs->reg_h = regs->reg_b;
	return (4);

ld_h_c:
	ADD_PC(1);
	regs->reg_h = regs->reg_c;
	return (4);

ld_h_d:
	ADD_PC(1);
	regs->reg_h = regs->reg_d;
	return (4);

ld_h_e:
	ADD_PC(1);
	regs->reg_h = regs->reg_e;
	return (4);

ld_h_h:
	ADD_PC(1);
	regs->reg_h = regs->reg_h;
	return (4);

ld_h_l:
	ADD_PC(1);
	regs->reg_h = regs->reg_l;
	return (4);

ld_h_hl:
	ADD_PC(1);
	address = GET_REAL_READ_ADDR(regs->reg_hl);
	regs->reg_h = *address;
	return (8);

ld_h_a:
	ADD_PC(1);
	regs->reg_h = regs->reg_a;
	return (4);

ld_l_b:
	ADD_PC(1);
	regs->reg_l = regs->reg_b;
	return (4);

ld_l_c:
	ADD_PC(1);
	regs->reg_l = regs->reg_c;
	return (4);

ld_l_d:
	ADD_PC(1);
	regs->reg_l = regs->reg_d;
	return (4);

ld_l_e:
	ADD_PC(1);
	regs->reg_l = regs->reg_e;
	return (4);

ld_l_h:
	ADD_PC(1);
	regs->reg_l = regs->reg_h;
	return (4);

ld_l_l:
	ADD_PC(1);
	regs->reg_l = regs->reg_l;
	return (4);

ld_l_hl:
	ADD_PC(1);
	address = GET_REAL_READ_ADDR(regs->reg_hl);
	regs->reg_l = *address;
	return (8);

ld_l_a:
	ADD_PC(1);
	regs->reg_l = regs->reg_a;
	return (4);

ld_hl_b:
	ADD_PC(1);
	address = GET_REAL_WRITE_ADDR(regs->reg_hl);
	*address = regs->reg_b;
	return (4);

ld_hl_c:
	ADD_PC(1);
	address = GET_REAL_WRITE_ADDR(regs->reg_hl);
	*address = regs->reg_c;
	return (4);

ld_hl_d:
	ADD_PC(1);
	address = GET_REAL_WRITE_ADDR(regs->reg_hl);
	*address = regs->reg_d;
	return (4);

ld_hl_e:
	ADD_PC(1);
	address = GET_REAL_WRITE_ADDR(regs->reg_hl);
	*address = regs->reg_e;
	return (4);

ld_hl_h:
	ADD_PC(1);
	address = GET_REAL_WRITE_ADDR(regs->reg_hl);
	*address = regs->reg_h;
	return (4);

ld_hl_l:
	ADD_PC(1);
	address = GET_REAL_WRITE_ADDR(regs->reg_hl);
	*address = regs->reg_l;
	return (4);

halt:
	ADD_PC(1);
	
	// Va te faire ...

	return (8);

ld_hl_a:
	ADD_PC(1);
	address = GET_REAL_WRITE_ADDR(regs->reg_hl);
	*address = regs->reg_a;
	return (4);

ld_a_b:
	ADD_PC(1);
	regs->reg_a = regs->reg_b;
	return (4);

ld_a_c:
	ADD_PC(1);
	regs->reg_a = regs->reg_c;
	return (4);

ld_a_d:
	ADD_PC(1);
	regs->reg_a = regs->reg_d;
	return (4);

ld_a_e:
	ADD_PC(1);
	regs->reg_a = regs->reg_e;
	return (4);

ld_a_h:
	ADD_PC(1);
	regs->reg_a = regs->reg_h;
	return (4);

ld_a_l:
	ADD_PC(1);
	regs->reg_a = regs->reg_l;
	return (4);

ld_a_hl:
	ADD_PC(1);
	address = GET_REAL_READ_ADDR(regs->reg_hl);
	regs->reg_a = *address;
	return (8);

ld_a_a:
	ADD_PC(1);
	regs->reg_a = regs->reg_a;
	return (4);

add_a_b:
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
	ADD_PC(1);
	regs->reg_f = 0;
	address = GET_REAL_READ_ADDR(regs->reg_hl);
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
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = 0;
	imm_8 = regs->reg_a + regs->reg_b;
	imm_8 += ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 < regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu) ||
		(imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY)
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

adc_a_c:
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = 0;
	imm_8 = regs->reg_a + regs->reg_c;
	imm_8 += ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 < regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu) ||
		(imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY)
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

adc_a_d:
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = 0;
	imm_8 = regs->reg_a + regs->reg_d;
	imm_8 += ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 < regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu) ||
		(imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY)
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

adc_a_e:
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = 0;
	imm_8 = regs->reg_a + regs->reg_e;
	imm_8 += ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 < regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu) ||
		(imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY)
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

adc_a_h:
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = 0;
	imm_8 = regs->reg_a + regs->reg_h;
	imm_8 += ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 < regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu) ||
		(imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY)
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

adc_a_l:
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = 0;
	imm_8 = regs->reg_a + regs->reg_l;
	imm_8 += ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 < regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu) ||
		(imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY)
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

adc_a_hl:
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = 0;
	address = GET_REAL_READ_ADDR(regs->reg_hl);
	imm_8 = regs->reg_a + *address;
	imm_8 += ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 < regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu) ||
		(imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY)
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (8);

adc_a_a:
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = 0;
	imm_8 = regs->reg_a + regs->reg_a;
	imm_8 += ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 < regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) < (regs->reg_a & 0xFu) ||
		(imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY)
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

sub_a_b:
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
	ADD_PC(1);
	regs->reg_f = FLAG_N;
	address = GET_REAL_READ_ADDR(regs->reg_hl);
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
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = FLAG_N;
	imm_8 = regs->reg_a - regs->reg_b;
	imm_8 -= ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 > regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu) ||
		(imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY)
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

sbc_a_c:
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = FLAG_N;
	imm_8 = regs->reg_a - regs->reg_c;
	imm_8 -= ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 > regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu) ||
		(imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY)
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

sbc_a_d:
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = FLAG_N;
	imm_8 = regs->reg_a - regs->reg_d;
	imm_8 -= ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 > regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu) ||
		(imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY)
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

sbc_a_e:
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = FLAG_N;
	imm_8 = regs->reg_a - regs->reg_e;
	imm_8 -= ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 > regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu) ||
		(imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY)
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

sbc_a_h:
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = FLAG_N;
	imm_8 = regs->reg_a - regs->reg_h;
	imm_8 -= ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 > regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu) ||
		(imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY)
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

sbc_a_l:
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = FLAG_N;
	imm_8 = regs->reg_a - regs->reg_l;
	imm_8 -= ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 > regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu) ||
		(imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY)
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

sbc_a_hl:
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = FLAG_N;
	address = GET_REAL_READ_ADDR(regs->reg_hl);
	imm_8 = regs->reg_a - *address;
	imm_8 -= ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 > regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu) ||
		(imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY)
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (8);

sbc_a_a:
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = FLAG_N;
	imm_8 = regs->reg_a - regs->reg_a;
	imm_8 -= ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 > regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu) ||
		(imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY)
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

and_a_b:
	ADD_PC(1);
	regs->reg_f = FLAG_H;
	regs->reg_a &= regs->reg_b;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

and_a_c:
	ADD_PC(1);
	regs->reg_f = FLAG_H;
	regs->reg_a &= regs->reg_c;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

and_a_d:
	ADD_PC(1);
	regs->reg_f = FLAG_H;
	regs->reg_a &= regs->reg_d;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

and_a_e:
	ADD_PC(1);
	regs->reg_f = FLAG_H;
	regs->reg_a &= regs->reg_e;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

and_a_h:
	ADD_PC(1);
	regs->reg_f = FLAG_H;
	regs->reg_a &= regs->reg_h;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

and_a_l:
	ADD_PC(1);
	regs->reg_f = FLAG_H;
	regs->reg_a &= regs->reg_l;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

and_a_hl:
	ADD_PC(1);
	regs->reg_f = FLAG_H;
	address = GET_REAL_READ_ADDR(regs->reg_hl);
	regs->reg_a &= *address;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

and_a_a:
	ADD_PC(1);
	regs->reg_f = FLAG_H;
	regs->reg_a &= regs->reg_a;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

xor_a_b:
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a ^= regs->reg_b;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

xor_a_c:
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a ^= regs->reg_c;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

xor_a_d:
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a ^= regs->reg_d;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

xor_a_e:
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a ^= regs->reg_e;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

xor_a_h:
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a ^= regs->reg_h;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

xor_a_l:
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a ^= regs->reg_l;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

xor_a_hl:
	ADD_PC(1);
	regs->reg_f = 0;
	address = GET_REAL_READ_ADDR(regs->reg_hl);
	regs->reg_a ^= *address;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

xor_a_a:
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a ^= regs->reg_a;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

or_a_b:
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a |= regs->reg_b;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

or_a_c:
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a |= regs->reg_c;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

or_a_d:
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a |= regs->reg_d;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

or_a_e:
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a |= regs->reg_e;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

or_a_h:
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a |= regs->reg_h;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

or_a_l:
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a |= regs->reg_l;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

or_a_hl:
	ADD_PC(1);
	regs->reg_f = 0;
	address = GET_REAL_READ_ADDR(regs->reg_hl);
	regs->reg_a |= *address;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (8);

or_a_a:
	ADD_PC(1);
	regs->reg_f = 0;
	regs->reg_a |= regs->reg_a;
	if (regs->reg_a == 0)
		regs->reg_f |= FLAG_Z;
	return (4);

cp_a_b:
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
	ADD_PC(1);
	regs->reg_f = FLAG_N;
	address = GET_REAL_READ_ADDR(regs->reg_hl);
	if (regs->reg_a == *address)
		regs->reg_f |= FLAG_Z;
	if (regs->reg_a < *address)
		regs->reg_f |= FLAG_CY;
	if ((regs->reg_a & 0xFu) < (*address & 0xFu))
		regs->reg_f |= FLAG_H;
	return (8);

cp_a_a:
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
	if ((regs->reg_f & FLAG_Z) == 0)
	{
		address = GET_REAL_READ_ADDR(regs->reg_sp);
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
	ADD_PC(1);
	address = GET_REAL_READ_ADDR(regs->reg_sp);
	regs->reg_sp += 2;
	regs->reg_bc = ((uint16_t)address[1] << 8) | (uint16_t)address[0];
	return (12);

jpnz_imm16:
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
	SET_PC(imm_16);
	return (16);

callnz_imm16:
	if ((regs->reg_f & FLAG_Z) == 0)
	{
		address = GET_REAL_WRITE_ADDR(regs->reg_sp);
		regs->reg_sp -= 2;
		address[-1] = (uint8_t)(imm_16 >> 8);
		address[-2] = (uint8_t)(imm_16);
		SET_PC(imm_16);
		return (24);
	}
	else
	{
		ADD_PC(3);
		return (12);
	}

push_bc:
	ADD_PC(1);
	address = GET_REAL_WRITE_ADDR(regs->reg_sp);
	regs->reg_sp -= 2;
	address[-1] = (uint8_t)(regs->reg_bc >> 8);
	address[-2] = (uint8_t)(regs->reg_bc);
	return (16);

add_a_imm8:
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
	ADD_PC(1);
	address = GET_REAL_WRITE_ADDR(regs->reg_sp);
	regs->reg_sp -= 2;
	address[-1] = (uint8_t)(regs->reg_pc >> 8);
	address[-2] = (uint8_t)(regs->reg_pc);
	regs->reg_pc = 0x00u;
	return (16);

retz:
	if ((regs->reg_f & FLAG_Z) == FLAG_Z)
	{
		address = GET_REAL_READ_ADDR(regs->reg_sp);
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
	address = GET_REAL_READ_ADDR(regs->reg_sp);
	regs->reg_sp += 2;
	SET_PC(((uint16_t)address[1] << 8) | (uint16_t)address[0]);
	return (20);

jpz_imm16:
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
	if ((regs->reg_f & FLAG_Z) == FLAG_Z)
	{
		address = GET_REAL_WRITE_ADDR(regs->reg_sp);
		regs->reg_sp -= 2;
		address[-1] = (uint8_t)(imm_16 >> 8);
		address[-2] = (uint8_t)(imm_16);
		SET_PC(imm_16);
		return (24);
	}
	else
	{
		ADD_PC(3);
		return (12);
	}

call_imm16:
	address = GET_REAL_WRITE_ADDR(regs->reg_sp);
	regs->reg_sp -= 2;
	address[-1] = (uint8_t)(imm_16 >> 8);
	address[-2] = (uint8_t)(imm_16);
	SET_PC(imm_16);
	return (24);

adc_a_imm8:
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
	ADD_PC(1);
	address = GET_REAL_WRITE_ADDR(regs->reg_sp);
	regs->reg_sp -= 2;
	address[-1] = (uint8_t)(regs->reg_pc >> 8);
	address[-2] = (uint8_t)(regs->reg_pc);
	regs->reg_pc = 0x08u;
	return (16);

retnc:
	if ((regs->reg_f & FLAG_CY) == 0)
	{
		address = GET_REAL_READ_ADDR(regs->reg_sp);
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
	ADD_PC(1);
	address = GET_REAL_READ_ADDR(regs->reg_sp);
	regs->reg_sp += 2;
	regs->reg_de = ((uint16_t)address[1] << 8) | (uint16_t)address[0];
	return (12);

jpnc_imm16:
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
	if ((regs->reg_f & FLAG_CY) == 0)
	{
		address = GET_REAL_WRITE_ADDR(regs->reg_sp);
		regs->reg_sp -= 2;
		address[-1] = (uint8_t)(imm_16 >> 8);
		address[-2] = (uint8_t)(imm_16);
		SET_PC(imm_16);
		return (24);
	}
	else
	{
		ADD_PC(3);
		return (12);
	}

push_de:
	ADD_PC(1);
	address = GET_REAL_WRITE_ADDR(regs->reg_sp);
	regs->reg_sp -= 2;
	address[-1] = (uint8_t)(regs->reg_de >> 8);
	address[-2] = (uint8_t)(regs->reg_de);
	return (16);

sub_a_imm8:
	ADD_PC(1);
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
	ADD_PC(1);
	address = GET_REAL_WRITE_ADDR(regs->reg_sp);
	regs->reg_sp -= 2;
	address[-1] = (uint8_t)(regs->reg_pc >> 8);
	address[-2] = (uint8_t)(regs->reg_pc);
	regs->reg_pc = 0x10u;
	return (16);

retc:
	if ((regs->reg_f & FLAG_CY) == FLAG_CY)
	{
		address = GET_REAL_READ_ADDR(regs->reg_sp);
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
	ADD_PC(1);

	// Va te faire ...

	return (16);

jpc_imm16:
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
	if ((regs->reg_f & FLAG_CY) == FLAG_CY)
	{
		address = GET_REAL_WRITE_ADDR(regs->reg_sp);
		regs->reg_sp -= 2;
		address[-1] = (uint8_t)(imm_16 >> 8);
		address[-2] = (uint8_t)(imm_16);
		SET_PC(imm_16);
		return (24);
	}
	else
	{
		ADD_PC(3);
		return (12);
	}

sbc_a_imm8:
	ADD_PC(1);
	tmpflag = regs->reg_f;
	regs->reg_f = FLAG_N;
	imm_8 = regs->reg_a - imm_8;
	imm_8 -= ((tmpflag & FLAG_CY) == FLAG_CY) ? 1 : 0;
	if (imm_8 > regs->reg_a ||
		(imm_8 == regs->reg_a && (tmpflag & FLAG_CY) == FLAG_CY))
		regs->reg_f |= FLAG_CY;
	if ((imm_8 & 0xFu) > (regs->reg_a & 0xFu) ||
		(imm_8 & 0xFu) == (regs->reg_a & 0xFu) && (tmpflag & FLAG_CY) == FLAG_CY)
		regs->reg_f |= FLAG_H;
	if (imm_8 == 0)
		regs->reg_f |= FLAG_Z;
	regs->reg_a = imm_8;
	return (4);

rst_18h:
	ADD_PC(1);
	address = GET_REAL_WRITE_ADDR(regs->reg_sp);
	regs->reg_sp -= 2;
	address[-1] = (uint8_t)(regs->reg_pc >> 8);
	address[-2] = (uint8_t)(regs->reg_pc);
	regs->reg_pc = 0x18u;
	return (16);

illegal:
	ADD_PC(1);
	return (0);

prefix_cb:
	ADD_PC(2);
	return (8);
}
