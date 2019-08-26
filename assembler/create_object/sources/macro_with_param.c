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
				&&	(!is_alnum(chr[replace_length]) && chr[replace_length] != '_')
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
extern char	*add_macro_with_param(char *name_start, vector_t *macro, char *s, data_t *data, char *name)
{
	uint32_t	cur = 0;
	char		*pos[11] = {NULL};
	char		*content;

	*name = '\0';
	if (vector_search(macro, (void*)&name_start) != -1)
		goto __macro_already_defined;

	name++;
	while (is_space(*name)) name++;
	while (1)
	{
		pos[cur++] = name;
		while (is_alnum(*name) || *name == '_') name++;

		if (*name == ')') {
			*name = '\0';
			break;
		}
		*(name++) = '\0';
		while (*name == ',' || is_space(*name)) name++;
		if (*name == ')') {
			*name = '\0';
			break;
		}
		if (cur == 10)
			goto __too_many_parameters;
	}

	uint32_t	count = cur;
	uint32_t	length;

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
	while (cur--)
	{
		if (string_replace(content, pos[cur], cur + '0') == 0)
		{
			sprintf(data->buf, "unused argument %s", pos[cur]);
			print_warning(data->filename, data->lineno, data->line, data->buf);
		}
	}

	size_t	index = vector_index(macro, &name_start);
	macro_t	new = {name_start, content, count, 1};
	vector_insert(macro, (void*)&new, index);
	return (s);

/*
 *	=========ERRORS=========
 */

__unexpected_char:
	sprintf(data->buf, "(#6) unexpected character `%c`", *s);
	goto __perror_and_exit;
	/*
__empty_argument_x:
	sprintf(data->buf, "argument %u is empty", cur + 1);
	goto __perror_and_exit;
*/
__too_many_parameters:
	sprintf(data->buf, "too many parameters declared in macro `%s`", name_start);
	goto __perror_and_exit;

__macro_already_defined:
	sprintf(data->buf, "macro `%s` already defined", name_start);
__perror_and_exit:
	print_error(data->filename, data->lineno, data->line, data->buf);
	free(name_start);
	skip_macro(&s, &data->lineno);
	return (s);
}
