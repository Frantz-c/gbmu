/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_create_object_file.c                       .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/27 19:25:27 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/27 21:29:43 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "gbasm_struct.h"
#include "gbasm_tools.h"
#include "gbasm_struct_tools.h"

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

void	add_intern_labels(vector_t *_intern, vector_t *label)
{
	label_t				new = {0};
	register label_t	*lab = (label_t *)label->data;

	for (uint32_t i = 0; i < label->nitems; i++, lab += sizeof(label_t))
	{
		new.name = lab->name;
		new.type = LABEL;
		new.data1 = lab->base_or_status;
		VEC_SORTED_INSERT(_intern, lab->name, new);
	}
}

void	add_intern_memblocks_and_variables(vector_t *_intern, vector_t *memblock)
{
	memblock_t			new = {0};
	register memblock_t	*block = (memblock_t *)memblock->data;

	for (uint32_t i = 0; i < memblock->nitems; i++, block += sizeof(memblock_t))
	{
		//
	}
}

void	__attribute__((always_inline))
	add_intern_symbols(vector_t *_intern, loc_sym_t *local_symbol)
{
	add_intern_labels(_intern, local_symbol->label);
	add_intern_memblocks_and_variables(_intern, local_symbol->memblock);
}


int		create_object_file(vector_t *code_area, loc_sym_t *local_symbol, vector_t *extern_symbol, char *filename)
{
	uint8_t		*code;
	uint32_t	i = 0;
	uint32_t	allocsize;
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

	code = malloc(sizeof(uint8_t) * 128);
	allocsize = 128;

	for (code_area_t *area = VEC_ELEM(code_area_t, code_area, 0); j < code_area->nitems; area += sizeof(code_area_t))
	{
		if ((allocsize - i) < 10)
		{
			allocsize += 128;
			code = realloc(code, allocsize);
		}

		*((uint16_t*)(code + i)) = (uint16_t)c->addr;
		i += 2;
		len_pos = i;
		i += 4;

		/************* ADD VAR OR LABEL *************/
		for (code_t *c = area->data; c; c = c->next)
		{
			if (c->size > 0xffu)
			{
				c->size >>= 8;
				if ((allocsize - i) < c->size)
				{
					allocsize += c->size;
					code = realloc(code, allocsize);
				}
				len += c->size;
				i += c->size;
				memcpy(code + i, c->symbol, c->size);
			}
			else if (c->symbol)
			{
				ssize_t	index;

				// extern symbol
				if ((index = vector_search(extern_symbol, (void*)&c->symbol)) != -1)
				{
					register symbol_t	*sym = VEC_ELEM(symbol_t, extern_symbol, index);

					if ((index = vector_search(_extern, (void*)&c->symbol)) != -1)
					{
						register extern_symbols_t	*sym = VEC_ELEM(extern_symbols_t, _intern, index);
						sym->pos[sym->quantity] = i;
						sym->quantity++;
						if ((sym->quantity % 4) == 0)
							sym->pos = realloc(sym->pos, sym->quantity + 4);
						// add pos + inc quantity
					}
					else
					{
						extern_symbols_t	new = {sym->name, sym->type, 1, NULL};
						new.pos = malloc(sizeof(uint32_t) * 4);
						new.pos[0] = i;
						VEC_SORTED_INSERT(_extern, c->symbol, new);
						// add new symbol in _extern (quantity = 1, pos = i + 1)
					}
				}

				// add instruction
				if ((allocsize - i) < c->size + (c->size == 3))
				{
					allocsize += 128;
					code = realloc(code, allocsize);
				}
				memcpy(code + i, c->code, c->size + (c->size == 3));
				i += c->size + (c->size == 3);
			}
			else
			{
				if ((allocsize - i) < c->size)
				{
					allocsize += 128;
					code = realloc(code, allocsize);
				}
				memcpy(code + i, c->code, c->size);
				i += c->size;
			}
		}
		/*****************END******************/
		*(uint32_t*)(code + len_pos) = (uint32_t)(i - (len_pos + 4));
	}

	// fonction a coder
	add_intern_symbols(_intern, local_symbol);	// symbols dans le vector local_symbol (non utilisés par les instructions)

	// ecrire dans le fichier .o
	FILE *file = fopen(filename, "w+");

	uint32_t	i = 0;

	uint32_t	header[4] =
	{
		_extern->nitems * sizeof(extern_symbols_t) + _intern->nitems * sizeof(intern_symbols_t),
		_extern->nitems,
		_intern->nitems,
		i
	};

	fwrite(header, sizeof(uint32_t), 4, file);
	for (extern_symbols_t *ext = (extern_symbols_t *)_extern->data; i < _extern->nitems; i++, ext += sizeof(extern_symbols_t))
	{
		uint32_t	len = strlen(ext->name) + 1;

		fwrite(ext->name, 1, len, file);
		fwrite(&ext->type, sizeof(uint32_t), 1, file);
		fwrite(&ext->quantity, sizeof(uint32_t), 1, file);
		fwrite(ext->pos, sizeof(uint32_t), ext->quantity, ext->pos);
	}
	i = 0;
	for (intern_symbols_t *in = (intern_symbols_t *)_intern->data; i < _intern->nitems; i++, in += sizeof(intern_symbols_t))
	{
		uint32_t	len = strlen(in->name) + 1;

		fwrite(in->name, 1, len, file);
		fwrite(&in->type, sizeof(uint32_t), 1, file);
		fwrite(&in->quantity, sizeof(uint32_t), 1, file);
		if (in->type & VAR_OR_LABEL)
		{
			fwrite(in->pos, sizeof(uint32_t), in->quantity, in->pos);
			if (in->type == VAR)
			{
				len = strlen(in->blockname) + 1;

				fwrite(in->blockname, 1, len, file);
				fwrite(&in->data1, sizeof(uint32_t), 1, file);
				fwrite(&in->data2, sizeof(uint32_t), 1, file);
			}
			else
			{
				fwrite(&in->data1, sizeof(uint32_t), 1, file);
			}
		}
		else //memblock
		{
			fwrite(&in->data1, sizeof(uint32_t), 1, file);
			fwrite(&in->data2, sizeof(uint32_t), 1, file);
		}
	}
	fwrite(code, sizeof(uint8_t), i, file);

	fclose(file);
}
