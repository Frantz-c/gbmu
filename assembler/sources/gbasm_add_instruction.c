/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_add_instruction.c                          .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/16 22:10:25 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/18 05:09:15 by fcordon     ###    #+. /#+    ###.fr     */
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

ssize_t		instruction_search(instruction_t inst[71], char *tofind)
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
	uint8_t	parent = 0;

	while (is_space(**s)) (*s)++;
	if (is_endl(**s))
	{
		*p1 = NULL;
		return (0);
	}

	*p1 = *s;

	//param1
	if (**s == '(' || **s == '[') {
		parent += **s;
		(*s)++;
		while (is_space(**s)) (*s)++;
	}
	while (is_alnum(**s) || **s == '_' || is_operator(**s)) (*s)++;
	*p1 = strndup(*p1, *s - *p1);
	if (**s == ')' || **s == ']') {
		parent -= **s;
		(*s)++;
	}
	if (parent)
		return (-1);

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
	parent = 0;
	*p2 = *s;
	if (**s == '(' || **s == '[') {
		parent += **s;
		(*s)++;
		while (is_space(**s)) (*s)++;
	}
	while (is_alnum(**s) || **s == '_' || is_operator(**s)) (*s)++;
	*p2 = strndup(*p2, *s - *p2);
	while (is_space(**s)) (*s)++;
	if (**s == ')' || **s == ']') {
		parent -= **s;
		(*s)++;
	}
	if (parent)
		return (-1);

	while (is_space(**s)) (*s)++;
	if (is_endl(**s))
		return (0);
	return (-1);
}

//"truc + bidule * machin"

int		replace_macro(char **param, vector_t *macro)
{
	char		*s = *param;
	char		*start, *end;
	char		*new = NULL;
	uint32_t	newl = 0;
	char		*macro_name;
	int32_t		index;
	uint8_t		parent = 0;

	if (*s == '[' || *s == '(') {
		parent += *s;
		s++;
		while (is_space(*s)) s++;
	}

	// maybe a macro
	while (is_alpha(*s) || *s == '_')
	{
		start = s;
		s++;
		while (is_alnum(*s) || *s == '_') s++;
		if (!is_space(*s) && !is_endl(*s)
				&& (parent == 0 || (new == NULL && *s != '+') || (new && *s != '*') || *s != ']' || *s != ')'))
		{
			goto __unexpected_char;
		}
		macro_name = strndup(start, s - start);
		index = vector_search(macro, macro_name);
		if (index > -1)
		{
			char		*macro_content = VEC_ELEM(macro_t, macro, index)->content;
			uint32_t	macro_argc = VEC_ELEM(macro_t, macro, index)->argc;
			uint32_t	len;

			if (macro_argc != 0)
				goto __macro_with_param;

			if (is_numeric(macro_content, &len))
			{
				macro_content += len;
				while (is_space(macro_content[len])) len++;
				if (macro_length[len] != '\0')
					goto __invalid_macro;
				if (new == NULL)
				{
					newl = (start - *param) + (strlen(macro_content));
					new = malloc(newl + 1);
					strncpy(new, *param, start - *param);
					strcpy(new + (start - *param), macro_content);
				}
				else
				{
					newl += (start - end) + (strlen(macro_content));
					new = realloc(new, newl + 1);
					strncat(new, end, start - end);
					strcat(new, macro_content);
				}
			}
			end = s;
		}
		if (*s == ']' || *s == ')') {
			parent -= *s;
			s++;
			while (is_space(*s)) s++;
		}
		if (*s == '\0')
			break;
	}

	if (new)
	{
		strcat(new, end);
		free(*param);
		*param = new;
	}
	return (0);

__macro_with_param:
__unexpected_char:
	return (-1);
__invalid_macro:
	free(macro_name);
	return (-1);
}

