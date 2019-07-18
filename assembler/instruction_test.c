# include <string.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <ctype.h>
# include "vector.h"
#include "includes/gbasm_struct.h"
#include "sources/gbasm_tools.c"

int		calcul_param(char *param, uint16_t *n, int is_ld)
{
	int			parent = 0;
	uint32_t	len;
	uint32_t	base = 0;
	uint32_t	result = 0;
	uint8_t		neg = 0;
	uint8_t		type;
	char		*first_operator = NULL;

	if (*param == '(') {
		parent = 1;
		param++;
	}

// A + b * c + d
// ^
	if ((type = is_numeric(param, &len)))
	{
		printf("type = %u (arg 1)", type);
		base = atou_type(param, NULL, type);
		param += len;
	}
	while (*param != '+' && *param != '-')
	{
		if (*param == '\0')
			goto __set_n_return;
		param++;
	}
	if (*param == '-')
		neg = 1;
	first_operator = param;
	param++;

// a + B * c + d
//     ^
	if ((type = is_numeric(param, &len)))
	{
		printf("type = %u (arg 2)", type);
		result = atou_type(param, NULL, type);
		param += len;
	}
	else
	{
		if (*param == 'c' && param[1] == '\0') {
			*n = 0;
			return (0);
		}
		else
			goto __error;
	}
	while (*param != '*')
	{
		if (*param == '\0')
			goto __set_n_return;
		param++;
	}
	param++;

// a + b * C + d
//         ^
	puts(">>> 3");
	if ((type = is_numeric(param, &len)))
	{
		result *= atou_type(param, NULL, type);
		param += len;
	}
	else
		goto __error;
	while (*param != '+')
	{
		if (*param == '\0')
			goto __set_n_return;
		param++;
	}
	param++;

// a + b * c + D
//             ^
	puts(">>> 4");
	if ((type = is_numeric(param, &len)))
	{
		result += atou_type(param, NULL, type);
		param += len;
	}
	if (*param != '\0')
		goto __error;


__set_n_return:
	// ld (ff80+x) exception
	if (parent && is_ld && base == 0xff00 && neg == 0)
	{
		base = 0;
		if (result > 0xff)
		{
			// overflow error
		}
		*n = result;
		while (param[-1] != '+') param--;
		*param = '\0';
		return (0);
	}

	result = (neg) ? base - result : base + result;
	if (result > 0xffff)
	{
		// overflow error
	}
	*n = (result & 0xffff);
	*first_operator = '\0';
	return (0);

__error:
	*n = 0;
	fprintf(stderr, "ERROR CALCUL\n");
	return (-1);
}


void	parse_instruction(char *s)
{
	char		*inst, *param[2] = {NULL};
	uint16_t	val[2] = {0};
	uint8_t		parent = 0;
	uint32_t	len, i;
	char		*tmp;
	int			is_ld = 0;

// PARAM 1
	inst = s;
	while (!is_space(*s))
	{
		if (is_endl(*s))
			goto __no_param;
		s++;
	}
	inst = strndup(inst, s - inst);

	if (strcmp(inst, "ld") == 0)
		is_ld = 1;


	while (is_space(*s)) s++;
	tmp = s;
	if (*s == '[' || *s == '(')
		parent = *s;
	
	for (len = 0; *s != ',' && !is_endl(*s); s++)
	{
		if (!is_space(*s)) len++;
	}
	param[0] = malloc(len + 1);
	for (s = tmp, i = 0; *s != ',' && !is_endl(*s); s++)
	{
		if (!is_space(*s)) param[0][i++] = *s;
	}
	param[0][i] = '\0';
	if (parent) {
		param[0][0] = '(';
		param[0][i - 1] = '\0';
	}

	if (is_endl(*s))
		goto __add_inst;
	if (*s == ',') s++;
	parent = 0;

	while (is_space(*s)) s++;
	tmp = s;
	if (*s == '[' || *s == '(')
		parent = *s;
	
	for (len = 0; *s != ',' && !is_endl(*s); s++)
	{
		if (!is_space(*s)) len++;
	}
	param[1] = malloc(len + 1);
	for (s = tmp, i = 0; *s != ',' && !is_endl(*s); s++)
	{
		if (!is_space(*s)) param[1][i++] = *s;
	}
	param[1][i] = '\0';
	if (parent) {
		param[1][0] = '(';
		param[1][i - 1] = '\0';
	}
	
	if (*s == ',') goto __too_many_arguments;



__add_inst:
	if (calcul_param(param[0], val, is_ld) == -1)
		exit(1);
	printf("%s %s(%u) %s(%u)\n", inst, param[0], val[0], param[1], val[1]);
	return;

__no_param:
	inst = strndup(inst, s - inst);
	goto __add_inst;

__too_many_arguments:
	puts("TOO MANY ARGUMENTS\n");
}

int		main(int argc, char *argv[])
{
	if (argc != 2)
		return (1);

	parse_instruction(argv[1]);
}
