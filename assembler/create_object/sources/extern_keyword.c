#include "std_includes.h"
#include "struct.h"
#include "tools.h"
#include "error.h"
#include "keywords.h"

extern void	set_extern_symbol(vector_t *symbol, arguments_t args[4], data_t *data)
{
	char *name = NULL;

	if (args[0].value == NULL)
		goto __too_few_arguments;
	if (args[1].value != NULL)
		goto __too_many_arguments;
	if ((args->type & STRING_TYPE))
	{
		if ((args->type & ID_STRING_TYPE) == 0)
			goto __not_well_formated_arg1;
		name = strdup((char *)args->value);
	}
	else
		goto __wrong_type;

	symbol_t	new = {name, 0, data->lineno, strdup(data->filename)};
	vector_push(symbol, (void*)&new);
	return;
	

/* ||||||||||||||||||||||||||||||||||||||||||*\
** ================ errors ==================**
\* ||||||||||||||||||||||||||||||||||||||||||*/
	register const char	*error_msg;
__not_well_formated_arg1:
	error_msg = "argument 1 format must be [a-zA-Z_][a-zA-Z0-9_]*";
	goto __print_error;
__wrong_type:
	error_msg = "argument 1 must be a string: .extern identifier";
	goto __print_error;
__too_few_arguments:
	error_msg = "too few arguments: .extern identifier";
	goto __print_error;
__too_many_arguments:
	error_msg = "too many arguments: .extern identifier";
	goto __print_error;
__print_error:
	print_error(data->filename, data->lineno, data->line, error_msg);
}
