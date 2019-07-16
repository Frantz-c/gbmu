/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_add_instruction.c                          .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/16 22:10:25 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/16 23:26:11 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "gbasm_struct.h"
#include "gbasm_tools.h"
#include "gbasm_error.h"

typedef struct	instruction_s
{
	const char			*name;
	const void *const	addr;
}
instruction_t;

ssize_t		instruction_search(instructions_t inst[71], char *tofind)
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

int		set_params(char **p1, char **p2, char **s)
{
	while (is_space(**s)) (*s)++;
	if (is_endl(**s))
	{
		*p1 = NULL;
		return (0);
	}

	*p1 = *s;

	//param1
	while (is_alnum(**s) || **s == '_') (*s)++;
	*p1 = strndup(*p1, *s - *p1);

	while (is_space(**s)) (*s)++;
	if (is_endl(**s))
	{
		*p2 = NULL;
		return (0);
	}
	if (**s != ',')
		return (-1);
	while (is_space(**s)) (*s)++;

	//param2
	while (is_alnum(**s) || **s == '_') (*s)++;
	*p2 = strndup(*p2, *s - *p2);

	while (is_space(**s)) (*s)++;
	if (is_endl(**s))
		return (0);
	return (-1);
}

// C_FF00 = (C)
typedef enum	param_e
{
	NONE,A,B,C,D,E,F,H,L,AF,BC,DE,HL,SP,HL_ADDR,BC_ADDR,DE_ADDR,AF_ADDR,C_FF00,IMM8,ADDR8,SYMBOL8,IMM16,ADDR16,SYMBOL
}
param_t;

char	*add_instruction_with_macro_replace(char *inst, vector_t *area, vector_t *label, char *s, data_t *data)
{
	char	*param1, *param2;
	param_t	p1 = NONE, p2 = NONE;

	str_to_lower(inst);
	if (set_params(&param1, &param2, &s) == -1)
	{
		if (*s == ',')
			goto __too_many_param;
		goto __unexpected_char;
	}

	if (param1)
	{
		if	(
			   (p1 = get_r(param1)) || (p1 = get_rr(param1)) || (p1 = get_rr_addr(param1))
			|| (p1 = get_imm(param1)) || (p1 = get_addr(param1)) || (p1 = get_c_ff00(param1))
			|| (p1 = get_macro(param1))
			)
		{
			p1 = SYMBOL;
		}
		if (param2)
		{
			if ((p2 = get_r(param1)) || (p2 = get_rr(param1)) || (p2 = get_rr_addr(param1))
					|| (p2 = get_imm(param1)) || (p2 = get_addr(param1)) || (p2 = get_c_ff00(param1)))
			{
				p2 = SYMBOL;
			}
		}
	}

	char	*symbol = param2;

	if (p1 == SYMBOL)
		symbol = param1;

	add_bin_instruction(inst, p1, p2, area, symbol);
	return (s);

__unexpected_char:

}

uint32_t	add_instruction_without_macro_replace(char *inst, vector_t *area, vector_t *label, char **s, data_t *data)
{
	
}

/*
 *	si le type recu est imm8, c'est peut-etre une imm16.
 *	penser a changer le type si necessaire. (idem pour addr8)
 */

uint32_t	add_bin_instruction(char *inst, param_t param1, param_t param2, vector_t *area, char *symbol)
{
	static const instructions_t	inst[71] = {
		{"adc", &&__adc},		{"add", &&__add},		{"and", &&__and},		{"bit", &&__bit},
		{"call", &&__call},		{"callc", &&__callc},	{"callnc", &&__callnc}, {"callnz", &&__callnz}
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
	uint32_t	index;
	uint8_t		arg1[3];
	uint8_t		arg2[3];
	uint32_t	error1 = 0;
	uint32_t	error2 = 0;

	index = instruction_search(inst, name);
	if (index == 0xffffffffu)
		return (UNKNOWN_INST);
	
//	new_instruction( VEC_ELEM(code_area_t, area, data->cur_area) );
	goto *(inst[index].addr);

__adc:
	while (is_space(*s)) s++;
	// on zape le premier si c'est un A (adc A, X  ==  adc X)
	uint32_t	reg = 0;
	uint32_t	len = 0;
	
	if ((reg = get_8bit_register(s)) != 0)
	{
		s++;
		while (is_space(*s)) s++;
		if (!is_alnum(*s)
	}
	if ((reg = get_8bit_register(s)) == 0 && (reg = get_addr_register(s)) == 0 && is_numeric(s, &len) == 0)
		error2 |= PARAM_NOT_R_N_HLADDR;
	return (error1 | (error2 << 16));

}

char	*add_instruction(char *name, vector_t *area, vector_t *macro, vector_t *label, char *s, data_t *data)
{
	static const instructions_t	inst[71] = {
		{"adc", &&__adc},		{"add", &&__add},		{"and", &&__and},		{"bit", &&__bit},
		{"call", &&__call},		{"callc", &&__callc},	{"callnc", &&__callnc}, {"callnz", &&__callnz}
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
	uint32_t	index;
	uint8_t		is_macro = 0;
	char		*macro_base_ptr;	// ptr sur le debut de la macro pour la libérer à la fin

__start:
	index = instruction_search(inst, name);
	if (index == 0xffffffffu)
	{
		if (is_macro) {
			name = get_keyword(name);
			free(macro_base_ptr);
			goto __unknown_instruction;
		}
		uint32_t	macro_index;
		char		*param[10];
		uint32_t	nparam = (s[-1] == '(');

		if ((macro_index = (uint32_t)vector_search(macro, (void*)&name)) != 0xffffffffu)
		{
			uint8_t		has_param = (VEC_ELEM(macro_t, macro, macro_index)->argc != 0);

			is_macro = 1;
			free(name);
			name = NULL;

			if (nparam != has_param)
				goto __unknown_instruction;
			if (nparam)
			{
				nparam = get_called_macro_params(&s, param);
				name = set_macro_content_with_param(param, nparam, VEC_ELEM(macro_t, macro, macro_index)->content);
				macro_base_ptr = name;
			}
			else
			{
				name = strdup(VEC_ELEM(macro_t, macro, macro_index)->content);
			}
			goto __start;
		}
		goto __unknown_instruction;
	}
	if (!is_macro)
	{
		free(name);
		name = s;
	}

	while (is_space(*name)) name++;
	new_instruction( VEC_ELEM(code_area_t, area, data->cur_area) );
	goto *(inst[index].addr);

__adc:
	if (is_8bits_register(name))


__end:
	if (is_macro)
	{
		while (!is_endl(*name)) name++;
		if (*name == '\n')
		{
			name++;
			while (is_space(*name)) name++;
			if (!is_endl(*name))
				goto __unexpected_char;
		}
	}
	free(name);


__unknown_instruction:
	sprintf(data->buf, "unknown instruction `%s`", name);
	goto __print_error;
__unexpected_char:
	sprintf(data->buf, "unexpected character `%c`", *name);
	if (macro_base_ptr)
		free(macro_base_ptr);
__print_Error:
	print_error(data->filename, data->lineno, data->line, data->buf);
	while (!is_endl(*s)) s++;
	return (s);
}

void	add_label(char *name, vector_t *area, vector_t *label, data_t *data)
{
	label_t	new = {name, VEC_ELEM(code_area_t, area, 0)->addr};
	vector_push(label, (void*)&new);
}
