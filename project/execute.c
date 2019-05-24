/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   execute.c                                        .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/23 13:22:56 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/05/23 17:06:30 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "registers.h"
#include "bit_mask.h"

#define ADD_PC(offset)	regs->packed_bytes[5] += (offset)
#define SET_PC(value)	regs->packed_bytes[5] = (value)

#define CARRY_CHECK(source, result) ((source & BIT_7) && !(result & BIT_7))

cycle_count_t	execute(reg_t *regs, memory_map_t *map)
{
	const void* const opcodes_jump[256] =
	{
		&&nop,				&&ld_bc_imm16,		&&ld_bc_a,			&&inc_bc,
		&&inc_b,			&&dec_b,			&&ld_b_imm8,		&&rlca,
		&&ld_ad_sp,			&&add_hl_bc,		&&ld_a_bc,			&&dec_bc,
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
		&&cmp_a_b,			&&cmp_a_c,			&&cmp_a_d,			&&cmp_a_e,
		&&cmp_a_h,			&&cmp_a_l,			&&cmp_a_hl,			&&cmp_a_a,
		&&retnz,			&&pop_bc,			&&jpnz_imm16,		&&jp_imm16,
		&&callnz_imm16,		&&push_bc,			&&add_a_imm8,		&&rst_00h,
		&&retz,				&&ret,				&&jpz_imm16,		&&prefix_cb,
		&&callz_imm16,		&&call_imm16,		&&adc_a_imm8,		&&rst_08h,
		&&retnc,			&&pop_de,			&&jpnc_imm16,		&&illegal_inst,
		&&callnc_imm16,		&&push_de,			&&sub_a_imm8,		&&rst_10h,
		&&retc,				&&reti,				&&jpc_imm16,		&&illegal_inst,
		&&callc_imm16,		&&illegal_inst,		&&sbc_a_imm8,		&&rst_18h,
		&&ld_ff00_imm8_a,	&&pop_hl,			&&ld_ff00_c_a,		&&illegal_inst,
		&&illegal_inst,		&&push_hl,			&&and_a_imm8,		&&rst_20h,
		&&add_sp_imm8,		&&jp_hl,			&&ld_imm16_a,		&&illegal_inst,
		&&illegal_inst,		&&illegal_inst,		&&xor_a_imm8,		&&rst_28h,
		&&ld_ff00_a_imm8,	&&pop_af,			&&ld_ff00_a_c,		&&di,
		&&illegal_inst,		&&push_af,			&&or_a_imm8,		&&rst_30h,
		&&ld_hl_sp_imm8,	&&ld_sp_hl,			&&ld_a_imm16,		&&ei,
		&&illegal_inst,		&&illegal_inst,		&&cmp_a_imm8,		&&rst_38h
	};

	uint8_t		*current = map->rst_address + regs->reg_pc;
	uint8_t		opcode = current[0];
	uint16_t	immediate_16 = ((uint16_t)current[2] << 8) + (uint16_t)current[1];
	uint8_t		immediate_8 = current[1];

	goto *(opcodes_jump[opcode]);

nop:
	ADD_PC(1);
	return (4);

ld_bc_imm16:
	ADD_PC(3);
	regs->reg_bc = immediate_16;
	return (12);

ld_bc_a:
	ADD_PC(1);
	map->rst_address[regs->reg_bc] = regs->reg_a;
	return (8);

inc_bc:
	ADD_PC(1);
	regs->reg_bc++;
	return (8);

inc_b:
	ADD_PC(1);
	regs->reg_f &= FLAG_C;
	regs->reg_b++;
	
	if (regs->reg_b == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_b & BIT_4) && !(regs->reg_b & BITS_0123))
		regs->reg_f |= FLAG_H;
	
	return (4);

dec_b:
	ADD_PC(1);
	regs->reg_f &= FLAG_C;
	regs->reg_b--;

	if (regs->reg_b == 0)
		regs->reg_f |= FLAG_Z;
	if (!(regs->reg_b & BIT_4) && (regs->reg_b & BITS_0123) == BITS_0123)
		regs->reg_f |= FLAG_H;
	
	return (4);

ld_b_imm8:
	ADD_PC(2);
	regs->reg_b = immediate_8;
	return (8);

rlca:
	ADD_PC(1);
	regs->reg_f = ((regs->reg_a & BIT_7) == BIT_7);
	regs->reg_a <<= 1;
	regs->reg_a |= regs->reg_f;
	return (4);

ld_ad_sp:
	ADD_PC(3);
	map->rst_address[immediate_16] = (uint8_t)(regs->reg_sp);
	map->rst_address[immediate_16 + 1] = (uint8_t)(regs->reg_sp >> 8);
	return (20);

