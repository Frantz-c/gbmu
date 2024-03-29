/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   get_bin_instruction.c                            .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/25 10:03:09 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/30 15:12:34 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "struct.h"
#include "tools.h"
#include "error.h"

#define	N_MNEMONICS	73

static ssize_t		instruction_search(const instruction_t inst[N_MNEMONICS], char *tofind)
{
	uint32_t	left, right, middle;
	int			side;

	left = 0;
	right = N_MNEMONICS;
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
	static const instruction_t	inst[N_MNEMONICS] = {
		{"adc", &&__adc},		{"add", &&__add},		{"and", &&__and},		{"bit", &&__bit},
		{"call", &&__call},		{"callc", &&__callc},	{"callnc", &&__callnc}, {"callnz", &&__callnz},
		{"callz", &&__callz},	{"ccf", &&__ccf},		{"cmp", &&__cmp},		{"cp", &&__cp},
		{"cpl", &&__cpl},		{"daa", &&__daa},		{"dec", &&__dec},		{"di", &&__di},
		{"ei", &&__ei},			{"halt", &&__halt},		{"inc", &&__inc},		{"jp", &&__jp},
		{"jpc", &&__jpc},		{"jpnc", &&__jpnc},		{"jpnz", &&__jpnz},		{"jpz", &&__jpz},
		{"jr", &&__jr},			{"jrc", &&__jrc},		{"jrnc", &&__jrnc},		{"jrnz", &&__jrnz},
		{"jrz", &&__jrz},		{"ld", &&__ld},			{"ldd", &&__ldd},		{"ldff", &&__ldff},
		{"ldh", &&__ldff},		{"ldhl", &&__ldhl},		{"ldi", &&__ldi},		{"mov", &&__mov},
		{"nop", &&__nop},		{"not", &&__not},		{"or", &&__or},			{"pop", &&__pop},
		{"push", &&__push},		{"res", &&__res},		{"reset", &&__reset},	{"ret", &&__ret},
		{"retc", &&__retc},		{"reti", &&__reti},		{"retnc", &&__retnc},	{"retnz", &&__retnz},
		{"retz", &&__retz},		{"rl", &&__rl},			{"rla", &&__rla},		{"rlc", &&__rlc},
		{"rlca", &&__rlca},		{"rr", &&__rr},			{"rra", &&__rra},		{"rrc", &&__rrc},
		{"rrca", &&__rrca},		{"rst", &&__rst},		{"sar", &&__sar},		{"sbb", &&__sbb},
		{"sbc", &&__sbc},		{"scf", &&__scf},		{"set", &&__set},		{"shl", &&__shl},
		{"shr", &&__shr},		{"sla", &&__sla},		{"sra", &&__sra},		{"srl", &&__srl},
		{"stop", &&__stop},		{"sub", &&__sub},		{"swap", &&__swap},		{"testb", &&__testb},
		{"xor", &&__xor}
	};
	uint32_t		index;
	param_error_t	error = {0, 0}; // error.p1, error.p2

	index = instruction_search(inst, mnemonic);
	if (index == 0xffffffffu)
	{
		puts("__PROBLEM__");
		error.p1 = UNKNOWN_INSTRUCTION;
		error.p2 = 0;
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
__testb:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[0] != IMM8 || val->value > 7)
		error.p1 = INVAL_SRC;
	else
	{
		val->value *= 8;
		bin[0] = 0xCBu;
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
				bin[0] = 0xCCu;
				goto __call_check_param2;
		case _NZ_:
				bin[0] = 0xC4u;
				goto __call_check_param2;
		case _C_: case C:
				bin[0] = 0xDCu;
				goto __call_check_param2;
		case _NC_:
				bin[0] = 0xD4u;

	__call_check_param2:
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
			error.p1 = INVAL_DST;
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
	goto __done;

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
	goto __done;

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
			bin[0] = 0xCAu;
			goto __jp_check_param2;
		case _NZ_:
			bin[0] = 0xC2u;
			goto __jp_check_param2;
		case _C_: case C:
			bin[0] = 0xDAu;
			goto __jp_check_param2;
		case _NC_:
			bin[0] = 0xD2u;

		__jp_check_param2:
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
	switch (param[0])
	{
		case NONE:
			error.p1 = MISSING_PARAM;
			break;

		case IMM16:
		case ADDR16:
			error.p1 = OVERFLOW;
			break ;

		case SYMBOL:
		case IMM8:
		case ADDR8:
			if (param[1] != NONE)
				error.p2 = TOO_MANY_PARAMS;
			else
			{
				bin[0] = 0x18u;
				bin[1] = (uint8_t)val->value;
			}
			break ;

		case _Z_:
			bin[0] = 0x28u;
			goto __jr_check_param2;
		case _NZ_:
			bin[0] = 0x20u;
			goto __jr_check_param2;
		case _C_: case C:
			bin[0] = 0x38u;
			goto __jr_check_param2;
		case _NC_:
			bin[0] = 0x30u;

	__jr_check_param2:
			switch (param[1])
			{
				case NONE:
					error.p2 = MISSING_PARAM;
					break ;

				case IMM16:
				case ADDR16:
					error.p2 = OVERFLOW;
					break ;

				case SYMBOL:
				case IMM8:
				case ADDR8:
					bin[1] = (uint8_t)val->value;
					break ;

				default:
					error.p2 = INVAL_DST;
					break ;
			}
			break ;

		default:
			error.p1 = INVAL_DST;
			break ;
	}
	goto __done;

__jrc:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else
	{
		switch (param[0])
		{
			case IMM16:
			case ADDR16:
				error.p1 = OVERFLOW;
				break ;

			case SYMBOL:
			case IMM8:
			case ADDR8:
				bin[0] = 0x38u;
				bin[1] = (uint8_t)val->value;
				break ;

			default:
				error.p1 = INVAL_DST;
				break ;
		}
	}
	goto __done;

__jrnc:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else
	{
		switch (param[0])
		{
			case IMM16:
			case ADDR16:
				error.p1 = OVERFLOW;
				break ;

			case SYMBOL:
			case IMM8:
			case ADDR8:
				bin[0] = 0x30u;
				bin[1] = (uint8_t)val->value;
				break ;

			default:
				error.p1 = INVAL_DST;
				break ;
		}
	}
	goto __done;

__jrnz:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else
	{
		switch (param[0])
		{
			case IMM16:
			case ADDR16:
				error.p1 = OVERFLOW;
				break ;

			case SYMBOL:
			case IMM8:
			case ADDR8:
				bin[0] = 0x20u;
				bin[1] = (uint8_t)val->value;
				break ;

			default:
				error.p1 = INVAL_DST;
				break ;
		}
	}
	goto __done;

__jrz:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else
	{
		switch (param[0])
		{
			case IMM16:
			case ADDR16:
				error.p1 = OVERFLOW;
				break ;

			case SYMBOL:
			case IMM8:
			case ADDR8:
				bin[0] = 0x28u;
				bin[1] = (uint8_t)val->value;
				break ;

			default:
				error.p1 = INVAL_DST;
				break ;
		}
	}
	goto __done;

__ld:
__mov:
	if (param[0] == NONE)
	{
		error.p1 = MISSING_PARAM;
		goto __done;
	}
	if (param[1] == NONE)
	{
		error.p2 = MISSING_PARAM;
		goto __done;
	}
	switch (param[0])
	{
		case BC:
			switch (param[1])
			{
				case IMM8:
					param[1] = IMM16;

				case SYMBOL:
				case IMM16:
					bin[0] = 0x01u;
					bin[1] = (uint8_t)val->value;
					bin[2] = (uint8_t)(val->value >> 8);
					break ;

				default:
					error.p2 = INVAL_SRC;
					break ;
			}
			break ;

		case DE:
			switch (param[1])
			{
				case IMM8:
					param[1] = IMM16;

				case SYMBOL:
				case IMM16:
					bin[0] = 0x11u;
					bin[1] = (uint8_t)val->value;
					bin[2] = (uint8_t)(val->value >> 8);
					break ;

				default:
					error.p2 = INVAL_SRC;
					break ;
			}
			break ;

		case HL:
			switch (param[1])
			{
				case IMM8:
					param[1] = IMM16;

				case SYMBOL:
				case IMM16:
					bin[0] = 0x21u;
					bin[1] = (uint8_t)val->value;
					bin[2] = (uint8_t)(val->value >> 8);
					break ;

				case SP_IMM8:
					bin[0] = 0xF8u;
					bin[1] = (uint8_t)val->value;
					break ;

				default:
					error.p2 = INVAL_SRC;
					break ;
			}
			break ;

		case SP:
			switch (param[1])
			{
				case IMM8:
					param[1] = IMM16;

				case SYMBOL:
				case IMM16:
					bin[0] = 0x31u;
					bin[1] = (uint8_t)val->value;
					bin[2] = (uint8_t)(val->value >> 8);
					break ;

				case HL:
					bin[0] = 0xF9u;
					break ;

				default:
					error.p2 = INVAL_SRC;
					break ;
			}
			break ;

		case ADDR8:
		case ADDR16:
			switch (param[1])
			{
				case SP:
					param[0] = ADDR16;
					bin[0] = 0x08u;
					bin[1] = (uint8_t)val->value;
					bin[2] = (uint8_t)(val->value >> 8);
					break ;

				case A:
					if (val->value > 0xFF00u)
					{
						param[0] = ADDR8;
						bin[0] = 0xE0u;
						bin[1] = (uint8_t)val->value;
					}
					else
					{
						param[0] = ADDR16;
						bin[0] = 0xEAu;
						bin[1] = (uint8_t)val->value;
						bin[2] = (uint8_t)(val->value >> 8);
					}
					break ;

				default:
					error.p2 = INVAL_SRC;
					break ;
			}
			break ;

		case SYMBOL:
			switch (param[1])
			{
				case SP:
					bin[0] = 0x08u;
					bin[1] = (uint8_t)val->value;
					bin[2] = (uint8_t)(val->value >> 8);
					break ;

				case A:
					bin[0] = 0xEAu;
					bin[1] = (uint8_t)val->value;
					bin[2] = (uint8_t)(val->value >> 8);
					break ;

				default:
					error.p2 = INVAL_SRC;
					break ;
			}
			break ;

		case BC_ADDR:
			switch (param[1])
			{
				case A:
					bin[0] = 0x02u;
					break ;

				default:
					error.p2 = INVAL_SRC;
					break ;
			}
			break ;

		case DE_ADDR:
			switch (param[1])
			{
				case A:
					bin[0] = 0x12u;
					break ;

				default:
					error.p2 = INVAL_SRC;
					break ;
			}
			break ;

		case HLI:
			switch (param[1])
			{
				case A:
					bin[0] = 0x22u;
					break ;

				default:
					error.p2 = INVAL_SRC;
					break ;
			}
			break ;

		case HLD:
			switch (param[1])
			{
				case A:
					bin[0] = 0x32u;
					break ;

				default:
					error.p2 = INVAL_SRC;
					break ;
			}
			break ;

		case FF00_C:
			switch (param[1])
			{
				case A:
					bin[0] = 0xE2u;
					break ;

				default:
					error.p2 = INVAL_SRC;
					break ;
			}
			break ;

		case FF00_IMM8:
			switch (param[1])
			{
				case A:
					bin[0] = 0xE0u;
					bin[1] = (uint8_t)val->value;
					break ;

				default:
					error.p2 = INVAL_SRC;
					break ;
			}
			break ;

		case A:
			switch (param[1])
			{
				case BC_ADDR:
					bin[0] = 0x0Au;
					break ;

				case DE_ADDR:
					bin[0] = 0x1Au;
					break ;

				case HLI:
					bin[0] = 0x2Au;
					break ;

				case HLD:
					bin[0] = 0x3Au;
					break ;

				case IMM8:
					bin[0] = 0x3Eu;
					bin[1] = (uint8_t)val->value;
					break ;

				case B:
					bin[0] = 0x78u;
					break ;

				case C:
					bin[0] = 0x79u;
					break ;

				case D:
					bin[0] = 0x7Au;
					break ;

				case E:
					bin[0] = 0x7Bu;
					break ;

				case H:
					bin[0] = 0x7Cu;
					break ;

				case L:
					bin[0] = 0x7Du;
					break ;

				case HL_ADDR:
					bin[0] = 0x7Eu;
					break ;

				case A:
					bin[0] = 0x7Fu;
					break ;

				case ADDR8:
				case ADDR16:
					if (val->value > 0xFF00u)
					{
						param[0] = ADDR8;
						bin[0] = 0xF0u;
						bin[1] = (uint8_t)val->value;
					}
					else
					{
						param[0] = ADDR16;
						bin[0] = 0xFAu;
						bin[1] = (uint8_t)val->value;
						bin[2] = (uint8_t)(val->value >> 8);
					}
					break ;

				case FF00_C:
					bin[0] = 0xF2u;
					break ;

				case FF00_IMM8:
					bin[0] = 0xF0u;
					bin[1] = (uint8_t)val->value;
					break ;

				case SYMBOL:
					bin[0] = 0xFAu;
					bin[1] = (uint8_t)val->value;
					bin[2] = (uint8_t)(val->value >> 8);
					break ;

				default:
					error.p2 = INVAL_SRC;
					break ;
			}
			break ;

		case B:
			switch (param[1])
			{
				case IMM8:
					bin[0] = 0x06u;
					bin[1] = (uint8_t)val->value;
					break ;

				case B:
					bin[0] = 0x40u;
					break ;

				case C:
					bin[0] = 0x41u;
					break ;

				case D:
					bin[0] = 0x42u;
					break ;

				case E:
					bin[0] = 0x43u;
					break ;

				case H:
					bin[0] = 0x44u;
					break ;

				case L:
					bin[0] = 0x45u;
					break ;

				case HL_ADDR:
					bin[0] = 0x46u;
					break ;

				case A:
					bin[0] = 0x47u;
					break ;

				default:
					error.p2 = INVAL_SRC;
					break ;
			}
			break ;

		case C:
			switch (param[1])
			{
				case IMM8:
					bin[0] = 0x0Eu;
					bin[1] = (uint8_t)val->value;
					break ;

				case B:
					bin[0] = 0x48u;
					break ;

				case C:
					bin[0] = 0x49u;
					break ;

				case D:
					bin[0] = 0x4Au;
					break ;

				case E:
					bin[0] = 0x4Bu;
					break ;

				case H:
					bin[0] = 0x4Cu;
					break ;

				case L:
					bin[0] = 0x4Du;
					break ;

				case HL_ADDR:
					bin[0] = 0x4Eu;
					break ;

				case A:
					bin[0] = 0x4Fu;
					break ;

				default:
					error.p2 = INVAL_SRC;
					break ;
			}
			break ;

		case D:
			switch (param[1])
			{
				case IMM8:
					bin[0] = 0x16u;
					bin[1] = (uint8_t)val->value;
					break ;

				case B:
					bin[0] = 0x50u;
					break ;

				case C:
					bin[0] = 0x51u;
					break ;

				case D:
					bin[0] = 0x52u;
					break ;

				case E:
					bin[0] = 0x53u;
					break ;

				case H:
					bin[0] = 0x54u;
					break ;

				case L:
					bin[0] = 0x55u;
					break ;

				case HL_ADDR:
					bin[0] = 0x56u;
					break ;

				case A:
					bin[0] = 0x57u;
					break ;

				default:
					error.p2 = INVAL_SRC;
					break ;
			}
			break ;

		case E:
			switch (param[1])
			{
				case IMM8:
					bin[0] = 0x1Eu;
					bin[1] = (uint8_t)val->value;
					break ;

				case B:
					bin[0] = 0x58u;
					break ;

				case C:
					bin[0] = 0x59u;
					break ;

				case D:
					bin[0] = 0x5Au;
					break ;

				case E:
					bin[0] = 0x5Bu;
					break ;

				case H:
					bin[0] = 0x5Cu;
					break ;

				case L:
					bin[0] = 0x5Du;
					break ;

				case HL_ADDR:
					bin[0] = 0x5Eu;
					break ;

				case A:
					bin[0] = 0x5Fu;
					break ;

				default:
					error.p2 = INVAL_SRC;
					break ;
			}
			break ;

		case H:
			switch (param[1])
			{
				case IMM8:
					bin[0] = 0x26u;
					bin[1] = (uint8_t)val->value;
					break ;

				case B:
					bin[0] = 0x60u;
					break ;

				case C:
					bin[0] = 0x61u;
					break ;

				case D:
					bin[0] = 0x62u;
					break ;

				case E:
					bin[0] = 0x63u;
					break ;

				case H:
					bin[0] = 0x64u;
					break ;

				case L:
					bin[0] = 0x65u;
					break ;

				case HL_ADDR:
					bin[0] = 0x66u;
					break ;

				case A:
					bin[0] = 0x67u;
					break ;

				default:
					error.p2 = INVAL_SRC;
					break ;
			}
			break ;

		case L:
			switch (param[1])
			{
				case IMM8:
					bin[0] = 0x2Eu;
					bin[1] = (uint8_t)val->value;
					break ;

				case B:
					bin[0] = 0x68u;
					break ;

				case C:
					bin[0] = 0x69u;
					break ;

				case D:
					bin[0] = 0x6Au;
					break ;

				case E:
					bin[0] = 0x6Bu;
					break ;

				case H:
					bin[0] = 0x6Cu;
					break ;

				case L:
					bin[0] = 0x6Du;
					break ;

				case HL_ADDR:
					bin[0] = 0x6Eu;
					break ;

				case A:
					bin[0] = 0x6Fu;
					break ;

				default:
					error.p2 = INVAL_SRC;
					break ;
			}
			break ;

		case HL_ADDR:
			switch (param[1])
			{
				case IMM8:
					bin[0] = 0x36u;
					bin[1] = (uint8_t)val->value;
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

				case A:
					bin[0] = 0x77u;
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

__ldff:
	if (param[0] == A)
	{
		if (param[1] == NONE)
			error.p2 = MISSING_PARAM;
		else if (param[1] != IMM8 && param[1] != ADDR8)
			error.p2 = INVAL_SRC;
		else
		{
			bin[0] = 0xF0u;
			bin[1] = (uint8_t)val->value;
		}
	}
	else if (param[0] == IMM8 || param[0] == ADDR8)
	{
		if (param[1] == NONE)
			error.p2 = MISSING_PARAM;
		else if (param[1] != A)
			error.p2 = INVAL_SRC;
		else
		{
			bin[0] = 0xE0u;
			bin[1] = (uint8_t)val->value;
		}
	}
	else if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else
		error.p1 = INVAL_DST;
	goto __done;

__ldd:
	if (param[0] == A)
	{
		if (param[1] != HL_ADDR)
			error.p2 = INVAL_SRC;
		else
			bin[0] = 0x2Au;
	}
	else if (param[0] == HL_ADDR)
	{
		if (param[1] != A)
			error.p2 = INVAL_SRC;
		else
			bin[0] = 0x22u;
	}
	else
		error.p1 = INVAL_DST;
	goto __done;

__ldhl:
	if (param[0] != SP)
		error.p1 = INVAL_SRC;
	else if (param[1] != IMM8)
		error.p1 = INVAL_DST;
	else
	{
		bin[0] = 0xF8u;
		bin[1] = (uint8_t)val->value;
	}
	goto __done;

__ldi:
	if (param[0] == A)
	{
		if (param[1] != HL_ADDR)
			error.p2 = INVAL_SRC;
		else
			bin[0] = 0x3Au;
	}
	else if (param[0] == HL_ADDR)
	{
		if (param[1] != A)
			error.p2 = INVAL_SRC;
		else
			bin[0] = 0x32u;
	}
	else
		error.p1 = INVAL_DST;
	goto __done;

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
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else
	{
		switch (param[0])
		{
			case BC:
				bin[0] = 0xC1u;
				break ;

			case DE:
				bin[0] = 0xD1u;
				break ;

			case HL:
				bin[0] = 0xE1u;
				break ;

			case AF:
				bin[0] = 0xF1u;
				break ;

			default:
				error.p1 = INVAL_DST;
				break ;
		}
	}
	goto __done;

__push:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else
	{
		switch (param[0])
		{
			case BC:
				bin[0] = 0xC5u;
				break ;

			case DE:
				bin[0] = 0xD5u;
				break ;

			case HL:
				bin[0] = 0xE5u;
				break ;

			case AF:
				bin[0] = 0xF5u;
				break ;

			default:
				error.p1 = INVAL_SRC;
				break ;
		}
	}
	goto __done;

__res:
__reset:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[0] != IMM8 || val->value > 7)
		error.p1 = INVAL_SRC;
	else
	{
		val->value *= 8;
		bin[0] = 0xCBu;
		switch (param[1])
		{
			case NONE:
				error.p2 = MISSING_PARAM;
				break ;

			case A:
				bin[1] = 0x87u + (uint8_t)val->value;
				break ;

			case B:
				bin[1] = 0x80u + (uint8_t)val->value;
				break ;

			case C:
				bin[1] = 0x81u + (uint8_t)val->value;
				break ;

			case D:
				bin[1] = 0x82u + (uint8_t)val->value;
				break ;

			case E:
				bin[1] = 0x83u + (uint8_t)val->value;
				break ;

			case H:
				bin[1] = 0x84u + (uint8_t)val->value;
				break ;

			case L:
				bin[1] = 0x85u + (uint8_t)val->value;
				break ;

			case HL_ADDR:
				bin[1] = 0x86u + (uint8_t)val->value;
				break ;

			default:
				error.p2 = INVAL_DST;
				break ;
		}
	}
	goto __done;

__ret:
	if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else
	{
		switch (param[0])
		{
			case NONE:
				bin[0] = 0xC9u;
				break ;

			case _C_: case C:
				bin[0] = 0xD8u;
				break ;

			case _NC_:
				bin[0] = 0xD0u;
				break ;

			case _Z_:
				bin[0] = 0xC8u;
				break ;

			case _NZ_:
				bin[0] = 0xC0u;
				break ;

			default:
				error.p1 = INVAL_DST;
				break ;
		}
	}
	goto __done;

__retc:
	if (param[0] != NONE)
		error.p1 = TOO_MANY_PARAMS;
	else
		bin[0] = 0xD8u;
	goto __done;

__reti:
	if (param[0] != NONE)
		error.p1 = TOO_MANY_PARAMS;
	else
		bin[0] = 0xD9u;
	goto __done;

__retnc:
	if (param[0] != NONE)
		error.p1 = TOO_MANY_PARAMS;
	else
		bin[0] = 0xD0u;
	goto __done;

__retnz:
	if (param[0] != NONE)
		error.p1 = TOO_MANY_PARAMS;
	else
		bin[0] = 0xC0u;
	goto __done;

__retz:
	if (param[0] != NONE)
		error.p1 = TOO_MANY_PARAMS;
	else
		bin[0] = 0xC8u;
	goto __done;

__rl:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else
	{
		bin[0] = 0xCBu;
		switch (param[0])
		{
			case A:
				bin[1] = 0x17u;
				break ;

			case B:
				bin[1] = 0x10u;
				break ;

			case C:
				bin[1] = 0x11u;
				break ;

			case D:
				bin[1] = 0x12u;
				break ;

			case E:
				bin[1] = 0x13u;
				break ;

			case H:
				bin[1] = 0x14u;
				break ;

			case L:
				bin[1] = 0x15u;
				break ;

			case HL_ADDR:
				bin[1] = 0x16u;
				break ;

			default:
				error.p1 = INVAL_DST;
				break ;
		}
	}
	goto __done;

__rla:
	if (param[0] != NONE)
		error.p1 = TOO_MANY_PARAMS;
	else
		bin[0] = 0x17u;
	goto __done;

__rlc:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else
	{
		bin[0] = 0xCBu;
		switch (param[0])
		{
			case A:
				bin[1] = 0x07u;
				break ;

			case B:
				bin[1] = 0x00u;
				break ;

			case C:
				bin[1] = 0x01u;
				break ;

			case D:
				bin[1] = 0x02u;
				break ;

			case E:
				bin[1] = 0x03u;
				break ;

			case H:
				bin[1] = 0x04u;
				break ;

			case L:
				bin[1] = 0x05u;
				break ;

			case HL_ADDR:
				bin[1] = 0x06u;
				break ;

			default:
				error.p1 = INVAL_DST;
				break ;
		}
	}
	goto __done;

__rlca:
	if (param[0] != NONE)
		error.p1 = TOO_MANY_PARAMS;
	else
		bin[0] = 0x07u;
	goto __done;

__rr:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else
	{
		bin[0] = 0xCBu;
		switch (param[0])
		{
			case A:
				bin[1] = 0x1Fu;
				break ;

			case B:
				bin[1] = 0x18u;
				break ;

			case C:
				bin[1] = 0x19u;
				break ;

			case D:
				bin[1] = 0x1Au;
				break ;

			case E:
				bin[1] = 0x1Bu;
				break ;

			case H:
				bin[1] = 0x1Cu;
				break ;

			case L:
				bin[1] = 0x1Du;
				break ;

			case HL_ADDR:
				bin[1] = 0x1Eu;
				break ;

			default:
				error.p1 = INVAL_DST;
				break ;
		}
	}
	goto __done;

__rra:
	if (param[0] != NONE)
		error.p1 = TOO_MANY_PARAMS;
	else
		bin[0] = 0x1Fu;
	goto __done;

__rrc:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else
	{
		bin[0] = 0xCBu;
		switch (param[0])
		{
			case A:
				bin[1] = 0x0Fu;
				break ;

			case B:
				bin[1] = 0x08u;
				break ;

			case C:
				bin[1] = 0x09u;
				break ;

			case D:
				bin[1] = 0x0Au;
				break ;

			case E:
				bin[1] = 0x0Bu;
				break ;

			case H:
				bin[1] = 0x0Cu;
				break ;

			case L:
				bin[1] = 0x0Du;
				break ;

			case HL_ADDR:
				bin[1] = 0x0Eu;
				break ;

			default:
				error.p1 = INVAL_DST;
				break ;
		}
	}
	goto __done;

__rrca:
	if (param[0] != NONE)
		error.p1 = TOO_MANY_PARAMS;
	else
		bin[0] = 0x0Fu;
	goto __done;

__rst:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else if (param[0] != IMM8)
		error.p1 = INVAL_DST;
	else
	{
		switch (val->value)
		{
			case 0x00u:
			case 0x08u:
			case 0x10u:
			case 0x18u:
			case 0x20u:
			case 0x28u:
			case 0x30u:
			case 0x38u:
				bin[0] = 0xC7 + (uint8_t)val->value;
				param[0] = NONE;
				break ;

			default:
				error.p1 = INVAL_DST;
				break ;
		}
	}
	goto __done;

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
		bin[0] = 0x37u;
	goto __done;

__set:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[0] != IMM8 || val->value > 7)
		error.p1 = INVAL_SRC;
	else
	{
		val->value *= 8;
		bin[0] = 0xCBu;
		switch (param[1])
		{
			case NONE:
				error.p2 = MISSING_PARAM;
				break ;

			case A:
				bin[1] = 0xC7u + (uint8_t)val->value;
				break ;

			case B:
				bin[1] = 0xC0u + (uint8_t)val->value;
				break ;

			case C:
				bin[1] = 0xC1u + (uint8_t)val->value;
				break ;

			case D:
				bin[1] = 0xC2u + (uint8_t)val->value;
				break ;

			case E:
				bin[1] = 0xC3u + (uint8_t)val->value;
				break ;

			case H:
				bin[1] = 0xC4u + (uint8_t)val->value;
				break ;

			case L:
				bin[1] = 0xC5u + (uint8_t)val->value;
				break ;

			case HL_ADDR:
				bin[1] = 0xC6u + (uint8_t)val->value;
				break ;

			default:
				error.p2 = INVAL_DST;
				break ;
		}
	}
	goto __done;

__shl:
__sla:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else
	{
		bin[0] = 0xCBu;
		switch (param[0])
		{
			case A:
				bin[1] = 0x27u;
				break ;

			case B:
				bin[1] = 0x20u;
				break ;

			case C:
				bin[1] = 0x21u;
				break ;

			case D:
				bin[1] = 0x22u;
				break ;

			case E:
				bin[1] = 0x23u;
				break ;

			case H:
				bin[1] = 0x24u;
				break ;

			case L:
				bin[1] = 0x25u;
				break ;

			case HL_ADDR:
				bin[1] = 0x26u;
				break ;

			default:
				error.p1 = INVAL_DST;
				break ;
		}
	}
	goto __done;

__sar:
__sra:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else
	{
		bin[0] = 0xCBu;
		switch (param[0])
		{
			case A:
				bin[1] = 0x2Fu;
				break ;

			case B:
				bin[1] = 0x28u;
				break ;

			case C:
				bin[1] = 0x29u;
				break ;

			case D:
				bin[1] = 0x2Au;
				break ;

			case E:
				bin[1] = 0x2Bu;
				break ;

			case H:
				bin[1] = 0x2Cu;
				break ;

			case L:
				bin[1] = 0x2Du;
				break ;

			case HL_ADDR:
				bin[1] = 0x2Eu;
				break ;

			default:
				error.p1 = INVAL_DST;
				break ;
		}
	}
	goto __done;

__shr:
__srl:
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else
	{
		bin[0] = 0xCBu;
		switch (param[0])
		{
			case A:
				bin[1] = 0x3Fu;
				break ;

			case B:
				bin[1] = 0x38u;
				break ;

			case C:
				bin[1] = 0x39u;
				break ;

			case D:
				bin[1] = 0x3Au;
				break ;

			case E:
				bin[1] = 0x3Bu;
				break ;

			case H:
				bin[1] = 0x3Cu;
				break ;

			case L:
				bin[1] = 0x3Du;
				break ;

			case HL_ADDR:
				bin[1] = 0x3Eu;
				break ;

			default:
				error.p1 = INVAL_DST;
				break ;
		}
	}
	goto __done;

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
	if (param[0] == NONE)
		error.p1 = MISSING_PARAM;
	else if (param[1] != NONE)
		error.p2 = TOO_MANY_PARAMS;
	else
	{
		bin[0] = 0xCBu;
		switch (param[0])
		{
			case A:
				bin[1] = 0x37u;
				break ;

			case B:
				bin[1] = 0x30u;
				break ;

			case C:
				bin[1] = 0x31u;
				break ;

			case D:
				bin[1] = 0x32u;
				break ;

			case E:
				bin[1] = 0x33u;
				break ;

			case H:
				bin[1] = 0x34u;
				break ;

			case L:
				bin[1] = 0x35u;
				break ;

			case HL_ADDR:
				bin[1] = 0x36u;
				break ;

			default:
				error.p1 = INVAL_DST;
				break ;
		}
	}
	goto __done;

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
