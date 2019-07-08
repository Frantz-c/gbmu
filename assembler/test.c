#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct	defines_s
{
	char		*name;
	char		*content;
	uint32_t	count;
	uint32_t	length;
	struct defines_s	*next;
};



struct	operands_s
{
	char				*name;
	struct operands_s	*next;
};

struct	mnemonic_s
{
	char				*name;
	unsigned int		n_operand;
	char				*line;
	struct mnemonics_s	*next;
	struct operands_s	*operands;
};

struct	zones_s
{
	unsigned int		addr;
	struct mnemonics_s	*data;
	struct mnemonics_s	*cur;
	struct zones_s		*next;
	struct zones_s		*prev;
};

struct	error_s
{
	uint32_t	error;
	uint32_t	total;
	int			type[5];
	uint32_t	info[5];
};

typedef struct operands_s	operands_t;
typedef struct mnemonics_s	mnemonics_t;
typedef struct defines_s	defines_t;
typedef struct zones_s		zones_t;
typedef struct error_s		error_t;

void	push_macro(defines_t **def, char *name, char *content, uint32_t count)
{
	defines_t	*new;
	defines_t	*p;
	defines_t	*prev;
	uint32_t	len = strlen(name);

	new = malloc(sizeof(defines_t));
	new->name = name;
	new->content = content;
	new->count = count;
	new->length = len;
	new->next = NULL;
	printf("push(\"%s\")\n", name);

	if (!*def)
	{
		*def = new;
		return;
	}

	p = *def;
	prev = NULL;
	if (p)
	{
		while (p->length < len)
		{
			prev = p;
			p = p->next;
			if (!p)
				goto __add_macro;
		}
		while (p->length == len)
		{
			printf("cmp \"%s\", \"%s\"\n", p->name, name);
			if (strncmp(p->name, name, len) > 0)
			{
				break;
			}
			prev = p;
			p = p->next;
			if (!p)
				break;
		}
	}

	__add_macro:
	if (p == *def)
	{
		*def = new;
		new->next = p;
	}
	else
	{
		new->next = prev->next;
		prev->next = new;
	}
}

int		macro_exists(defines_t *p, char *name)
{
	const uint32_t	len = strlen(name);
	int				diff;

	printf("p = %p\n\n", (void*)p);
	if (p)
	{
		printf("p->len = %u && len = %u\n", p->length, len);
		while (p->length < len)
		{
			p = p->next;
			if (!p)
				return (0);
		}
		while (p->length == len)
		{
			printf("cmp(\"%s\", \"%s\")\n", p->name, name);
			if ((diff = strncmp(p->name, name, len)) == 0)
				return (1);
			else if (diff > 0)
				break;
			p = p->next;
			if (!p)
				break;
		}
	}
	return (0);
}

char	*add_macro_without_param(char *name, defines_t **def, char *s, error_t *err)
{
	fprintf(stderr, "MACRO... exit\n");
	exit(1);
	return (s);
}

void		copy_content(char *dest, char *s)
{
	int			exit = 0;

	do
	{
		for (; ; s++, dest++)
		{
			if (*s == '\n') {
				*(dest++) = *s;
				exit = 1;
				break;
			}
			else if (*s == '\\') {
				*(dest++) = '\n';
				s++;
				break;
			}
			else
				*dest = *s;
		}
		for (; *s != '\n'; s++);
		s++;
	}
	while (!exit);
	*dest = '\0';
}

uint32_t	get_content_length(char *s)
{
	uint32_t	count = 1;
	int			exit = 0;

	do
	{
		for (; ; s++, count++)
		{
			if (*s == '\n') {
				exit = 1;
				count++;
				break;
			}
			else if (*s == '\\') {
				s++;
				break;
			}
		}
		for (; *s != '\n'; s++);
		s++;
	}
	while (!exit);

	return (count);
}

