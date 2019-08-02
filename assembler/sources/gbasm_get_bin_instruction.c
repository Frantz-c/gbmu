/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_get_bin_instruction.c                      .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/25 10:03:09 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/02 09:51:34 by mhouppin    ###    #+. /#+    ###.fr     */
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
		printf("left = %u, right = %u\n\e[1;36mCOMPARE(\e[0;33m\"%s\"\e[1;36m, \e[0;33m\"%s\"\e[1;36m);\n\e[0m", left, right, inst[middle].name, tofind);
		side = strcmp(inst[middle].name, tofind);
		if (side < 0)
			left = middle + 1;
		else if (side > 0)
			right = middle;
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
		puts("__PROBLEM__");
		error.p1 = 0xffffffffu;
		error.p2 = 0xffffffffu;
		return (error);
	}
	
//	new_instruction( VEC_ELEM(code_area_t, area, data->cur_area) );
	goto *(inst[index].addr);

__adc:
	if (param[0] == NONE)
	{
		error.p1 = MISSING_PARAM;
	}
	else if (param[0] == A)
	{
		switch (param[1])
		{
			case A:
			case NONE:
				bin[0] = 0x8Fu;
				break ;

			case B:
				bin[0] = 0x88u;
				break ;

			case C:
				bin[0] = 0x89u;
				break ;

			case D:
				bin[0] = 0x8Au;
				break ;

			case E:
				bin[0] = 0x8Bu;
				break ;

			case H:
				bin[0] = 0x8Cu;
				break ;

			case L:
				bin[0] = 0x8Du;
				break ;

			case HL_ADDR:
				bin[0] = 0x8Eu;
				break ;

			case IMM8:
				bin[0] = 0xCEu;
				bin[1] = (uint8_t)val->value;
				break ;

			default:
				error.p2 = INVAL_SRC;
				break ;
		}
		break ;
	}
	else
	{
		if (param[1] != NONE)
		{
			error.p2 = TOO_MANY_PARAMS;
			goto __done;
		}
		switch (param[0])
		{
			case B:
				bin[0] = 0x88u;
				break ;

			case C:
				bin[0] = 0x89u;
				break ;

			case D:
				bin[0] = 0x8Au;
				break ;

			case E:
				bin[0] = 0x8Bu;
				break ;

			case H:
				bin[0] = 0x8Cu;
				break ;

			case L:
				bin[0] = 0x8Du;
				break ;

			case HL_ADDR:
				bin[0] = 0x8Eu;
				break ;

			case IMM8:
				bin[0] = 0xCEu;
				bin[1] = (uint8_t)val->value;
				break ;

			default:
				error.p1 = INVAL_SRC;
				break ;
		}
	}
	goto __done;

__add:
	switch (param[0])
	{
		case NONE:
			error.p1 = MISSING_PARAM;
			break ;

		case A:
			switch (param[1])
			{
				case NONE:
					error.p2 = MISSING_PARAM;
					break ;

				case A:
					bin[0] = 0x77u;
					break ;

				case B:
					bin[0] = 0x70u;
					break ;

				case C:
					bin[0] = 0x71u;
					break ;

				case D:
					bin[0] = 0x72u;
					break ;

				case E:
					bin[0] = 0x73u;
					break ;

				case H:
					bin[0] = 0x74u;
					break ;

				case L:
					bin[0] = 0x75u;
					break ;

				case HL_ADDR:
					bin[0] = 0x76u;
					break ;

				case IMM8:
					bin[0] = 0x77u;
					bin[1] = (uint8_t)val->value;
					break ;

				default:
					error.p2 = INVAL_SRC;
					break ;
			}
			break ;

		case HL:
			switch (param[1])
			{
				case NONE:
					error.p2 = MISSING_PARAM;
					break ;

				case BC:
					bin[0] = 0x09u;
					break ;

				case DE:
					bin[0] = 0x19u;
					break ;

				case HL:
					bin[0] = 0x29u;
					break ;

				case SP:
					bin[0] = 0x39u;
					break ;

				default:
					error.p2 = INVAL_SRC;
					break ;
			}
			break ;

		case SP:
			switch (param[1])
			{
				case NONE:
					error.p2 = MISSING_PARAM;
					break ;

				case IMM8:
					bin[0] = 0xE8u;
					bin[1] = (uint8_t)val->value;
					break ;

				default:
					error.p2 = INVAL_SRC;
					break ;
			}
			break ;

		default:
			error.p1 = INVAL_DST;
			break ;
	}
	goto __done;

