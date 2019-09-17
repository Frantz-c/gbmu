/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   error.c                                          .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/12 12:47:39 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/17 12:03:29 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "error.h"

#define MAX_ERROR	20

extern void			print_warning(char *filename, uint32_t lineno, char *line, const char *error)
{
	char	*p = line;
	uint32_t	len;

	g_warning++;
	while (!is_endl(*p)) p++;
	len = p - line;
	if (len > 70)
	{
		p = line + (len - 70);
		len = 70;
	}

	fprintf
	(
		stderr,
		"\e[1;33m[WARNING]\e[0m \e[1min file \e[0;33m\"%s\"\e[0m\e[1m:\t"
		"line %u: \e[0;33m%s\e[0m\n\t"
		"\e[1;35m> \e[0m%.*s\n\n",
		filename,
		lineno, error,
		(int)(p - line), line
	);
}

/*
**	format d'erreur:
**
**		in file "myfile.gbs":	l-231: Bad token '$'
**			> %include "file$"
*/
extern void			print_error(char *filename, uint32_t lineno, char *line, const char *error)
{
	char		*p = line;
	uint32_t	len;

	g_error++;
	while (!is_endl(*p)) p++;
	len = p - line;
	if (len > 70)
	{
		p = line + (len - 70);
		len = 70;
	}

	fprintf
	(
		stderr,
		"\e[1;31m[ERROR]\e[0m \e[1min file \e[0;33m\"%s\"\e[0m\e[1m:\t"
		"line %u: \e[0;31m%s\e[0m\n\t"
		"\e[1;31m> \e[0m%.*s\n\n",
		filename,
		lineno, error,
		(int)(len), line
	);

	if (g_error == MAX_ERROR)
	{
		fprintf(stderr, "\e[1;31mtoo many errors emitted\e[0m (%u)\n", g_error);
		exit(g_error);
	}
}

extern void			print_error_dont_show(char *filename, uint32_t lineno, const char *error)
{
	g_error++;

	fprintf
	(
		stderr,
		"\e[1;31m[ERROR]\e[0m \e[1min file \e[0;33m\"%s\"\e[0m\e[1m:\t"
		"line %u: \e[0;31m%s\e[0m\n",
		filename,
		lineno, error
	);

	if (g_error == MAX_ERROR)
	{
		fprintf(stderr, "\e[1;31mtoo many errors emitted\e[0m (%u)\n", g_error);
		exit(g_error);
	}
}
