#include "../includes/std_includes.h"
#include "../includes/gbasm_struct.h"
#include "../includes/gbasm_macro_func.h"
#include "../includes/gbasm_macro.h"

extern char	*define_macro(defines_t *def[], char *s, error_t *err)
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
			if (with_param == 0 && (*s == ' ' || *s == '\t'))
				break;
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

static void	delete_macro_if_defined(defines_t **def, char *name)
{
	const uint32_t	len = strlen(name);
	int				diff;
	defines_t		*prev = *def;
	defines_t		*p = *def;

	if (p)
	{
		if (p->next == NULL)
		{
			if (p->length == len && strcmp(p->name, name) == 0)
			{
__del_first:
				free(p->name);
				prev = p->next;
				free(p);
				*def = prev;
			}
			return;
		}
		while (p->length < len)
		{
			prev = p;
			p = p->next;
			if (!p)
				return;
		}
		while (p->length == len)
		{
			if ((diff = strncmp(p->name, name, len)) == 0)
			{
				if (prev == p)
					goto __del_first;
				free(p->name);
				prev->next = p->next;
				free(p);
			}
			else if (diff > 0)
				break;
			prev = p;
			p = p->next;
			if (!p)
				break;
		}
	}
	return;
}

extern char	*undef_macro(defines_t *def[], char *s, error_t *err)
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
	{
		//error
		fprintf(stderr, "error #1\n");
		while (*s != '\n') s++;
		return (s);
	}
	name = strndup(name, s - name);
	if (*name >= 'a' && *name <= 'z')
		delete_macro_if_defined(def + (*name - 'a') + 26, name);
	else if (*name >= 'A' && *name <= 'Z')
		delete_macro_if_defined(def + (*name - 'A'), name);
	else // if (*name == '_')
		delete_macro_if_defined(def + 53, name);
	while (*s == ' ' && *s == '\t') s++;
	if (*s != '\n')
	{
		//error;
		fprintf(stderr, "error #2\n");
		while (*s != '\n') s++;
	}
	return (s);
}
