/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_get_bin_instruction.c                      .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/25 10:03:09 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/05 14:23:12 by mhouppin    ###    #+. /#+    ###.fr     */
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
					bin[0] = 0x87u;
					break ;

				case B:
					bin[0] = 0x80u;
					break ;

				case C:
					bin[0] = 0x81u;
					break ;

				case D:
					bin[0] = 0x82u;
					break ;

				case E:
					bin[0] = 0x83u;
					break ;

				case H:
					bin[0] = 0x84u;
					break ;

				case L:
					bin[0] = 0x85u;
					break ;

				case HL_ADDR:
					bin[0] = 0x86u;
					break ;

				case IMM8:
					bin[0] = 0xC6u;
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
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[0] != IMM8 || val->value > 7)
		error.p1 = INVAL_SRC;
	else
	{
		val->value *= 8;
		bin[0] = 0xCB;
		switch (param[1])
		{
			case NONE:
				error.p2 = MISSING_PARAM;
				break ;

			case A:
				bin[1] = 0x47u + (uint8_t)val->value;
				break ;

			case B:
				bin[1] = 0x40u + (uint8_t)val->value;
				break ;

			case C:
				bin[1] = 0x41u + (uint8_t)val->value;
				break ;

			case D:
				bin[1] = 0x42u + (uint8_t)val->value;
				break ;

			case E:
				bin[1] = 0x43u + (uint8_t)val->value;
				break ;

			case H:
				bin[1] = 0x44u + (uint8_t)val->value;
				break ;

			case L:
				bin[1] = 0x45u + (uint8_t)val->value;
				break ;

			case HL_ADDR:
				bin[1] = 0x46u + (uint8_t)val->value;
				break ;

			default:
				error.p2 = INVAL_DST;
				break ;
		}
	}
	goto __done;

__call:
	switch (param[0])
	{
		case NONE:
			error.p1 = MISSING_PARAM;
			break ;

		case IMM16:
		case ADDR16:
		case SYMBOL:
			if (param[1] != NONE)
				error.p2 = TOO_MANY_PARAMS;
			else
			{
				bin[0] = 0xCDu;
				bin[1] = (uint8_t)val->value;
				bin[2] = (uint8_t)(val->value >> 8);
			}
			break ;

		case IMM8:
		case ADDR8:
			param[0] = IMM16;
			if (param[1] != NONE)
				error.p2 = TOO_MANY_PARAMS;
			else
			{
				bin[0] = 0xCDu;
				bin[1] = (uint8_t)val->value;
				bin[2] = 0;
			}
			break ;

		case _Z_:
		case _NZ_:
		case _C_:
		case _NC_:
			if (param[0] == _Z_)
				bin[0] = 0xCCu;
			else if (param[0] == _NZ_)
				bin[0] = 0xC4u;
			else if (param[0] == _C_)
				bin[0] = 0xDCu;
			else
				bin[0] = 0xD4u;
			switch (param[1])
			{
				case NONE:
					error.p2 = MISSING_PARAM;
					break ;

				case IMM16:
				case ADDR16:
				case SYMBOL:
					bin[1] = (uint8_t)val->value;
					bin[2] = (uint8_t)(val->value >> 8);
					break ;

				case IMM8:
				case ADDR8:
					param[1] = IMM16;
					bin[1] = (uint8_t)val->value;
					bin[2] = (uint8_t)(val->value >> 8);
					break ;

				default:
					error.p2 = INVAL_DST;
					break ;
			}
			break ;

		default:
			error.p2 = INVAL_DST;
			break ;
	}
	goto __done;

__callc:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else
	{
		bin[0] = 0xDCu;
		switch (param[0])
		{
			case NONE:
				error.p1 = MISSING_PARAM;
				break ;

			case IMM16:
			case ADDR16:
			case SYMBOL:
				bin[1] = (uint8_t)val->value;
				bin[2] = (uint8_t)(val->value >> 8);
				break ;

			case IMM8:
			case ADDR8:
				param[0] = IMM16;
				bin[1] = (uint8_t)val->value;
				bin[2] = 0;
				break ;

			default:
				error.p1 = INVAL_DST;
				break ;
		}
	}
	goto __done;

__callnc:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else
	{
		bin[0] = 0xD4u;
		switch (param[0])
		{
			case NONE:
				error.p1 = MISSING_PARAM;
				break ;

			case IMM16:
			case ADDR16:
				bin[1] = (uint8_t)val->value;
				bin[2] = (uint8_t)(val->value >> 8);
				break ;

			case IMM8:
			case ADDR8:
				param[0] = IMM16;
				bin[1] = (uint8_t)val->value;
				bin[2] = 0x00u;
				break ;

			case SYMBOL:
				bin[1] = (uint8_t)val->value;
				bin[2] = (uint8_t)(val->value >> 8);
				break ;

			default:
				error.p1 = INVAL_DST;
				break ;
		}
	}
	goto __done;

