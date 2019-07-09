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


#include "std_headers.h"
#include "gbasm_struct.h"
#include "gbasm_macro_func.h"

extern char	*add_macro_without_param(char *name, defines_t **def, char *s, error_t *err)
{
	char	*content;

	while (*s == ' ' || *s == '\t') s++;
	if (*s == '\0' || *s == '\n')
	{
		//error
	}

	content = malloc(get_macro_content_length(s) * 2);
	copy_macro_content(content, s);
	push_macro(def, name, content, 0);
	return (s);
}