__and:
	if (param[0] == NONE)
	{
		error.p1 = MISSING_PARAM;
	}
	else if (param[0] == A)
	{
		switch (param[1])
		{
			case A:
			case NONE:
				bin[0] = 0xA7u;
				break ;

			case B:
				bin[0] = 0xA0u;
				break ;

			case C:
				bin[0] = 0xA1u;
				break ;

			case D:
				bin[0] = 0xA2u;
				break ;

			case E:
				bin[0] = 0xA3u;
				break ;

			case H:
				bin[0] = 0xA4u;
				break ;

			case L:
				bin[0] = 0xA5u;
				break ;

			case HL_ADDR:
				bin[0] = 0xA6u;
				break ;

			case IMM8:
				bin[0] = 0xE6u;
				bin[1] = (uint8_t)val->value;
				break ;

			default:
				error.p2 = INVAL_SRC;
				break ;
		}
		break ;
	}
	else
	{
		if (param[1] != NONE)
		{
			error.p2 = TOO_MANY_PARAMS;
			goto __done;
		}
		switch (param[0])
		{
			case B:
				bin[0] = 0xA0u;
				break ;

			case C:
				bin[0] = 0xA1u;
				break ;

			case D:
				bin[0] = 0xA2u;
				break ;

			case E:
				bin[0] = 0xA3u;
				break ;

			case H:
				bin[0] = 0xA4u;
				break ;

			case L:
				bin[0] = 0xA5u;
				break ;

			case HL_ADDR:
				bin[0] = 0xA6u;
				break ;

			case IMM8:
				bin[0] = 0xE6u;
				bin[1] = (uint8_t)val->value;
				break ;

			default:
				error.p1 = INVAL_SRC;
				break ;
		}
	}
	goto __done;

__bit:
__call:
__callc:
__callnc:
__callnz:
__callz:
__ccf:
__cmp:
__cp:
	if (param[0] == NONE)
	{
		error.p1 = MISSING_PARAM;
	}
	else if (param[0] == A)
	{
		switch (param[1])
		{
			case A:
			case NONE:
				bin[0] = 0xBFu;
				break ;

			case B:
				bin[0] = 0xB8u;
				break ;

			case C:
				bin[0] = 0xB9u;
				break ;

			case D:
				bin[0] = 0xBAu;
				break ;

			case E:
				bin[0] = 0xBBu;
				break ;

			case H:
				bin[0] = 0xBCu;
				break ;

			case L:
				bin[0] = 0xBDu;
				break ;

			case HL_ADDR:
				bin[0] = 0xBEu;
				break ;

			case IMM8:
				bin[0] = 0xFEu;
				bin[1] = (uint8_t)val->value;
				break ;

			default:
				error.p2 = INVAL_SRC;
				break ;
		}
		break ;
	}
	else
	{
		if (param[1] != NONE)
		{
			error.p2 = TOO_MANY_PARAMS;
			goto __done;
		}
		switch (param[0])
		{
			case B:
				bin[0] = 0xB8u;
				break ;

			case C:
				bin[0] = 0xB9u;
				break ;

			case D:
				bin[0] = 0xBAu;
				break ;

			case E:
				bin[0] = 0xBBu;
				break ;

			case H:
				bin[0] = 0xBCu;
				break ;

			case L:
				bin[0] = 0xBDu;
				break ;

			case HL_ADDR:
				bin[0] = 0xBEu;
				break ;

			case IMM8:
				bin[0] = 0xFEu;
				bin[1] = (uint8_t)val->value;
				break ;

			default:
				error.p1 = INVAL_SRC;
				break ;
		}
	}
	goto __done;

__cpl:
__not:
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
__mov:
__ldd:
__ldhl:
__ldi:
__nop:
__or:
	if (param[0] == NONE)
	{
		error.p1 = MISSING_PARAM;
	}
	else if (param[0] == A)
	{
		switch (param[1])
		{
			case A:
			case NONE:
				bin[0] = 0xB7u;
				break ;

			case B:
				bin[0] = 0xB0u;
				break ;

			case C:
				bin[0] = 0xB1u;
				break ;

			case D:
				bin[0] = 0xB2u;
				break ;

			case E:
				bin[0] = 0xB3u;
				break ;

			case H:
				bin[0] = 0xB4u;
				break ;

			case L:
				bin[0] = 0xB5u;
				break ;

			case HL_ADDR:
				bin[0] = 0xB6u;
				break ;

			case IMM8:
				bin[0] = 0xF6u;
				bin[1] = (uint8_t)val->value;
				break ;

			default:
				error.p2 = INVAL_SRC;
				break ;
		}
		break ;
	}
	else
	{
		if (param[1] != NONE)
		{
			error.p2 = TOO_MANY_PARAMS;
			goto __done;
		}
		switch (param[0])
		{
			case B:
				bin[0] = 0xB0u;
				break ;

			case C:
				bin[0] = 0xB1u;
				break ;

			case D:
				bin[0] = 0xB2u;
				break ;

			case E:
				bin[0] = 0xB3u;
				break ;

			case H:
				bin[0] = 0xB4u;
				break ;

			case L:
				bin[0] = 0xB5u;
				break ;

			case HL_ADDR:
				bin[0] = 0xB6u;
				break ;

			case IMM8:
				bin[0] = 0xF6u;
				bin[1] = (uint8_t)val->value;
				break ;

			default:
				error.p1 = INVAL_SRC;
				break ;
		}
	}
	goto __done;

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
	if (param[0] == NONE)
	{
		error.p1 = MISSING_PARAM;
	}
	else if (param[0] == A)
	{
		switch (param[1])
		{
			case A:
			case NONE:
				bin[0] = 0x9Fu;
				break ;

			case B:
				bin[0] = 0x98u;
				break ;

			case C:
				bin[0] = 0x99u;
				break ;

			case D:
				bin[0] = 0x9Au;
				break ;

			case E:
				bin[0] = 0x9Bu;
				break ;

			case H:
				bin[0] = 0x9Cu;
				break ;

			case L:
				bin[0] = 0x9Du;
				break ;

			case HL_ADDR:
				bin[0] = 0x9Eu;
				break ;

			case IMM8:
				bin[0] = 0xDEu;
				bin[1] = (uint8_t)val->value;
				break ;

			default:
				error.p2 = INVAL_SRC;
				break ;
		}
		break ;
	}
	else
	{
		if (param[1] != NONE)
		{
			error.p2 = TOO_MANY_PARAMS;
			goto __done;
		}
		switch (param[0])
		{
			case B:
				bin[0] = 0x98u;
				break ;

			case C:
				bin[0] = 0x99u;
				break ;

			case D:
				bin[0] = 0x9Au;
				break ;

			case E:
				bin[0] = 0x9Bu;
				break ;

			case H:
				bin[0] = 0x9Cu;
				break ;

			case L:
				bin[0] = 0x9Du;
				break ;

			case HL_ADDR:
				bin[0] = 0x9Eu;
				break ;

			case IMM8:
				bin[0] = 0xDEu;
				bin[1] = (uint8_t)val->value;
				break ;

			default:
				error.p1 = INVAL_SRC;
				break ;
		}
	}
	goto __done;