param_t	get_type(char *param1, uint16_t *n)
{
	char		*s = param1;
	int			parent = 0;
	int			num = 0;
	int			error;
	uint32_t	len;
	uint32_t	tmp;

	// addr param
	if (*s == '[' || *s == '(')
	{
		parent = (*s == '[') ? ']' : ')';
		s++;
		
		// (C)
		if (*s == 'c' && s[1] == parent && s[2] == '\0')
			return (FF00_C);
		if ((*s == '0' && s[1] == 'x')
		{
			num = 1;
			s += 2;
		}
		// (ff00+x)
		if (*s == 'f')
		{
			if (s[1] == 'f' && s[2] == '0' && s[3] == '0')
			{
				if ((!num && s[4] == 'h' && s[5] == '+') || (num && s[4] == '+'))
				{
					while (*s != '+') s++;
					s++;
					if (*s == 'c' && s[1] == parent)
						return (FF00_C);
					if (is_numeric(s, &len))
					{
						tmp = atou_all(s, &error)
						if (error)
							return (UNKNOWN);
						if (tmp > 0xff)
							goto __overflow_error;
						*n = tmp;
						return (FF00_IMM8);
					}
				}
			}
		}
		if (num)
			return (UNKNOWN);
		// (rr)
		if (*s == 'a' && s[1] == 'f' && *s == parent)
			return (AF_ADDR);
		if (*s == 'b' && s[1] == 'c' && *s == parent)
			return (BC_ADDR);
		if (*s == 'd' && s[1] == 'e' && *s == parent)
			return (DE_ADDR);
		if (*s == 'h' && s[1] == 'l' && *s == parent)
			return (HL_ADDR);
		if (*s == 's' && s[1] == 'p' && *s == parent)
			return (SP_ADDR);
		// (n) (nn)
		if (is_numeric(s, &len))
		{
			if (s[len] != parent)
				return (UNKNOWN);
			tmp = atou_all(s, &error);
			if (error)
				return (UNKNOWN);
			if (tmp > 0xffff)
				goto __overflow_error;
			if (tmp > 0xff) {
				*n = tmp & 0xffff;
				return (ADDR16);
			}
			return (ADDR8);
		}
		// (HL++?) (HLI) (HL--?) (HLD)
		if (*s == 'h' && s[1] == 'l')
		{
			if (s[2] == '+' && (s[3] == parent || (s[3] == '+' && s[4] == parent)))
				return (HLI);
			if (s[2] == 'i' && s[3] == parent)
				return (HLI);
			if (s[2] == '-' && (s[3] == parent || (s[3] == '-' && s[4] == parent)))
				return (HLD);
			if (s[2] == 'd' && s[3] == parent)
				return (HLD);
		}
	}
	else
	{
		if	(
				(*s == 'a' || *s == 'b' || *s == 'c' || *s == 'd' || *s == 'e')
				&& *s == '\0'
			)
			return (A + (*s - 'a'));
		if	(
				(*s == 'h' || *s == 'l')
				&& *s == '\0'
			)
			return (H + (*s == 'l'));
		if (*s == 'a' && s[1] == 'f' && *s == '\0')
			return (AF);
		if (*s == 'b' && s[1] == 'c' && *s == '\0')
			return (BC);
		if (*s == 'd' && s[1] == 'e' && *s == '\0')
			return (DE);
		if (*s == 'h' && s[1] == 'l' && *s == '\0')
			return (HL);
		if (*s == 's' && s[1] == 'p' && *s == '\0')
			return (SP);
		if (is_numeric(s, &len))
		{
			if (s[len] != '\0')
				return (UNKNOWN);
			tmp = atou_all(s, &error);
			if (error)
				return (UNKNOWN);
			if (tmp > 0xffff)
				goto __overflow_error;
			if (tmp > 0xff) {
				*n = tmp & 0xffff;
				return (IMM16);
			}
			return (IMM8);
		}
	}
	return (SYMBOL);

__overflow_error:
	return (UNKNOWN);
}
/*
"x"		// a,b,c,d,e,f,h,l
"xx"	// af,bc,de,hl,sp
"(c)"
"(ff00h+c)"
"(0xff00+c)"
"(ff00+c)"
"(xx)"	// af,bc,de,hl
"(n)"	// number (unsigned / signed) hexa/decimal/octal/binary : 0111b OU 0b0111, 0777, 12, 0xcb OU 0xCB OU cbh OU CBh OU cbH...
"(nn)"
"n"		// number "
"nn"
"(hl+)"
"(hl++)"
"(hli)"
"(hl-)"
"(hl--)"
"(hld)"
*/
/*
calculs -> addr only
var		-> addr, 16bit op only
*/
char	*add_instruction(char *inst, vector_t *area, vector_t *label, vector_t *macro, char *s, data_t *data)
{
	char		*param1, *param2;
	param_t		p1 = NONE, p2 = NONE;
	uint16_t	val[2] = {0, 0};

	str_to_lower(inst);
	if (set_params(&param1, &param2, &s) == -1)
	{
		if (*s == ',')
			goto __too_many_param;
		goto __unexpected_char;
	}
	printf("inst = \"%s\", param1 = \"%s\", param2 = \"%s\"\n", inst, param1, param2);

	if (param1)
	{
		if (macro && replace_macro(&param1, macro) == -1) //without params only
			goto __error;
		str_to_lower(param1);
		printf("replace_param1 = \"%s\"\n", param1);
		p1 = get_type(param1, val); //default SYMBOL
		if (param2)
		{
			if (macro && replace_macro(&param2, macro) == -1)
				goto __error;
			str_to_lower(param2);
			printf("replace_param2 = \"%s\"\n", param2);
			p2 = get_type(param2, val + 1); //default SYMBOL
		}
	}
/*
	char	*symbol = param2;

	if (p1 == SYMBOL)
		symbol = param1;

	add_bin_instruction(inst, p1, p2, area, symbol, val);
	return (s);
*/
__unexpected_char:
__too_many_param:

}

/*
 *	si le type recu est imm8, c'est peut-etre une imm16.
 *	penser a changer le type si necessaire. (idem pour addr8)
 */

uint32_t	add_bin_instruction(char *inst, param_t param1, param_t param2, vector_t *area, char *symbol, uint16_t val[2])
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
	return (error1 | (error2 << 16));
__add:
__and:
__bit:
__call:
__callc:
__callnc:
__callnz:
__callz:
__ccf:
__cmp:
__cp:
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

}
/:;;*
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
*/
void	add_label(char *name, vector_t *area, vector_t *label, data_t *data)
{
	label_t	new = {name, VEC_ELEM(code_area_t, area, 0)->addr};
	vector_push(label, (void*)&new);
}
