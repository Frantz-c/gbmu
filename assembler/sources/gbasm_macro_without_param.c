/*
**					gbasm_macro_without_param.c
**		``````````````````````````````````````````````````
**
**		add macro without parameters in defines_t struct
**		defines_t struct order :
**			1) ascii order of the first char
**				(in a linked list array -> t_defines *[53])
**			2) length order
**				(in a linked list)
**			3) ascii order
**				(in the same linked list)
**
**
**			char	*add_macro_without_param
**			(
**				char		*name,
**				defines_t	**def,
**				char		*s,
**				error_t		*err
**			);
**
**			name	-> file contents from macro's name
**			def		-> macro linked list (corresponding to the first char)
**			s		-> file contents (after macro's name)
**			err		-> error struct
*/


#include "../includes/std_includes.h"
#include "gbasm_struct.h"
#include "gbasm_macro_func.h"
#include "gbasm_error.h"

extern char	*add_macro_without_param(char *name, vector_t *macro, char *s, data_t *data)
{
	char	*content;

	while (*s == ' ' || *s == '\t') s++;
	if (*s == '\0' || *s == '\n')
		goto __no_param;

	content = malloc(get_macro_content_length(s) * 2);
	s = copy_macro_content(content, s, &data->lineno);
	if (*s != '\0' && *s != '\n')
		goto __unexpected_char;

	size_t	index = vector_index(macro, &name);
	macro_t	new = {name, content, 0, 1};
	vector_insert(macro, (void*)&new, index);
	return (s);

__unexpected_char:
	sprintf(data->buf, "unexpected character `%c`", *s);
	goto __perror;
__no_param:
	sprintf(data->buf, "missing value macro `%s`", name);
__perror:
	print_error(data->filename, data->lineno, data->line, data->buf);
	skip_macro(&s, &data->lineno);
	free(name);
	return (s);
}