__callnz:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else
	{
		bin[0] = 0xC4u;
		switch (param[0])
		{
			case NONE:
				error.p1 = MISSING_PARAM;
				break ;

			case IMM16:
			case ADDR16:
				bin[1] = (uint8_t)val->value;
				bin[2] = (uint8_t)(val->value >> 8);
				break ;

			case IMM8:
			case ADDR8:
				param[0] = IMM16;
				bin[1] = (uint8_t)val->value;
				bin[2] = 0;
				break ;

			case SYMBOL:
				bin[1] = (uint8_t)val->value;
				bin[2] = (uint8_t)(val->value >> 8);
				break ;

			default:
				error.p1 = INVAL_DST;
				break ;
		}
	}
	goto __done;

__callz:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else
	{
		bin[0] = 0xCCu;
		switch (param[0])
		{
			case NONE:
				error.p1 = MISSING_PARAM;
				break ;

			case IMM16:
			case ADDR16:
				bin[1] = (uint8_t)val->value;
				bin[2] = (uint8_t)(val->value >> 8);
				break ;

			case IMM8:
			case ADDR8:
				param[0] = IMM16;
				bin[1] = (uint8_t)val->value;
				bin[2] = 0;
				break ;

			case SYMBOL:
				bin[1] = (uint8_t)val->value;
				bin[2] = (uint8_t)(val->value >> 8);
				break ;

			default:
				error.p1 = INVAL_DST;
				break ;
		}
	}
	goto __done;

__ccf:
	if (param[0] != NONE)
		error.p1 = TOO_MANY_PARAMS;
	else
		bin[0] = 0x3Fu;
	goto __done;

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
	if (param[0] != NONE && param[0] != A)
		error.p1 = TOO_MANY_PARAMS;
	else
		bin[0] = 0x2Fu;
	goto __done;

__daa:
	if (param[0] != NONE)
		error.p1 = TOO_MANY_PARAMS;
	else
		bin[0] = 0x27u;
	goto __done;

__dec:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else
	{
		switch (param[0])
		{
			case A:
				bin[0] = 0x3Cu;
				break ;

			case B:
				bin[0] = 0x04u;
				break ;

			case C:
				bin[0] = 0x0Cu;
				break ;

			case D:
				bin[0] = 0x14u;
				break ;

			case E:
				bin[0] = 0x1Cu;
				break ;

			case H:
				bin[0] = 0x24u;
				break ;

			case L:
				bin[0] = 0x2Cu;
				break ;

			case HL_ADDR:
				bin[0] = 0x34u;
				break ;

			case BC:
				bin[0] = 0x03u;
				break ;

			case DE:
				bin[0] = 0x13u;
				break ;

			case HL:
				bin[0] = 0x23u;
				break ;

			case SP:
				bin[0] = 0x33u;
				break ;

			default:
				error.p1 = INVAL_DST;
				break ;
		}
	}

__di:
	if (param[0] != NONE)
		error.p1 = TOO_MANY_PARAMS;
	else
		bin[0] = 0xF3u;
	goto __done;

__ei:
	if (param[0] != NONE)
		error.p1 = TOO_MANY_PARAMS;
	else
		bin[0] = 0xFBu;
	goto __done;

__halt:
	if (param[0] != NONE)
		error.p1 = TOO_MANY_PARAMS;
	else
		bin[0] = 0x76u;
	goto __done;

__inc:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else
	{
		switch (param[0])
		{
			case A:
				bin[0] = 0x3Du;
				break ;

			case B:
				bin[0] = 0x05u;
				break ;

			case C:
				bin[0] = 0x0Du;
				break ;

			case D:
				bin[0] = 0x15u;
				break ;

			case E:
				bin[0] = 0x1Du;
				break ;

			case H:
				bin[0] = 0x25u;
				break ;

			case L:
				bin[0] = 0x2Du;
				break ;

			case HL_ADDR:
				bin[0] = 0x35u;
				break ;

			case BC:
				bin[0] = 0x0Bu;
				break ;

			case DE:
				bin[0] = 0x1Bu;
				break ;

			case HL:
				bin[0] = 0x2Bu;
				break ;

			case SP:
				bin[0] = 0x3Bu;
				break ;

			default:
				error.p1 = INVAL_DST;
				break ;
		}
	}

__jp:
	switch (param[0])
	{
		case NONE:
			error.p1 = MISSING_PARAM;
			break ;

		case HL_ADDR:
			if (param[1] != NONE)
				error.p2 = TOO_MANY_PARAMS;
			else
				bin[0] = 0xE9u;
			break ;

		case IMM16:
		case ADDR16:
		case SYMBOL:
			if (param[1] != NONE)
				error.p2 = TOO_MANY_PARAMS;
			else
			{
				bin[0] = 0xC3u;
				bin[1] = (uint8_t)val->value;
				bin[2] = (uint8_t)(val->value >> 8);
			}
			break ;

		case IMM8:
		case ADDR8:
			param[0] = IMM16;
			if (param[1] != NONE)
				error.p2 = TOO_MANY_PARAMS;
			else
			{
				bin[0] = 0xC3u;
				bin[1] = (uint8_t)val->value;
				bin[2] = 0;
			}
			break ;

		case _Z_:
		case _NZ_:
		case _C_:
		case _NC_:
			if (param[0] == _Z_)
				bin[0] = 0xCAu;
			else if (param[0] == _NZ_)
				bin[0] = 0xC2u;
			else if (param[0] == _C_)
				bin[0] = 0xDAu;
			else
				bin[0] = 0xD2u;
			switch (param[1])
			{
				case NONE:
					error.p2 = MISSING_PARAM;
					break ;

				case IMM16:
				case ADDR16:
				case SYMBOL:
					bin[1] = (uint8_t)val->value;
					bin[2] = (uint8_t)(val->value >> 8);
					break ;

				case IMM8:
				case ADDR8:
					param[1] = IMM16;
					bin[1] = (uint8_t)val->value;
					bin[2] = (uint8_t)(val->value >> 8);
					break ;

				default:
					error.p2 = INVAL_DST;
					break ;
			}
			break ;

		default:
			error.p2 = INVAL_DST;
			break ;
	}
	goto __done;

