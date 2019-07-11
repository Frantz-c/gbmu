/*
**					gbasm_macro_with_param.c
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
#include "gbasm_struct.h"
#include "gbasm_macro_func.h"

static int		string_replace(char *content, char *replace, char number)
{
	char			*chr = content;
	const uint32_t	replace_length = strlen(replace);
	int				count = 0;

	chr = content;
	while ((chr = strchr(chr, *replace)) != NULL)
	{
		if	(strncmp(chr, replace, replace_length) == 0
				&& (chr == content || chr[-1] == ' ' || chr[-1] == '\t' || chr[-1] == '\n'
					|| chr[-1] == ',' || chr[-1] == '(' || chr[-1] == ')' || chr[-1] == '[' || chr[-1] == ']')
				&&	(
						chr[replace_length] == ' ' || chr[replace_length] == '\t'
						|| chr[replace_length] == '\n' || chr[replace_length] == ','
						|| chr[replace_length] == '(' || chr[replace_length] == ')'
						|| chr[replace_length] == '[' || chr[replace_length] == ']'
						||	(
								chr[replace_length] == '\\' && !isalnum(chr[replace_length])
							)
					)
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

extern char	*add_macro_with_param(char *name, macro_t *macro, char *s, data_t *data)
{
	char		*name_start = name;
	uint32_t	count = 0;
	uint32_t	cur = 0;
	char		*pos[11] = {NULL};
	char		*content;
	uint32_t	length;
	macro_t		elem = {name, NULL, 0, 1};

	while (*name != '(') name++;
	*name = '\0';
	if (vector_search(macro, (void*)&elem) != -1)
		goto __macro_already_defined;

	do
	{
		if (cur == 10)
			goto __too_many_parameters;

		*name = '\0';
		name++;
		while (*name == ' ' || *name == '\t') name++;

		if (*name == ')' || *name == ',')
			goto __empty_argument_x;

		pos[cur++] = name;
		while (*name != '\0' && *name != ',') name++;
	}
	while (*name != '\0');

	count = cur;

	while (*s == ' ' || *s == '\t') s++;
	length = get_macro_content_length(s);
	content = malloc(length * 2);
	s = copy_macro_content(content, s);

/*	/!\ vérifier les doublons dans les parametres /!\	*/
	char *arg;
	while (cur--)
	{
		arg = pos[cur];

		while (*arg != ' ' && *arg != '\t' && *arg != ',' && *arg != ')' && *arg != '\0') arg++;
		*arg = '\0';
		if (string_replace(content, pos[cur], cur + '0') == 0)
		{
			sprintf(data->buf, "unused argument %s", pos[cur]);
			print_warning(data->filename, data->lineno, data->line, data->buf);
		}
	}

	elem.content = content;
	elem.argc = count;
	size_t	index = vector_index(macro, (void*)&elem);
	vector_insert(macro, (void*)&elem, index);
	return (s);

/*
 *	=========ERRORS=========
 */
__empty_argument_x:
	sprintf(data->buf, "argument %u is empty\n", cur + 1);
	goto __perror_and_exit;

__too_many_parameters:
	sprintf(data->buf, "too many parameters declared in macro `%s`", name_start);
	goto __perror_and_exit;

__macro_already_defined:
	sprintf(data->buf, "macro `%s` already defined", name_start);
__perror_and_exit:
	print_error(data->filename, data->lineno, data->line, data->buf);
__free_and_exit:
	free(name_start);
	skip_macro(&s, &data->lineno);
	return (s);
}