add_hl_bc:
	ADD_PC(1);
	regs->reg_f &= FLAG_Z;

	if ((regs->reg_hl & BIT_15) == BIT_15)
		regs->reg_f |= FLAG_C;
	if ((regs->reg_hl & BIT_11) == BIT_11)
		regs->reg_f |= FLAG_H;

	regs->reg_hl += regs->reg_bc;

	if ((regs->reg_hl & BIT_15) == BIT_15)
		regs->reg_f &= ~FLAG_C;
	if ((regs->reg_hl & BIT_11) == BIT_11)
		regs->reg_f &= ~FLAG_H;

	return (8);

ld_a_bc:
	ADD_PC(1);
	regs->reg_a = map->rst_address[regs->reg_bc];
	return (8);

dec_bc:
	ADD_PC(1);
	regs->reg_bc--;
	return (8);

inc_c:
	ADD_PC(1);
	regs->reg_f &= FLAG_C;
	regs->reg_c++;
	
	if (regs->reg_c == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_c & BIT_4) && !(regs->reg_c & BITS_0123))
		regs->reg_f |= FLAG_H;

	return (4);

dec_c:
	ADD_PC(1);
	regs->reg_f &= FLAG_C;
	regs->reg_c--;

	if (regs->reg_c == 0)
		regs->reg_f |= FLAG_Z;
	if (!(regs->reg_c & BIT_4) && (regs->reg_c & BITS_0123) == BITS_0123)
		regs->reg_f |= FLAG_H;

ld_c_imm8:
	ADD_PC(2);
	regs->reg_c = immediate_8;
	return (8);

rrca:
	ADD_PC(1);
	regs->reg_f = ((regs->reg_a & BIT_0) == BIT_0);
	regs->reg_a >>= 1;
	regs->reg_a |= (regs->reg_f << 7);
	return (4);

stop:
	ADD_PC(1);
	// We will see later
	return (4);

ld_de_imm16:
	ADD_PC(3);
	regs->reg_de = immediate_16;
	return (12);

ld_de_a:
	ADD_PC(1);
	map->rst_address[regs->reg_de] = regs->reg_a;
	return (8);

inc_de:
	ADD_PC(1);
	regs->reg_de++;
	return (8);

inc_d:
	ADD_PC(1);
	regs->reg_f &= FLAG_C;
	regs->reg_d++;
	
	if (regs->reg_d == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_d & BIT_4) && !(regs->reg_d & BITS_0123))
		regs->reg_f |= FLAG_H;
	
	return (4);

dec_d:
	ADD_PC(1);
	regs->reg_f &= FLAG_C;
	regs->reg_d--;

	if (regs->reg_d == 0)
		regs->reg_f |= FLAG_Z;
	if (!(regs->reg_d & BIT_4) && (regs->reg_d & BITS_0123) == BITS_0123)
		regs->reg_f |= FLAG_H;

	return (4);

ld_d_imm8:
	ADD_PC(2);
	regs->reg_d = immediate_8;
	return (8);

rla:
	ADD_PC(1);
	immediate_8 = regs->reg_f & FLAG_C;
	regs->reg_f = ((regs->reg_a & BIT_7) == BIT_7);
	regs->reg_a <<= 1;
	regs->reg_a |= immediate_8;
	return (4);

jr_imm8:
	ADD_PC((int8_t)immediate_8);
	return (12);

add_hl_de:
	ADD_PC(1);
	regs->reg_f &= FLAG_Z;

	if ((regs->reg_hl & BIT_15) == BIT_15)
		regs->reg_f |= FLAG_C;
	if ((regs->reg_hl & BIT_11) == BIT_11)
		regs->reg_f |= FLAG_H;

	regs->reg_hl += regs->reg_de;

	if ((regs->reg_hl & BIT_15) == BIT_15)
		regs->reg_f &= ~FLAG_C;
	if ((regs->reg_hl & BIT_11) == BIT_11)
		regs->reg_f &= ~FLAG_H;

	return (8);

ld_a_de:
	ADD_PC(1);
	regs->reg_a = map->rst_address[regs->reg_de];
	return (8);

dec_de:
	ADD_PC(1);
	regs->reg_de--;
	return (8);

inc_e:
	ADD_PC(1);
	regs->reg_f &= FLAG_C;
	regs->reg_e++;
	
	if (regs->reg_e == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_e & BIT_4) && !(regs->reg_e & BITS_0123))
		regs->reg_f |= FLAG_H;
	
	return (4);