__jpc:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else
	{
		bin[0] = 0xDAu;
		switch (param[0])
		{
			case NONE:
				error.p1 = MISSING_PARAM;
				break ;

			case IMM16:
			case ADDR16:
			case SYMBOL:
				bin[1] = (uint8_t)val->value;
				bin[2] = (uint8_t)(val->value >> 8);
				break ;

			case IMM8:
			case ADDR8:
				param[0] = IMM16;
				bin[1] = (uint8_t)val->value;
				bin[2] = 0;
				break ;

			default:
				error.p1 = INVAL_DST;
				break ;
		}
	}
	goto __done;

__jpnc:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else
	{
		bin[0] = 0xD2u;
		switch (param[0])
		{
			case NONE:
				error.p1 = MISSING_PARAM;
				break ;

			case IMM16:
			case ADDR16:
			case SYMBOL:
				bin[1] = (uint8_t)val->value;
				bin[2] = (uint8_t)(val->value >> 8);
				break ;

			case IMM8:
			case ADDR8:
				param[0] = IMM16;
				bin[1] = (uint8_t)val->value;
				bin[2] = 0;
				break ;

			default:
				error.p1 = INVAL_DST;
				break ;
		}
	}
	goto __done;

__jpnz:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else
	{
		bin[0] = 0xC2u;
		switch (param[0])
		{
			case NONE:
				error.p1 = MISSING_PARAM;
				break ;

			case IMM16:
			case ADDR16:
			case SYMBOL:
				bin[1] = (uint8_t)val->value;
				bin[2] = (uint8_t)(val->value >> 8);
				break ;

			case IMM8:
			case ADDR8:
				param[0] = IMM16;
				bin[1] = (uint8_t)val->value;
				bin[2] = 0;
				break ;

			default:
				error.p1 = INVAL_DST;
				break ;
		}
	}
	goto __done;

__jpz:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else
	{
		bin[0] = 0xCAu;
		switch (param[0])
		{
			case NONE:
				error.p1 = MISSING_PARAM;
				break ;

			case IMM16:
			case ADDR16:
			case SYMBOL:
				bin[1] = (uint8_t)val->value;
				bin[2] = (uint8_t)(val->value >> 8);
				break ;

			case IMM8:
			case ADDR8:
				param[0] = IMM16;
				bin[1] = (uint8_t)val->value;
				bin[2] = 0;
				break ;

			default:
				error.p1 = INVAL_DST;
				break ;
		}
	}
	goto __done;

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
	return ((param_error_t){0x1,0x1});
__nop:
	if (param[0] != NONE)
		error.p1 = TOO_MANY_PARAMS;
	else
		bin[0] = 0x00u;
	goto __done;

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
	return ((param_error_t){0x1,0x1});
__rla:
	if (param[0] != NONE)
		error.p1 = TOO_MANY_PARAMS;
	else
		bin[0] = 0x17u;
	goto __done;

__rlc:
	return ((param_error_t){0x1,0x1});
__rlca:
	if (param[0] != NONE)
		error.p1 = TOO_MANY_PARAMS;
	else
		bin[0] = 0x07u;
	goto __done;

__rr:
	return ((param_error_t){0x1,0x1});
__rra:
	if (param[0] != NONE)
		error.p1 = TOO_MANY_PARAMS;
	else
		bin[0] = 0x1Fu;
	goto __done;
__rrc:
	return ((param_error_t){0x1,0x1});
__rrca:
	if (param[0] != NONE)
		error.p1 = TOO_MANY_PARAMS;
	else
		bin[0] = 0x0Fu;
	goto __done;

__rst:
__sar:
	return ((param_error_t){0x1,0x1});
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
	if (param[0] != NONE)
		error.p1 = TOO_MANY_PARAMS;
	else
		bin[0] = 0x3u;
	goto __done;

__set:
__shl:
__shr:
__sla:
__sra:
__srl:
	return ((param_error_t){0x1,0x1});
__stop:
	if (param[0] != NONE && !(param[0] == IMM8 && val->value == 0))
		error.p1 = TOO_MANY_PARAMS;
	else
	{
		bin[0] = 0x10u;
		bin[1] = 0x00u;
		param[0] = IMM8;
	}
	goto __done;

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
	return ((param_error_t){0x1,0x1});
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
