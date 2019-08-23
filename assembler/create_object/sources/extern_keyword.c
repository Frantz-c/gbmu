#include "std_includes.h"
#include "struct.h"
#include "tools.h"
#include "error.h"
#include "keywords.h"

extern char	*set_extern_symbol(vector_t *symbol, arguments_t args[4], data_t *data)
{
	char *name = NULL;

	if (args->value == NULL)
		goto __too_few_arguments;
	if (args[1].value != NULL)
		goto __too_many_arguments;
	if ((args->type & STRING_TYPE))
	{
		if ((args->type & ID_STRING_TYPE) == 0)
			goto __not_well_formated_string;
		name = strdup((char *)args->value);
	}
	else
		goto __wrong_type;

	symbol_t	new = {name, 0, data->lineno, strdup(data->filename)};
	vector_push(symbol, (void*)&new);
	return;
	
__print_error:
	print_error(data->filename, data->lineno, data->line, data->buf);
	return;
}