dec_e:
	ADD_PC(1);
	regs->reg_f &= FLAG_C;
	regs->reg_e--;

	if (regs->reg_e == 0)
		regs->reg_f |= FLAG_Z;
	if (!(regs->reg_e & BIT_4) && (regs->reg_d & BITS_0123) == BITS_0123)
		regs->reg_f |= FLAG_H;

	return (4);

ld_e_imm8:
	ADD_PC(2);
	regs->reg_e = immediate_8;
	return (8);

rra:
	ADD_PC(1);
	immediate_8 = regs->reg_f & FLAG_C;
	regs->reg_f = ((regs->reg_a & BIT_0) == BIT_0);
	regs->reg_a >>= 1;
	regs->reg_a |= (immediate_8 << 7);
	return (4);

jrnz_imm8:
	if (!(regs->reg_f & FLAG_Z))
	{
		ADD_PC((int8_t)immediate_8);
		return (12);
	}
	else
	{
		ADD_PC(2);
		return (8);
	}

ld_hl_imm16:
	ADD_PC(3);
	regs->reg_hl = immediate_16;
	return (12);

ld_hli_a:
	ADD_PC(1);
	map->rst_address[regs->reg_hl++] = regs->reg_a;
	return (8);

inc_hl:
	ADD_PC(1);
	regs->reg_hl++;
	return (8);

inc_h:
	ADD_PC(1);
	regs->reg_f &= FLAG_C;
	regs->reg_h++;
	
	if (regs->reg_h == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_h & BIT_4) && !(regs->reg_h & BITS_0123))
		regs->reg_f |= FLAG_H;
	
	return (4);

dec_h:
	ADD_PC(1);
	regs->reg_f &= FLAG_C;
	regs->reg_h--;

	if (regs->reg_h == 0)
		regs->reg_f |= FLAG_Z;
	if (!(regs->reg_h & BIT_4) && (regs->reg_h & BITS_0123) == BITS_0123)
		regs->reg_f |= FLAG_H;

	return (4);

ld_h_imm8:
	ADD_PC(2);
	regs->reg_h = immediate_8;
	return (8);

daa:
	// Later
	return (4);