__scf:
__set:
__shl:
__shr:
__sla:
__sra:
__srl:
__stop:
__sub:
	if (param[0] == NONE)
	{
		error.p1 = MISSING_PARAM;
	}
	else if (param[0] == A)
	{
		switch (param[1])
		{
			case A:
			case NONE:
				bin[0] = 0x97u;
				break ;

			case B:
				bin[0] = 0x90u;
				break ;

			case C:
				bin[0] = 0x91u;
				break ;

			case D:
				bin[0] = 0x92u;
				break ;

			case E:
				bin[0] = 0x93u;
				break ;

			case H:
				bin[0] = 0x94u;
				break ;

			case L:
				bin[0] = 0x95u;
				break ;

			case HL_ADDR:
				bin[0] = 0x96u;
				break ;

			case IMM8:
				bin[0] = 0xD6u;
				bin[1] = (uint8_t)val->value;
				break ;

			default:
				error.p2 = INVAL_SRC;
				break ;
		}
		break ;
	}
	else
	{
		if (param[1] != NONE)
		{
			error.p2 = TOO_MANY_PARAMS;
			goto __done;
		}
		switch (param[0])
		{
			case B:
				bin[0] = 0x90u;
				break ;

			case C:
				bin[0] = 0x91u;
				break ;

			case D:
				bin[0] = 0x92u;
				break ;

			case E:
				bin[0] = 0x93u;
				break ;

			case H:
				bin[0] = 0x94u;
				break ;

			case L:
				bin[0] = 0x95u;
				break ;

			case HL_ADDR:
				bin[0] = 0x96u;
				break ;

			case IMM8:
				bin[0] = 0xD6u;
				bin[1] = (uint8_t)val->value;
				break ;

			default:
				error.p1 = INVAL_SRC;
				break ;
		}
	}
	goto __done;

__swap:
__testb:
	return ((param_error_t){0x1,0x1});
__xor:
	if (param[0] == NONE)
	{
		error.p1 = MISSING_PARAM;
	}
	else if (param[0] == A)
	{
		switch (param[1])
		{
			case A:
			case NONE:
				bin[0] = 0xAFu;
				break ;

			case B:
				bin[0] = 0xA8u;
				break ;

			case C:
				bin[0] = 0xA9u;
				break ;

			case D:
				bin[0] = 0xAAu;
				break ;

			case E:
				bin[0] = 0xABu;
				break ;

			case H:
				bin[0] = 0xACu;
				break ;

			case L:
				bin[0] = 0xADu;
				break ;

			case HL_ADDR:
				bin[0] = 0xAEu;
				break ;

			case IMM8:
				bin[0] = 0xEEu;
				bin[1] = (uint8_t)val->value;
				break ;

			default:
				error.p2 = INVAL_SRC;
				break ;
		}
		break ;
	}
	else
	{
		if (param[1] != NONE)
		{
			error.p2 = TOO_MANY_PARAMS;
			goto __done;
		}
		switch (param[0])
		{
			case B:
				bin[0] = 0xA8u;
				break ;

			case C:
				bin[0] = 0xA9u;
				break ;

			case D:
				bin[0] = 0xAAu;
				break ;

			case E:
				bin[0] = 0xABu;
				break ;

			case H:
				bin[0] = 0xACu;
				break ;

			case L:
				bin[0] = 0xADu;
				break ;

			case HL_ADDR:
				bin[0] = 0xAEu;
				break ;

			case IMM8:
				bin[0] = 0xEEu;
				bin[1] = (uint8_t)val->value;
				break ;

			default:
				error.p1 = INVAL_SRC;
				break ;
		}
	}
	goto __done;

__done:
	return (error);
}
