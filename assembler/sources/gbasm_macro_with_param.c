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

extern char	*add_macro_with_param(char *name, defines_t **def, char *s, error_t *err)
{
	char		*name_start = name;
	uint32_t	count = 0;
	uint32_t	cur = 0;
	char		*pos[11] = {NULL};
	char		*content;
	uint32_t	length;

	while (*name != '(') name++;
	*name = '\0';
	if (macro_exists(*def, name_start))
	{
		fprintf(stderr, "macro %s is already defined\n", name_start);
		free(name_start);
		skip_macro(&s);
		return (s);
	}
	do
	{
		if (cur == 10)
		{
			//error
			return (s);
		}
		*name = '\0';
		name++;
		while (*name == ' ' || *name == '\t') name++;
		pos[cur++] = name;

		// check if empty argument
		if (*name == ')' || *name == ',')
		{
			// error is like this -> fprintf(stderr, "line x: syntax argument x\n> $line", lineno, err.info);
			fprintf(stderr, "empty argument\n");
			err->error++;
			err->type[err->total] = 1; //ERR_ARGUMENT;
			err->info[err->total] = count;
			err->total++;
			free(name_start);
			skip_macro(&s);
			return (s);
		}
		while (*name != '\0' && *name != ',') name++;
		count++;
	}
	while (*name != '\0');

	while (*s == ' ' || *s == '\t') s++;
	length = get_macro_content_length(s);
	content = malloc(length * 2);
	copy_macro_content(content, s);
	skip_macro(&s);

/*
	vérifier les doublons dans les parametres /!\
*/
	char *arg;
	while (cur--)
	{
		arg = pos[cur];

		while (*arg != ' ' && *arg != '\t' && *arg != ',' && *arg != ')' && *arg != '\0') arg++;
		*arg = '\0';
		if (string_replace(content, pos[cur], cur + '0') == 0)
		{
			//WARNING unused argument pos[cur]
			fprintf(stderr, "WARNING: unused argument %s\n", pos[cur]);
		}
		//cur--;
	}

	push_macro(def, name_start, content, count);
	return (s);
}

/*
int main(void)
{
	defines_t	*def[53] = {NULL};
	error_t		err = {0, 0, {0}, {0}};

	add_macro(def, "ld_ab(arg1,arg2)  ld arg1, arg2\n", &err);
	err.total = 0;
	add_macro(def, "push_x(  azerty, qwerty )		push azerty\\\npush qwerty\n", &err);
	err.total = 0;
	add_macro(def, "pop_x(	a    ,a,     b   )		pop a\\\npop b\n", &err);
	err.total = 0;
	add_macro(def, "macro_test(	a    ,     b ,		 c,d)	add a\\\nbit b\\\ncp c\\\njmp d \n", &err);

	defines_t **p = def;
	defines_t **end = def + 53;
	while (p != end)
	{
		if (*p)
		{
			defines_t *p2 = *p;
			do
			{
				printf("name = \"%s\" -> content = \"%s\"\n\n", p2->name, p2->content);
				p2 = p2->next;
			}
			while (p2);
		}
		p++;
	}
	return (0);
}
*/