jrz_imm8:
	if (regs->reg_f & FLAG_Z)
	{
		ADD_PC((int8_t)immediate_8);
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

	if ((regs->reg_hl & BIT_15) == BIT_15)
		regs->reg_f |= FLAG_C;
	if ((regs->reg_hl & BIT_11) == BIT_11)
		regs->reg_f |= FLAG_H;

	regs->reg_hl += regs->reg_de;

	if ((regs->reg_hl & BIT_15) == BIT_15)
		regs->reg_f &= ~FLAG_C;
	if ((regs->reg_hl & BIT_11) == BIT_11)
		regs->reg_f &= ~FLAG_H;

	return (8);

ld_a_hli:
	ADD_PC(1);
	regs->reg_a = map->rst_address[regs->reg_hl++];
	return (8);

dec_hl:
	ADD_PC(1);
	regs->reg_hl--;
	return (8);

inc_l:
	ADD_PC(1);
	regs->reg_f &= FLAG_C;
	regs->reg_l++;
	
	if (regs->reg_l == 0)
		regs->reg_f |= FLAG_Z;
	if ((regs->reg_l & BIT_4) && !(regs->reg_h & BITS_0123))
		regs->reg_f |= FLAG_H;
	
	return (4);

dec_l:
	ADD_PC(1);
	regs->reg_f &= FLAG_C;
	regs->reg_l--;

	if (regs->reg_l == 0)
		regs->reg_f |= FLAG_Z;
	if (!(regs->reg_l & BIT_4) && (regs->reg_l & BITS_0123) == BITS_0123)
		regs->reg_f |= FLAG_H;

	return (4);

ld_l_imm8:
	ADD_PC(2);
	regs->reg_l = immediate_8;
	return (8);

cpl:
	ADD_PC(1);
	regs->reg_a ^= 0xFFu;
	return (4);

jrnc_imm8:
	if (!(regs->reg_f & FLAG_C))
	{
		ADD_PC((int8_t)immediate_8);
		return (12);
	}
	else
	{
		ADD_PC(2);
		return (8);
	}

ld_sp_imm16:
	ADD_PC(3);
	regs->reg_sp = immediate_16;
	return (12);

ld_hld_a:
	ADD_PC(1);
	map->rst_address[regs->reg_hl--] = regs->reg_a;
	return (8);

inc_sp:
	ADD_PC(1);
	regs->reg_sp++;
	return (8);

inc_ahl:
	ADD_PC(1);
	regs->reg_f &= FLAG_C;
	(map->rst_address[regs->reg_hl])++;
	
	if (map->rst_address[regs->reg_hl] == 0)
		regs->reg_f |= FLAG_Z;
	if ((map->rst_address[regs->reg_hl] & BIT_4) &&
		!(map->rst_address[regs->reg_hl] & BITS_0123))
		regs->reg_f |= FLAG_H;
	
	return (12);

dec_ahl:
	ADD_PC(1);
	regs->reg_f &= FLAG_C;
	(map->rst_address[regs->reg_hl])--;

	if (map->rst_address[regs->reg_hl] == 0)
		regs->reg_f |= FLAG_Z;
	if (!(map->rst_address[regs->reg_hl] & BIT_4) &&
		(map->rst_address[regs->reg_hl] & BITS_0123) == BITS_0123)
		regs->reg_f |= FLAG_H;

	return (12);

ld_hl_imm8:
	ADD_PC(2);
	map->rst_address[regs->reg_hl] = immediate_8;
	return (12);

scf:
	// Later
	return (4);

jrc_imm8:
add_hl_sp:
ld_a_hld:
dec_sp:

inc_a:
dec_a:
ld_a_imm8:
ccf:

ld_b_b:
ld_b_c:
ld_b_d:
ld_b_e:

ld_b_h:
ld_b_l:
ld_b_hl:
ld_b_a:

ld_c_b:
ld_c_c:
ld_c_d:
ld_c_e:

ld_c_h:
ld_c_l:
ld_c_hl:
ld_c_a:

ld_d_b:
ld_d_c:
ld_d_d:
ld_d_e:

ld_d_h:
ld_d_l:
ld_d_hl:
ld_d_a:

ld_e_b:
ld_e_c:
ld_e_d:
ld_e_e:

ld_e_h:
ld_e_l:
ld_e_hl:
ld_e_a:

ld_h_b:
ld_h_c:
ld_h_d:
ld_h_e:

ld_h_h:
ld_h_l:
ld_h_hl:
ld_h_a:

ld_l_b:
ld_l_c:
ld_l_d:
ld_l_e:

ld_l_h:
ld_l_l:
ld_l_hl:
ld_l_a:

ld_hl_b:
ld_hl_c:
ld_hl_d:
ld_hl_e:

ld_hl_h:
ld_hl_l:
halt:
ld_hl_a:

ld_a_b:
ld_a_c:
ld_a_d:
ld_a_e:

ld_a_h:
ld_a_l:
ld_a_hl:
ld_a_a:

add_a_b:
add_a_c:
add_a_d:
add_a_e:

add_a_h:
add_a_l:
add_a_hl:
add_a_a:

adc_a_b:
adc_a_c:
adc_a_d:
adc_a_e:

adc_a_h:
adc_a_l:
adc_a_hl:
adc_a_a:

sub_a_b:
sub_a_c:
sub_a_d:
sub_a_e:

sub_a_h:
sub_a_l:
sub_a_hl:
sub_a_a:

sbc_a_b:
sbc_a_c:
sbc_a_d:
sbc_a_e:

sbc_a_h:
sbc_a_l:
sbc_a_hl:
sbc_a_a:

and_a_b:
and_a_c:
and_a_d:
and_a_e:

and_a_h:
and_a_l:
and_a_hl:
and_a_a:

xor_a_b:
xor_a_c:
xor_a_d:
xor_a_e:

xor_a_h:
xor_a_l:
xor_a_hl:
xor_a_a:

or_a_b:
or_a_c:
or_a_d:
or_a_e:

or_a_h:
or_a_l:
or_a_hl:
or_a_a:

cmp_a_b:
cmp_a_c:
cmp_a_d:
cmp_a_e:

cmp_a_h:
cmp_a_l:
cmp_a_hl:
cmp_a_a:

retnz:
pop_bc:
jpnz_imm16:
jp_imm16:

callnz_imm16:
push_bc:
add_a_imm8:
rst_00h:

retz:
ret:
jpz_imm16:
prefix_cb:

callz_imm16:
call_imm16:
adc_a_imm8:
rst_08h:

illegal_inst:
	return (0);

retnc:
pop_de:
jpnc_imm16:

callnc_imm16:
push_de:
sub_a_imm8:
rst_10h:

retc:
reti:
jpc_imm16:

callc_imm16:
sbc_a_imm8:
rst_18h:

ld_ff00_imm8_a:
pop_hl:
ld_ff00_c_a:

push_hl:
and_a_imm8:
rst_20h:

add_sp_imm8:
jp_hl:
ld_imm16_a:

xor_a_imm8:
rst_28h:

ld_ff00_a_imm8:
pop_af:
ld_ff00_a_c:
di:

push_af:
or_a_imm8:
rst_30h:

ld_hl_sp_imm8:
ld_sp_hl:
ld_a_imm16:
ei:

cmp_a_imm8:
rst_38h:
	return (0);
}
