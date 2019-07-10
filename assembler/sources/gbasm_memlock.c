/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_memlock.c                                  .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/10 19:00:27 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/10 20:21:51 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */


char	*set_memlock_area(memblocks_t **memblock, char *s, error_t *err)
{
	uint32_t	addr;
	int			error;
	uint32_t	end;
	memblocks_t	*new, *p;
	char		*name;

//	ARGUMENT 1 -> (string) name
	while (*s == ' ' || *s == '\n') s++;
	name = s;
	if (!is_alpha(*s) && *s != '_')
	{
		goto _error;
	}
	while (is_alnum(*s) || *s == '_') s++;
	if (*s != ' ' && *s != '\t')
		goto __error;
	name = strndup(name, s - name);

//	ARGUMENT 2 -> (uint32) start_addr
	addr = atou_inc_all(&s, &error);
	if (error)
	{
		//error
		goto __error;
	}
	while (*s == ' ' || *s == '\n') s++;
	if (*s == ',') {
		s++;
		while (*s == ' ' || *s == '\n') s++;
	}

//	ARGUMENT 3 -> (uint32) end_addr OR length
	if (strncmp(s, "end", 3) == 0)
		end = 0;
	else if (strncmp(s, "len", 3) == 0)
		end = addr;
	else
	{
		//error
		goto __error;
	}
	s += 3;
	while (*s == ' ' || *s == '\t') s++;
	if (*s != '=')
	{
		//error
		goto __error;
	}
	s++;

	end += atou_inc_all(&s, &error);
	if (error)
	{
		goto __error;
	}

	while (*s == ' ' || *s == '\t') s++;
	if (*s && *s != '\n')
	{
		goto __error;
	}

	new = malloc(sizeof(memblock_t));
	new->start = addr;
	new->end = end;
	new->space = end - addr;
	new->name = name;
	new->next = NULL;

	if (!*memblock)
	{
		*memblock = new;
		return (s);
	}
	p = *memblock;

	while (p->next)
		p = p->next;
	p->next = new;
	return (s);

__error:
		fprintf(stderr, "bad token (%c)\n", *s);
		while (*s && *s != '\n') s++;
		return (s);
}
