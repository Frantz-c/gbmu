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

static int		param_replace(char *content, char *replace, char number)
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

static char		*macro_replace(char *s, vector_t *macro)
{
	char		*new = NULL;
	uint32_t	length = 0;
	char		*start;
	char		*name;
	char		c;
	char		*param[10];
	uint8_t		n_params;
	uint8_t		replacement = 1;
	char		*string_start = s;
	macro_t		*m = NULL;

	while (1)
	{
		if (replacement)
		{
			replacement--;
			start = s;
		}

		while (*s && !is_alpha(*s) && *s != '_')
			s++;
		if (!*s)
			break;

		name = s;
		while (is_alnum(*s) || *s == '_')
			s++;
		c = *s;
		*s = '\0';
		if ((m = get_macro(macro, name)) != NULL)
		{
			uint32_t	content_length;

			replacement++;
			*s = c;
			if (*s == '(')
			{
				s++;
				if ((n_params = get_params(&s, param)) == 0xffu)
					goto __error_macro_params;
				s++; // *s = ')'...
				
				if (n_params != m->argc)
					goto __error_n_params;

				char		*new_content = replace_content(m, param);
				content_length = strlen(new_content);
				new = realloc(new, length + content_length + (name - start) + 1);
				strncpy(new + length, start, (name - start));
				length += (name - start);
				strncpy(new + length, new_content, content_length);
				free(new_content);
			}
			else
			{
				content_length = strlen(m->content);
				new = realloc(new, length + content_length + (name - start) + 1);
				strncpy(new + length, start, (name - start));
				length += (name - start);
				strncpy(new + length, m->content, content_length);
			}
			new[length += content_length] = '\0';
		}
		else
			*s = c;
	}
	if (start != s)
	{
		new = realloc(new, length + (s - start) + 1);
		strncpy(new + length, start, s - start);
		new[length += (s - start)] = '\0';
		free(string_start);
	}

	return (new ? new : string_start);

__error_macro_params:
	fprintf(stderr, "macro params error\n");
	goto __return_error;
__error_n_params:
	fprintf(stderr, "n param error\n");

__return_error:
	if (new) free(new);
	free(string_start);
	return (NULL);
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
		if (param_replace(content, param[i], i + '0') == 0)
		{
			sprintf(data->buf, "unused argument %s", param[i]);
			print_warning(data->filename, data->lineno, data->line, data->buf);
		}
	}
	if ((content = macro_replace(content, macro)) == NULL)
		goto __free_and_skip_macro;

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
__macro_already_defined:
	sprintf(data->buf, "macro `%s` already defined", name);
__perror_and_exit:
	print_error(data->filename, data->lineno, data->line, data->buf);
__free_and_skip_macro:
	free(name);
	skip_macro(&s, &data->lineno);
	return (s);
}
