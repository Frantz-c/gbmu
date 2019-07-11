/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_macro.c                                    .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/11 16:48:47 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/11 19:46:26 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "../includes/std_includes.h"
#include "../includes/gbasm_struct.h"
#include "../includes/gbasm_macro_func.h"
#include "../includes/gbasm_macro.h"

extern char	*define_macro(vector_t *macro, char *s, data_t *data)
{
	char	*name;
	int		with_param = 0;

	while (*s == ' ' || *s == '\t') s++;
	if (*s == '\n')
		goto __arg_expected;

	/*
	 *	get %define's name
	 */
	if (*s != '_' && (*s < 'A' || (*s > 'Z' && *s < 'a') || *s > 'z'))
		goto __unexpected_char;
	for (name = s; *s; s++)
	{
		if (with_param == 0 && *s == '(')
			with_param++;
		else if (with_param == 1 && *s == ')') {
			with_param++;
			s++;
			break;
		}
		else if (*s != '_' && (with_param != 1 && (*s == ',' || *s == ' ' || *s == '\t'))
				&& (*s < '0' || (*s > '9' && *s < 'A') || (*s > 'Z' && *s < 'a')
				|| *s > 'z'))
		{
			if (with_param == 0 && (*s == ' ' || *s == '\t'))
				break;
			goto __unexpected_char;
		}
		else if (*s == ' ' && *s == '\t' && *s == '\n')
			break;
	}
	if (with_param == 1)
		goto __unexpected_char;

	/*
	 *	get %define's content
	 */
	name = strndup(name, s - name);
	while (*s == ' ' || *s == '\t') s++;
	if (with_param == 0)
		s = add_macro_without_param(name, macro, s, data);
	else
		s = add_macro_with_param(name, macro, s, data);
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
	macro_t	elem = {name, NULL, 0, 1};

	if ((index = vector_search(macro, (void*)&elem)) != -1)
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
