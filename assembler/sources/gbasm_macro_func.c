#include "stdinc.h"

extern void		copy_macro_content(char *dest, char *s)
{
	int	exit = 0;

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

extern uint32_t	get_macro_content_length(char *s)
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

extern void	skip_macro(char **s)
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

extern void	push_macro(defines_t **def, char *name, char *content, uint32_t count)
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

extern int		macro_exists(defines_t *p, char *name)
{
	const uint32_t	len = strlen(name);
	int				diff;

	if (p)
	{
		while (p->length < len)
		{
			p = p->next;
			if (!p)
				return (0);
		}
		while (p->length == len)
		{
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
