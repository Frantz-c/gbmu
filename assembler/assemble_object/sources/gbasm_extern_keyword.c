#include "std_includes.h"
#include "gbasm_struct.h"
#include "gbasm_tools.h"
#include "gbasm_error.h"
#include "gbasm_keywords.h"

extern char	*set_extern_symbol(vector_t *symbol, char *s, data_t *data)
{
	char *name = NULL;

	while (is_space(*s)) s++;
	if (is_endl(*s))
		goto __no_arg;
	
	if (!is_alpha(*s) && *s != '_')
		goto __unexpected_char;
	name = s;
	while (is_alnum(*s) || *s == '_') s++;
	name = strndup(name, s - name);
	while (is_space(*s)) s++;
	if (!is_endl(*s))
	{
		free(name);
		goto __unexpected_char;
	}
	symbol_t	new = {name, 0, data->lineno, strdup(data->filename)};
	vector_push(symbol, (void*)&new);
	return (s);
	
__no_arg:
	print_error(data->filename, data->lineno, data->line, "missing symbol name after .extern keyword");
	return (s);
__unexpected_char:
	sprintf(data->buf, "(#4) unexpected character `%c`", *s);
__print_error:
	print_error(data->filename, data->lineno, data->line, data->buf);
	while (!is_endl(*s)) s++;
	return (s);
}
