#include "stdinc.h"

extern char	*add_macro(defines_t *def[], char *s, error_t *err)
{
	char	*name;
	int		with_param = 0;

	while (*s == ' ' || *s == '\t') s++;
	if (*s == '\n') {
		// choisir l'erreur
		fprintf(stderr, "pas de valeur\n");
		return (s);
	}

	if (*s != '_' && (*s < 'A' || (*s > 'Z' && *s < 'a') || *s > 'z'))
	{
		// choisir l'erreur
		fprintf(stderr, "(1)invalid variable name\n");
		goto __end;
	}
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
			// choisir l'erreur
			fprintf(stderr, "(2)invalid variable name (%c)\n", *s);
			skip_macro(&s);
			return (s);
		}
		else if (*s == ' ' && *s == '\t' && *s == '\n')
			break;
	}
	if (with_param == 1)
	{
		fprintf(stderr, "invalid syntax\n");
		// choisir l'erreur
		goto __end;
	}

	name = strndup(name, s - name);
	while (*s == ' ' || *s == '\t') s++;
	if (*name >= 'a' && *name <= 'z')
	{
		if (with_param == 0)
			s = add_macro_without_param(name, def + (*name - 'a') + 26, s, err);
		else
			s = add_macro_with_param(name, def + (*name - 'a') + 26, s, err);
	}
	else if (*name >= 'A' && *name <= 'Z')
	{
		if (with_param == 0)
			s = add_macro_without_param(name, def + (*name - 'A'), s, err);
		else
			s = add_macro_with_param(name, def + (*name - 'A'), s, err);
	}
	else
	{
		if (with_param == 0)
			s = add_macro_without_param(name, def + 52, s, err);
		else
			s = add_macro_with_param(name, def + 52, s, err);
	}

	__end:
	return (s);
}
