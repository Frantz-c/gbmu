/*
**					macro_with_param.c
**		``````````````````````````````````````````````````
**
**		add macro with parameters in defines_t struct
**		defines_t struct order :
**			1) ascii order of the first char
**				(in a linked list array -> t_defines *[53])
**			2) length order
**				(in a linked list)
**			3) ascii order
**				(in the same linked list)
**
**
**			char	*add_macro_with_param
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
#include "struct.h"
#include "macro_func.h"
#include "error.h"
#include "tools.h"

static int		string_replace(char *content, char *replace, char number)
{
	char			*chr = content;
	const uint32_t	replace_length = strlen(replace);
	int				count = 0;

	chr = content;
	while ((chr = strchr(chr, *replace)) != NULL)
	{
		if	(strncmp(chr, replace, replace_length) == 0
				&& replace_length <= strlen(chr)
				&& (chr == content || (!is_alnum(chr[-1]) && chr[-1] != '_'))
				&& (!is_alnum(chr[replace_length]) && chr[replace_length] != '_')
			)
		{
			if (replace_length > 2)
				memmove(chr + 2, chr + replace_length, strlen(chr + replace_length) + 1);
			else if (replace_length == 1)
				memmove(chr + 1, chr, strlen(chr) + 1);
			chr[0] = '#';
			chr[1] = number;
			chr += 2;
			count++;
		}
		else
			chr++;
	}
	return (count);
}

// name_start = "macro(x,y) toto_x_y", name = "(x,y) toto_x_y", s = " toto_x_y"
extern char	*add_macro_with_param(char *name, vector_t *macro, char *s, data_t *data, char *param[], uint8_t argc)
{
	char		*content;
	uint32_t	length;

	if (vector_search(macro, (void*)&name) != -1)
		goto __macro_already_defined;

	while (is_space(*s)) s++;

	length = get_macro_content_length(s);
	if (length & 0x80000000u)
	{
		s += (length & 0x7fffffffu);
		goto __unexpected_char;
	}
	content = malloc(length * 2);
	s = copy_macro_content(content, s, &data->lineno);

/*	/!\ vérifier les doublons dans les parametres /!\	*/
	for (uint8_t i = 0; i < argc; i++)
	{
		if (string_replace(content, param[i], i + '0') == 0)
		{
			sprintf(data->buf, "unused argument %s", param[i]);
			print_warning(data->filename, data->lineno, data->line, data->buf);
		}
	}

	size_t	index = vector_index(macro, (void*)&name);
	macro_t	new = {name, content, argc, 1};
	vector_insert(macro, (void*)&new, index);
	return (s);

/*
 *	=========ERRORS=========
 */

__unexpected_char:
	sprintf(data->buf, "unexpected character `%c`", *s);
	goto __perror_and_exit;
__too_many_parameters:
	sprintf(data->buf, "too many parameters declared in macro `%s`", name);
	goto __perror_and_exit;
__macro_already_defined:
	sprintf(data->buf, "macro `%s` already defined", name);
__perror_and_exit:
	print_error(data->filename, data->lineno, data->line, data->buf);
	free(name);
	skip_macro(&s, &data->lineno);
	return (s);
}
