/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   macro.c                                          .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/11 16:48:47 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/29 18:02:37 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "struct.h"
#include "macro_func.h"
#include "macro.h"
#include "error.h"
#include "tools.h"

extern char	*define_macro(vector_t *macro, char *s, data_t *data)
{
	char		*name;
	char		*param[11] = {NULL};
	char		*macro_name = NULL;
	uint8_t		i = 0;

	while (is_space(*s)) s++;
	if (is_endl(*s))
		goto __arg_expected;

	/*
	 *	get %define's name
	 */
	if (*s != '_' && !is_alpha(*s))
		goto __identifier_syntax_error;
	macro_name = s++;
	while (is_alnum(*s) || *s == '_') s++;
	macro_name = strndup(macro_name, s - macro_name);

	if (*s == '(') // if (macro_with_params)
	{
		s++;
		while (is_space(*s)) s++;
		if (*s == ')')
			goto __error_no_param_but_parents;
		else // (*s != ')')
		{
			while (1)
			{
				if (*s != '_' && !is_alpha(*s))
					goto __identifier_syntax_error;
				name = s++;
				while (is_alnum(*s) || *s == '_') s++;
				param[i++] = strndup(name, s - name);

				while (is_space(*s)) s++;

				if (*s == ')')
				{
					s++;
					if (*s == '\\')
					{
						s++;
						while (is_space(*s)) s++;
						if (*s != '\n')
							goto __unexpected_char;
						s++;
					}
					else if (!is_space(*s))
						goto __unexpected_char;
					break;
				}
				if (*s != ',')
					goto __unexpected_char;
				s++;
				while (is_space(*s)) s++;
				if (i == 10)
					goto __too_many_params;
			}
		}

	}
	else if (!is_space(*s))
		goto __unexpected_char;

	/*
	 *	get %define's content
	 */
	while (is_space(*s)) s++;
	if (!i)
		s = add_macro_without_param(macro_name, macro, s, data);
	else
	{
		s = add_macro_with_param(macro_name, macro, s, data, param, i);
		for (; i < 0xffu; i--)
			free(param[i]);
	}
	return (s);

/*
 *	=========ERRORS=========
 */
	{
		register const char	*error_msg;

	__too_many_params:
		error_msg = "number of macro parameters must be lower or equal than 10";
		goto __print_error;
	__error_no_param_but_parents:
		error_msg = "() syntax forbidden if no parameters given";
		goto __print_error;
	__arg_expected:
		error_msg = "argument expected after %define";
		goto __print_error;
	__identifier_syntax_error:
		sprintf(data->buf, "bad character `%c`, syntax = [a-zA-Z_][a-zA-Z0-9_]*", *s);
		goto __print_error_fmt;
	__unexpected_char:
		sprintf(data->buf, "unexpected character `%c`", *s);
	__print_error_fmt:
		error_msg = (const char *)data->buf;
	__print_error:
		print_error(data->filename, data->lineno, data->line, error_msg);
		skip_macro(&s, &data->lineno);
		if (macro_name) free(macro_name);
		for (uint32_t j = 0; param[j]; j++)
			free(param[j]);
		return (s);
	}
}

extern char	*undef_macro(vector_t *macro, char *s, data_t *data)
{
	char	*name;

	while (*s == ' ' || *s == '\t') s++;
	name = s;

	if ((*s >= 'A' && *s <= 'Z') || (*s >= 'a' && *s <= 'z') || *s == '_')
	{
		while
		(
			(*s >= '0' && *s <= '9') ||
			(*s >= 'A' && *s <= 'Z') ||
			(*s >= 'a' && *s <= 'z') ||
			*s == '_'
		) s++;
	}

	if (*s != ' ' && *s != '\t' && *s != '\n')
		goto __unexpected_char;

	name = strndup(name, s - name);
	ssize_t index;

	if ((index = vector_search(macro, &name)) != -1)
		vector_delete(macro, (size_t)index);

	while (*s == ' ' && *s == '\t') s++;
	if (*s != '\n')
		goto __unexpected_char;
	return (s);

/*
 *	=========ERRORS=========
 */
__unexpected_char:
	sprintf(data->buf, "(#7) unexpected character `%c`", *s);
	print_error(data->filename, data->lineno, data->line, data->buf);
	for (; *s != 0 && *s != '\n'; s++);
	return (s);
}