int		string_replace(char *content, char *replace, char number)
{
	char			*chr = content;
	const uint32_t	replace_length = strlen(replace);
	int				count = 0;

	chr = content;
	while ((chr = strchr(chr, *replace)) != NULL)
	{
		if (strncmp(chr, replace, replace_length) == 0)
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

void	skip_macro(char **s)
{
	int	exit = 0;

	do
	{
		for (; ; (*s)++)
		{
			if (**s == '\n') {
				exit = 1;
				break;
			}
			else if (**s == '\\') {
				(*s)++;
				break;
			}
		}
		for (; **s != '\n'; (*s)++);
		(*s)++;
	}
	while (!exit);
	(*s)--;
}

/*
	remplacer les parametres :
	macro(toto,__titi__)  ld toto, __titi__          ->  (int param = 2)  macro, ld #0, #1

ex:
	name = "macro( arg1 , arg2 )"
	name = "macro\0 arg1\0, arg2\0)", pos[0] = "arg1\0", pos[1] = "arg2\0"
*/
char	*add_macro_with_param(char *name, defines_t **def, char *s, error_t *err)
{
	char		*to_free = name;
	uint32_t	count = 0;
	uint32_t	cur = 0;
	char		*pos[11] = {NULL};
	char		*content;
	char		*name2;
	uint32_t	length;

	name2 = name;
	while (*name != '(') name++;
	*name = '\0';
	if (macro_exists(*def, to_free))
	{
		fprintf(stderr, "macro %s is already defined\n", to_free);
		free(to_free);
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
			char	*newline = s;

			// error is like this -> fprintf(stderr, "line x: syntax argument x\n> $line", lineno, err.info);
			fprintf(stderr, "empty argument\n");
			err->error++;
			err->type[err->total] = 1; //ERR_ARGUMENT;
			err->info[err->total] = count;
			err->total++;

			skip_macro(&s);

			free(to_free);
			return (s);
		}
		while (*name != '\0' && *name != ',') name++;
		count++;
	}
	while (*name != '\0');

	while (*s == ' ' || *s == '\t') s++;
	length = get_content_length(s);
	length *= 2;
	content = malloc(length);
	copy_content(content, s);

	char *tmp;
	while (cur--)
	{
		tmp = pos[cur];

		while (*tmp != ' ' && *tmp != '\t' && *tmp != ',' && *tmp != ')' && *tmp != '\0') tmp++;
		*tmp = '\0';
		if (string_replace(content, pos[cur], cur + '0') == 0)
		{
			//WARNING unused argument pos[cur]
			fprintf(stderr, "WARNING: unused argument %s\n", pos[cur]);
		}
		//cur--;
	}

	push_macro(def, name2, content, count);
	return (s);
}

char	*add_macro(defines_t *def[], char *s, error_t *err)
{
	char	*name;
	char	*name_copy;
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
		else if (*s != '_' && *s != ' ' && *s != '\t' && (with_param != 1 || *s != ',')
				&& (*s < '0' || (*s > '9' && *s < 'A') || (*s > 'Z' && *s < 'a')
				|| *s > 'z' || (with_param != 1 && *s == ',')))
		{
			// choisir l'erreur
			fprintf(stderr, "(2)invalid variable name (%c)\n", *s);
			goto __end;
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

	name_copy = strndup(name, s - name);
	while (*s == ' ' || *s == '\t') s++;
	if (*name >= 'a' && *name <= 'z')
	{
		if (with_param == 0)
			s = add_macro_without_param(name_copy, def + (*name - 'a') + 26, s, err);
		else
			s = add_macro_with_param(name_copy, def + (*name - 'a') + 26, s, err);
	}
	else if (*name >= 'A' && *name <= 'Z')
	{
		if (with_param == 0)
			s = add_macro_without_param(name_copy, def + (*name - 'A'), s, err);
		else
			s = add_macro_with_param(name_copy, def + (*name - 'A'), s, err);
	}
	else
	{
		if (with_param == 0)
			s = add_macro_without_param(name_copy, def + 52, s, err);
		else
			s = add_macro_with_param(name_copy, def + 52, s, err);
	}

	__end:
	return (s);
}

int main(void)
{
	defines_t	*def[53] = {NULL};
	error_t		err = {0, 0, {0}, {0}};

	add_macro(def, "prea(arg1,arg2)  ld arg1, arg2\n", &err);
	err.total = 0;
	add_macro(def, "prea(  azerty, qwerty )		push azerty\\\npush qwerty\n", &err);
	err.total = 0;
	add_macro(def, "prea(	a    ,     b   )		pop a\\\npop b\n", &err);

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
