/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_get_bin_instruction.c                      .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/25 10:03:09 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/25 10:07:40 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "gbasm_struct.h"
#include "gbasm_tools.h"
#include "gbasm_error.h"

static ssize_t		instruction_search(const instruction_t inst[71], char *tofind)
{
	uint32_t	left, right, middle;
	int			side;

	left = 0;
	right = 71;
	while (left < right)
	{
		middle = (left + right) / 2;
		side = strcmp(inst[middle].name, tofind);
		if (side < 0)
			left = middle + 1;
		else if (side > 0)
			right = middle - 1;
		else
			return (middle);
	}
	return (0xffffffffu);
}

/*
 *	si le type recu est imm8, c'est peut-etre une imm16.
 *	penser a changer le type si necessaire. (idem pour addr8)
 */

extern param_error_t	get_bin_instruction(char *mnemonic, param_t param[2], value_t *val, uint8_t bin[4])
{
	static const instruction_t	inst[71] = {
		{"adc", &&__adc},		{"add", &&__add},		{"and", &&__and},		{"bit", &&__bit},
		{"call", &&__call},		{"callc", &&__callc},	{"callnc", &&__callnc}, {"callnz", &&__callnz},
		{"callz", &&__callz},	{"ccf", &&__ccf},		{"cmp", &&__cmp},		{"cp", &&__cp},
		{"cpl", &&__cpl},		{"daa", &&__daa},		{"dec", &&__dec},		{"di", &&__di},
		{"ei", &&__ei},			{"halt", &&__halt},		{"inc", &&__inc},		{"jp", &&__jp},
		{"jpc", &&__jpc},		{"jpnc", &&__jpnc},		{"jpnz", &&__jpnz},		{"jpz", &&__jpz},
		{"jr", &&__jr},			{"jrc", &&__jrc},		{"jrnc", &&__jrnc},		{"jrnz", &&__jrnz},
		{"jrz", &&__jrz},		{"ld", &&__ld},			{"ldd", &&__ldd},		{"ldhl", &&__ldhl},
		{"ldi", &&__ldi},		{"mov", &&__mov},		{"nop", &&__nop},		{"not", &&__not},
		{"or", &&__or},			{"pop", &&__pop},		{"push", &&__push},		{"res", &&__res},
		{"reset", &&__reset},	{"ret", &&__ret},		{"retc", &&__retc},		{"reti", &&__reti},
		{"retnc", &&__retnc},	{"retnz", &&__retnz},	{"retz", &&__retz},		{"rl", &&__rl},
		{"rla", &&__rla},		{"rlc", &&__rlc},		{"rlca", &&__rlca},		{"rr", &&__rr},
		{"rra", &&__rra},		{"rrc", &&__rrc},		{"rrca", &&__rrca},		{"rst", &&__rst},
		{"sar", &&__sar},		{"sbb", &&__sbb},		{"sbc", &&__sbc},		{"scf", &&__scf},
		{"set", &&__set},		{"shl", &&__shl},		{"shr", &&__shr},		{"sla", &&__sla},
		{"sra", &&__sra},		{"srl", &&__srl},		{"stop", &&__stop},		{"sub", &&__sub},
		{"swap", &&__swap},		{"testb", &&__testb},	{"xor", &&__xor}
	};
	uint32_t		index;
	param_error_t	error = {0, 0}; // error.p1, error.p2

	index = instruction_search(inst, mnemonic);
	if (index == 0xffffffffu)
	{
		error.p1 = 0xffffffffu;
		error.p2 = 0xffffffffu;
		return (error);
	}
	
//	new_instruction( VEC_ELEM(code_area_t, area, data->cur_area) );
	goto *(inst[index].addr);

__adc:
	if (param[0] == A)
	{
		switch (param[1])
		{
			case NONE:
			case A:
				bin[0] = 0x8Fu;
				goto __done;

			case B:
				bin[0] = 0x88u;
				goto __done;
				
			case C:
				bin[0] = 0x89u;
				goto __done;

			case D:
				bin[0] = 0x8Au;
				goto __done;

			case E:
				bin[0] = 0x8Bu;
				goto __done;

			case H:
				bin[0] = 0x8Cu;
				goto __done;

			case L:
				bin[0] = 0x8Du;
				goto __done;

			case HL_ADDR:
				bin[0] = 0x8Eu;
				goto __done;

			case IMM8:
				bin[0] = 0xCEu;
				bin[1] = (uint8_t)val->value;
				goto __done;

			default:
				error.p2 = INVAL_ADC_SBB_SRC;
				goto __done;
		}
	}
	if (param[1] != NONE)
	{
		error.p2 = TOO_MANY_PARAMS;
		goto __done;
	}
	switch (param[0])
	{
		case B:
			bin[0] = 0x88u;
			goto __done;

		case C:
			bin[0] = 0x89u;
			goto __done;

		case D:
			bin[0] = 0x8Au;
			goto __done;

		case E:
			bin[0] = 0x8Bu;
			goto __done;

		case H:
			bin[0] = 0x8Cu;
			goto __done;

		case L:
			bin[0] = 0x8Du;
			goto __done;

		case HL_ADDR:
			bin[0] = 0x8Eu;
			goto __done;

		case IMM8:
			bin[0] = 0xCEu;
			bin[1] = (uint8_t)val->value;
			goto __done;

		case NONE:
			error.p1 = MISSING_PARAM;
			goto __done;

		default:
			error.p1 = INVAL_ADC_SBB_SRC;
			goto __done;
	}

__add:
	switch (param[0])
	{
		case A:
			switch (param[1])
			{
				case A:
					bin[0] = 0x87u;
					goto __done;

				case B:
					bin[0] = 0x80u;
					goto __done;

				case C:
					bin[0] = 0x81u;
					goto __done;

				case D:
					bin[0] = 0x82u;
					goto __done;

				case E:
					bin[0] = 0x83u;
					goto __done;

				case H:
					bin[0] = 0x84u;
					goto __done;

				case L:
					bin[0] = 0x85u;
					goto __done;

				case HL_ADDR:
					bin[0] = 0x86u;
					goto __done;

				case IMM8:
					bin[0] = 0xC6u;
					bin[1] = (uint8_t)val->value;
					goto __done;

				case NONE:
					error.p2 = MISSING_PARAM;
					goto __done;

				default:
					error.p2 = INVAL_ADD_SUB_SRC;
					goto __done;
			}

		case HL:
			switch (param[1])
			{
				case BC:
					bin[0] = 0x09u;
					goto __done;

				case DE:
					bin[0] = 0x19u;
					goto __done;

				case HL:
					bin[0] = 0x29u;
					goto __done;

				case SP:
					bin[0] = 0x39u;
					goto __done;

				case NONE:
					error.p2 = MISSING_PARAM;
					goto __done;

				default:
					error.p2 = INVAL_ADD_SUB_SRC;
					goto __done;
			}

		case SP:
			if (param[1] == IMM8)
			{
				bin[0] = 0xE8u;
				bin[1] = (uint8_t)val->value;
			}
			else if (param[1] == NONE)
				error.p2 = MISSING_PARAM;
			else
				error.p2 = INVAL_ADD_SUB_SRC;
			goto __done;

		case NONE:
			error.p1 = MISSING_PARAM;
			goto __done;

		default:
			error.p1 = INVAL_ADD_SUB_DST;
			goto __done;
	}

__and:
	if (param[0] == A)
	{
		switch (param[1])
		{
			case NONE:
			case A:
				bin[0] = 0xA7u;
				goto __done;

			case B:
				bin[0] = 0xA0u;
				goto __done;
				
			case C:
				bin[0] = 0xA1u;
				goto __done;

			case D:
				bin[0] = 0xA2u;
				goto __done;

			case E:
				bin[0] = 0xA3u;
				goto __done;

			case H:
				bin[0] = 0xA4u;
				goto __done;

			case L:
				bin[0] = 0xA5u;
				goto __done;

			case HL_ADDR:
				bin[0] = 0xA6u;
				goto __done;

			case IMM8:
				bin[0] = 0xE6u;
				bin[1] = (uint8_t)val->value;
				goto __done;

			default:
				error.p2 = INVAL_BIT_ARITH_SRC;
				goto __done;
		}
	}
	if (param[1] != NONE)
	{
		error.p2 = TOO_MANY_PARAMS;
		goto __done;
	}
	switch (param[0])
	{
		case B:
			bin[0] = 0xA0u;
			goto __done;

		case C:
			bin[0] = 0xA1u;
			goto __done;

		case D:
			bin[0] = 0xA2u;
			goto __done;

		case E:
			bin[0] = 0xA3u;
			goto __done;

		case H:
			bin[0] = 0xA4u;
			goto __done;

		case L:
			bin[0] = 0xA5u;
			goto __done;

		case HL_ADDR:
			bin[0] = 0xA6u;
			goto __done;

		case IMM8:
			bin[0] = 0xE6u;
			bin[1] = (uint8_t)val->value;
			goto __done;

		case NONE:
			error.p1 = MISSING_PARAM;
			goto __done;

		default:
			error.p1 = INVAL_BIT_ARITH_SRC;
			goto __done;
	}

__bit:
	if (param[0] != IMM8 || val->value > 7)
	{
		error.p1 = INVAL_CB_BIT_VALUE;
		goto __done;
	}
	bin[0] = 0xCBu;
	val->value *= 8;
	switch (param[1])
	{
		case A:
			bin[1] = 0x47u + (uint8_t)val->value;
			goto __done;

		case B:
			bin[1] = 0x40u + (uint8_t)val->value;
			goto __done;

		case C:
			bin[1] = 0x41u + (uint8_t)val->value;
			goto __done;

		case D:
			bin[1] = 0x42u + (uint8_t)val->value;
			goto __done;

		case E:
			bin[1] = 0x43u + (uint8_t)val->value;
			goto __done;

		case H:
			bin[1] = 0x44u + (uint8_t)val->value;
			goto __done;

		case L:
			bin[1] = 0x45u + (uint8_t)val->value;
			goto __done;

		case HL_ADDR:
			bin[1] = 0x46u + (uint8_t)val->value;
			goto __done;

		case NONE:
			error.p1 = MISSING_PARAM;
			goto __done;

		default:
			error.p1 = INVAL_CB_BIT_DST;
			goto __done;
	}

__call:
	switch (param[0])
	{
		case IMM16:
		case ADDR16:
			bin[0] = 0xCDu;
			bin[1] = (uint8_t)val->value;
			bin[2] = (uint8_t)(val->value >> 8);
			goto __done;

		case _NZ_:
		case _Z_:
		case _NC_:
		case _C_:
			if (param[1] != IMM16 && param[1] != ADDR16)
			{
				error.p2 = (param[1] == NONE) ? MISSING_PARAM : INVAL_JP_CALL_ADDR;
				goto __done;
			}
			bin[1] = (uint8_t)val->value;
			bin[2] = (uint8_t)(val->value >> 8);
			if (param[0] == _NZ_)
				bin[0] = 0xC4u;
			else if (param[0] == _Z_)
				bin[0] = 0xCCu;
			else if (param[0] == _NC_)
				bin[0] = 0xD4u;
			else
				bin[0] = 0xDCu;
			goto __done;

		case NONE:
			error.p1 = MISSING_PARAM;
			goto __done;

		default:
			error.p1 = INVAL_JP_CALL_ADDR;
			goto __done;
	}

__callc:
	if (param[0] == IMM16 || param[0] == ADDR16)
	{
		bin[0] = 0xDCu;
		bin[1] = (uint8_t)val->value;
		bin[2] = (uint8_t)(val->value >> 8);
	}
	else if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else
		error.p1 = INVAL_JP_CALL_ADDR;
	goto __done;

__callnc:
	if (param[0] == IMM16 || param[0] == ADDR16)
	{
		bin[0] = 0xD4u;
		bin[1] = (uint8_t)val->value;
		bin[2] = (uint8_t)(val->value >> 8);
	}
	else if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else
		error.p1 = INVAL_JP_CALL_ADDR;
	goto __done;

__callnz:
	if (param[0] == IMM16 || param[0] == ADDR16)
	{
		bin[0] = 0xC4u;
		bin[1] = (uint8_t)val->value;
		bin[2] = (uint8_t)(val->value >> 8);
	}
	else if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else
		error.p1 = INVAL_JP_CALL_ADDR;
	goto __done;

__callz:
	if (param[0] == IMM16 || param[0] == ADDR16)
	{
		bin[0] = 0xCCu;
		bin[1] = (uint8_t)val->value;
		bin[2] = (uint8_t)(val->value >> 8);
	}
	else if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else
		error.p1 = INVAL_JP_CALL_ADDR;
	goto __done;

__ccf:
	if (param[0] != NONE)
		error.p1 = TOO_MANY_PARAMS;
	else
		bin[0] = 0x3Fu;
	goto __done;

__cmp:
__cp:
	if (param[0] == A)
	{
		switch (param[1])
		{
			case NONE:
			case A:
				bin[0] = 0xBFu;
				goto __done;

			case B:
				bin[0] = 0xB8u;
				goto __done;
				
			case C:
				bin[0] = 0xB9u;
				goto __done;

			case D:
				bin[0] = 0xBAu;
				goto __done;

			case E:
				bin[0] = 0xBBu;
				goto __done;

			case H:
				bin[0] = 0xBCu;
				goto __done;

			case L:
				bin[0] = 0xBDu;
				goto __done;

			case HL_ADDR:
				bin[0] = 0xBEu;
				goto __done;

			case IMM8:
				bin[0] = 0xFEu;
				bin[1] = (uint8_t)val->value;
				goto __done;

			default:
				error.p2 = INVAL_CMP_SRC;
				goto __done;
		}
	}
	if (param[1] != NONE)
	{
		error.p2 = TOO_MANY_PARAMS;
		goto __done;
	}
	switch (param[0])
	{
		case B:
			bin[0] = 0xB8u;
			goto __done;

		case C:
			bin[0] = 0xB9u;
			goto __done;

		case D:
			bin[0] = 0xBAu;
			goto __done;

		case E:
			bin[0] = 0xBBu;
			goto __done;

		case H:
			bin[0] = 0xBCu;
			goto __done;

		case L:
			bin[0] = 0xBDu;
			goto __done;

		case HL_ADDR:
			bin[0] = 0xBEu;
			goto __done;

		case IMM8:
			bin[0] = 0xFEu;
			bin[1] = (uint8_t)val->value;
			goto __done;

		case NONE:
			error.p1 = MISSING_PARAM;
			goto __done;

		default:
			error.p1 = INVAL_CMP_SRC;
			goto __done;
	}


__cpl:
__daa:
__dec:
__di:
__ei:
__halt:
__inc:
__jp:
__jpc:
__jpnc:
__jpnz:
__jpz:
__jr:
__jrc:
__jrnc:
__jrnz:
__jrz:
__ld:
__ldd:
__ldhl:
__ldi:
__mov:
__nop:
__not:
__or:
__pop:
__push:
__res:
__reset:
__ret:
__retc:
__reti:
__retnc:
__retnz:
__retz:
__rl:
__rla:
__rlc:
__rlca:
__rr:
__rra:
__rrc:
__rrca:
__rst:
__sar:
__sbb:
__sbc:
__scf:
__set:
__shl:
__shr:
__sla:
__sra:
__srl:
__stop:
__sub:
__swap:
__testb:
__xor:

__done:
	return (error);
}
