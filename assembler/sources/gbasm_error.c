/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_error.c                                    .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/12 12:47:39 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/12 13:19:29 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"

extern void			print_warning(char *filename, uint32_t lineno, char *line, char *error)
{
	char	*p = line;

	g_error++;
	while (*p && *p != '\n') p++;
	if (p - line > 70)
		p = line + 70;

	fprintf
	(
		stderr,
		"\e[1;33m[WARNING]\e[0m \e[1min file \e[0;33m\"%s\"\e[0m\e[1m:\t"
		"line %u: \e[0;33m%s\e[0m\n\t"
		"\e[1;35m> \e[0m%.*s",
		filename,
		lineno, error,
		(int)(p - line), line
	);
}

/*
 *	format d'erreur:
 *
 *		in file "myfile.gs":	l-231: Bad token '$'
 *			> %include "file$"
 */
extern void			print_error(char *filename, uint32_t lineno, char *line, char *error)
{
	char	*p = line;

	g_error++;
	while (*p && *p != '\n') p++;
	if (p - line > 70)
		p = line + 70;

	fprintf
	(
		stderr,
		"\e[1;31m[ERROR]\e[0m \e[1min file \e[0;33m\"%s\"\e[0m\e[1m:\t"
		"line %u: \e[0;31m%s\e[0m\n\t"
		"\e[1;31m> \e[0m%.*s",
		filename,
		lineno, error,
		(int)(p - line), line
	);
}
