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

ssize_t		instruction_search(instructions_t inst[53], char *tofind)
{
	uint32_t	left, right, middle;
	int			side;

	left = 0;
	right = 53;
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


char	*add_instruction(char *name, vector_t *area, vector_t *macro, vector_t *label, char *s, data_t *data)
{
	static const instructions_t	inst[53] = {
		{"adc", &&__adc}, {"add", &&__add}, {"and", &&__and}, {"bit", &&__bit},
		{"call", &&__call}, {"callc", &&__callc}, {"callnc", &&__callnc}, {"callnz", &&__callnz}
		{"callz", &&__callz}, {"ccf", &&__ccf}, {"cmp", &&__cmp}, {"cp", &&__cp},
		{"cpl", &&__cpl}, {"daa", &&__daa}, {"dec", &&__dec}, {"di", &&__di},
		{"ei", &&__ei}, {"halt", &&__halt}, {"inc", &&__inc}, {"jp", &&__jp},
		{"jpc", &&__jpc}, {"jpnc", &&__jpnc}, {"jpnz", &&__jpnz}, {"jpz", &&__jpz},
		{"jr", &&__jr}, {"jrc", &&__jrc}, {"jrnc", &&__jrnc}, {"jrnz", &&__jrnz},
		{"jrz", &&__jrz}, {"ld", &&__ld}, {"ldd", &&__ldd}, {"ldhl", &&__ldhl},
		{"ldi", &&__ldi}, {"mov", &&__mov}, {"nop", &&__nop}, {"not", &&__not},
		{"or", &&__or}, {"pop", &&__pop}, {"push", &&__push}, {"res", &&__res},
		{"reset", &&__reset}, {"ret", &&__ret}, {"retc", &&__retc}, {"reti", &&__reti},
		{"retnc", &&__retnc}, {"retnz", &&__retnz}, {"retz", &&__retz}, {"rl", &&__rl},
		{"rla", &&__rla}, {"rlc", &&__rlc}, {"rlca", &&__rlca}, {"rr", &&__rr},
		{"rra", &&__rra}, {"rrc", &&__rrc}, {"rrca", &&__rrca}, {"rst", &&__rst},
		{"sar", &&__sar}, {"sbb", &&__sbb}, {"sbc", &&__sbc}, {"scf", &&__scf},
		{"set", &&__set}, {"shl", &&__shl}, {"shr", &&__shr}, {"sla", &&__sla},
		{"sra", &&__sra}, {"srl", &&__srl}, {"stop", &&__stop}, {"sub", &&__sub},
		{"swap", &&__swap}, {"testb", &&__testb}, {"xor", &&__xor}
	};
	uint32_t	index;

__start:
	index = instruction_search(inst, name);
	if (index == 0xffffffffu)
	{
		// verifier s'il y a une parenthese (macro avec param) & récupérer le nom des parametres
		// verifier les caracteres interdits
		uint32_t	macro_index;
		char		*param[10];
		uint32_t	nparam = 0;
		if ((macro_index = (uint32_t)vector_search(macro, (void*)&name)) != 0xffffffffu)
		{
			free(name);
			name = strdup(VEC_ELEM(macro_t, macro, macro_index)->content);
			goto __start;
		}
		goto __unknown_instruction;
	}
	goto *(inst[index].addr);

__adc:

__unknown_instruction:
	while (!is_endl(*s)) s++;
	return (s);
}

void	add_label(char *name, vector_t *area, vector_t *label, data_t *data)
{
	label_t	new = {name, VEC_ELEM(code_area_t, area, 0)->addr};
	vector_push(label, (void*)&new);
}
