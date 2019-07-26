/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_create_object_file.c                       .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/26 20:24:08 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/26 22:39:29 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

typedef struct	intern_symbols_s
{
	uint8_t		*name;
	uint32_t	type;
	uint32_t	quantity;	//start if block
	uint32_t	*pos;
	//var
	uint8_t		*blockname;
	uint32_t	size;
	//block
	uint32_t	end;
}
intern_symbols_t;

typedef struct	extern_symbols_s
{
	uint8_t		*name;
	uint32_t	type;
	uint32_t	quantity;
	uint32_t	*pos;
}
extern_symbols_t;

int		intern_cmp(const void *a, const void *b)
{
	register intern_symbols_t	*ia = (intern_symbols_t *)a;
	register intern_symbols_t	*ib = (intern_symbols_t *)b;
	return (strcmp(ia->name, ib->name));
}

int		extern_cmp(const void *a, const void *b)
{
	register extern_symbols_t	*ea = (extern_symbols_t *)a;
	register extern_symbols_t	*eb = (extern_symbols_t *)b;
	return (strcmp(ea->name, eb->name));
}

void	intern_destroy(const void *a)
{
	register intern_symbols_t	*ia = (intern_symbols_t *)a;

	if (ia->pos) free(ia->pos);
}

void	extern_destroy(const void *a)
{
	register extern_symbols_t	*ea = (extern_symbols_t *)a;

	if (ea->pos) free(ea->pos);
}

int		intern_match(const void *a, const void *b)
{
	register intern_symbols_t	*ia = (intern_symbols_t *)b;
	register char				*comp = *(char **)a;

	return (strcmp(ia->name, comp));
}

int		extern_match(const void *a, const void *b)
{
	register extern_symbols_t	*ea = (extern_symbols_t *)b;
	register char				*comp = *(char **)a;

	return (strcmp(ea->name, comp));
}


int		create_object_file(vector_t *code_area, loc_sym_t *local_symbol, vector_t *extern_symbol, char *filename)
{
	uint8_t		*code;
	uint32_t	i = 0;
	uint32_t	j = 0;
	uint32_t	len = 0;
	uint32_t	len_pos = 0;
	vector_t	*intern_, *extern_;
	
	intern_ = vector_init(sizeof(intern_symbols_t));
	intern_->compar = &intern_cmp;
	intern_->destroy = &intern_destroy;
	intern_->search = &intern_match;
	extern_ = vector_init(sizeof(extern_symbols_t));
	extern_->compar = &extern_cmp;
	extern_->destroy = &extern_destroy;
	extern_->search = &extern_match;

	for (code_area_t *area = VEC_ELEM(code_area_t, code_area, 0); j < code_area->nitems; area += sizeof(code_area_t))
	{
		// allouer, reallouer code
		*((uint16_t*)(code + i)) = (uint16_t)c->addr;
		i += 2;
		len_pos = i;
		i += 4;

		for (code_t *c = area->data; c; c = c->next)
		{
			if (c->size > 0xffu)
			{
				c->size >>= 8;
				len += c->size;
				i += c->size;
				memcpy(code + i, c->symbol, c->size);
			}
			else if (c->symbol)
			{
				ssize_t	index;

				memcpy(code + i, c->code, c->size + (c->size == 3));
				i += c->size + (c->size == 3);
				if ((index = vector_search(extern_symbol, (void*)&c->symbol)) != -1)
				{
					register symbol_t	*sym = VEC_ELEM(symbol_t, extern_symbol, index);

					if ((index = vector_search(_extern, (void*)&c->symbol)) != -1)
					{
						// add pos + inc quantity
					}
					else
					{
						// add new symbol in _extern (quantity = 1, pos = i + 1)
					}
				}
				else
				{
					uint32_t	type;

					if ((index = vector_search(local_symbol->label, (void*)&c->symbol)) != -1)
					{
						register label_t	*lab = VEC_ELEM(label_t, local_symbol->label, index);
					}
					else if ((index = vector_search(local_symbol->memblock, (void*)&c->symbol)) != -1)
					{
						register label_t	*block = VEC_ELEM(memblock_t, local_symbol->memblock, index);
					}
					else
					{
						register variable_t	*var = variable_search(local_symbol->memblock, c->symbol);
					}

					if ((index = vector_search(_extern, (void*)&c->symbol)) != -1)
					{
						// add pos + inc quantity
					}
					else
					{
						// add new symbol in _extern (quantity = 1, pos = i + 1, type = type)
					}

				}
			}
			else
			{
				memcpy(code + i, c->code, c->size);
				i += c->size;
			}
		}
	}
}
