#include "std_includes.h"
#include "gbasm_struct.h"
#include "gbasm_tools.h"
#include "gbasm_error.h"
/*
typedef struct	instruction_s
{
	const char			*name;
	const void *const	addr;
}
instruction_t;

static const instructions_t	inst[] = {
	{"adc", &&__adc},
	{"add", &&__add},
	{"and", &&__and},
	{"bit", &&__bit},
	{"call", &&__call},
	{"callc", &&__callc},
	{"callnc", &&__callnc},
	{"callnz", &&__callnz}
	{"callz", &&__callz},
	{"ccf", &&__ccf},
	{"cmp", &&__cmp},
	{"cp", &&__cp},
	{"cpl", &&__cpl},
	{"daa", &&__daa},
	{"dec", &&__dec},
	{"di", &&__di},
	{"ei", &&__ei},
	{"halt", &&__halt},
	{"inc", &&__inc},
	{"jp", &&__jp},
	{"jpc", &&__jpc},
	{"jpnc", &&__jpnc},
	{"jpnz", &&__jpnz},
	{"jpz", &&__jpz},
	{"jr", &&__jr},
	{"jrc", &&__jrc},
	{"jrnc", &&__jrnc},
	{"jrnz", &&__jrnz},
	{"jrz", &&__jrz},
	{"ld", &&__ld},
	{"ldd", &&__ldd},
	{"ldhl", &&__ldhl},
	{"ldi", &&__ldi},
	{"mov", &&__mov},
	{"nop", &&__nop},
	{"not", &&__not},
	{"or", &&__or},
	{"pop", &&__pop},
	{"push", &&__push},
	{"res", &&__res},
	{"reset", &&__reset},
	{"ret", &&__ret},
	{"retc", &&__retc},
	{"reti", &&__reti},
	{"retnc", &&__retnc},
	{"retnz", &&__retnz},
	{"retz", &&__retz},
	{"rl", &&__rl},
	{"rla", &&__rla},
	{"rlc", &&__rlc},
	{"rlca", &&__rlca},
	{"rr", &&__rr},
	{"rra", &&__rra},
	{"rrc", &&__rrc},
	{"rrca", &&__rrca},
	{"rst", &&__rst},
	{"sar", &&__sar},
	{"sbb", &&__sbb},
	{"sbc", &&__sbc},
	{"scf", &&__scf},
	{"set", &&__set},
	{"shl", &&__shl},
	{"shr", &&__shr},
	{"sla", &&__sla},
	{"sra", &&__sra},
	{"srl", &&__srl},
	{"stop", &&__stop},
	{"sub", &&__sub},
	{"swap", &&__swap},
	{"testb", &&__testb},
	{"xor", &&__xor},
	{NULL, NULL}
};
*/

char	*add_instruction(char *name, vector_t *area, vector_t *macro, char *s, data_t *data)
{
	while (!is_endl(*s)) s++;
	return (s);
}

void	add_label(char *name, vector_t *area, vector_t *label, data_t *data)
{
	return;
}
