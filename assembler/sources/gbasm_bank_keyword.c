#include "std_includes.h"
#include "gbasm_struct.h"
#include "gbasm_tools.h"

static int	set_addr(char **s, uint32_t *addr, error_t *err)
{
	int32_t		error;

	while (**s == ' ' || **s == '\t') (*s)++;
	
	*addr = atou_inc_all(s, &error);
	if (error == 1)
	{
		fprintf(stderr, ".bank: error argument 1\n");
		__error_argument:
		err->error++;
		//...
		return (-1);
	}
	while (**s == ' ' && **s == '\t') (*s)++;
	if (**s == '\n' || **s == '\0')
		return (0);
	if (**s == ',')
		(*s)++;
	else
	{
		fprintf(stderr, "error token \"%c\"\n", **s);
		err->error++;
		return (-1);
	}
	*addr += atou_inc_all(s, &error);
	if (error == 1)
	{
		fprintf(stderr, ".bank: error argument 2\n");
		goto __error_argument;
	}
	return (0);
}

extern char	*bank_switch(zones_t **zon, zones_t **curzon, char *s, error_t *err)
{
	zones_t		*new;
	uint32_t	addr;

	if (set_addr(&s, &addr, err) == -1)
		goto __ret_s;

	new = malloc(sizeof(zones_t));
	new->addr = addr;
	new->data = NULL;
	new->cur = NULL;
	new->next = NULL;
	new->prev = NULL;
	*curzon = new;

	if (*zon == NULL)
	{
		*zon = new;
		goto __ret_s;
	}
	zones_t		*p = *zon;
	while (p->addr < addr) {
		if (p->next)
			p = p->next;
		else
		{
			if (p->addr < addr)
			{
				p->next = new;
				new->prev = p;
				goto __ret_s;
			}
			break;
		}
	}
	if (p->addr == addr)
	{
		err->error++;
		fprintf(stderr, "duplicate address\n");
		free(new);
		goto __ret_s;
	}

	if (p == *zon)
	{
		new->next = p;
		new->prev = NULL;
		p->prev = new;
		*zon = new;
	}
	else
	{
		new->next = p;
		new->prev = p->prev;
		p->prev->next = p;
		p->prev = new;
	}

	__ret_s:
	while (*s && *s != '\n') s++;
	return (s);
}
