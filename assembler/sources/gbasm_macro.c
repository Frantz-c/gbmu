/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_macro.c                                    .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/11 16:48:47 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/20 21:24:25 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "gbasm_struct.h"
#include "gbasm_macro_func.h"
#include "gbasm_macro.h"
#include "gbasm_error.h"
#include "gbasm_tools.h"

extern char	*define_macro(vector_t *macro, char *s, data_t *data)
{
	char		*name;
	char		*name_start;
	uint32_t	parent = 0;

	while (*s == ' ' || *s == '\t') s++;
	if (*s == '\n')
		goto __arg_expected;

	/*
	 *	get %define's name
	 */
	if (*s != '_' && !is_alpha(*s))
		goto __unexpected_char;
	name = s;
	s++;
	while (is_alnum(*s) || *s == '_') s++;
	if (*s == '(') // if (macro_with_params)
	{
		parent = s - name;
		s++;
		while (is_space(*s)) s++;
		if (*s != ')')
		{
			while (1)
			{
				if (*s != '_' && !is_alpha(*s))
					goto __unexpected_char;
				s++;
				while (is_alnum(*s) || *s == '_') s++;
				while (is_space(*s)) s++;
				if (*s == ')') {
					s++;
					if (!is_space(*s))
						goto __unexpected_char;
					break;
				}
				if (*s != ',')
					goto __unexpected_char;
				s++;
				while (is_space(*s)) s++;
			}
		}
		else if (!is_space(s[1]))
			goto __unexpected_char;
		else
		{
			parent = 0;
			s++;
		}
	}
	else if (!is_space(*s))
		goto __unexpected_char;

	/*
	 *	get %define's content
	 */
	name_start = strndup(name, s - name);
	while (is_space(*s)) s++;
	if (!parent)
		s = add_macro_without_param(name_start, macro, s, data);
	else
		s = add_macro_with_param(name_start, macro, s, data, name_start + parent);
	return (s);

/*
 *	=========ERRORS=========
 */
__arg_expected:
	print_error(data->filename, data->lineno, data->line,
				"argument expected after %define");
	return (s);

__unexpected_char:
	sprintf(data->buf, "unexpected character `%c`", *s);
	print_error(data->filename, data->lineno, data->line, data->buf);
	skip_macro(&s, &data->lineno);
	return (s);
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
	sprintf(data->buf, "unexpected character `%c`", *s);
	print_error(data->filename, data->lineno, data->line, data->buf);
	for (; *s != 0 && *s != '\n'; s++);
	return (s);
}
