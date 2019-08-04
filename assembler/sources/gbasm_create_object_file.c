/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_create_object_file.c                       .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/27 19:25:27 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/04 18:39:44 by fcordon     ###    #+. /#+    ###.fr     */
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
	return (strcmp((char*)ia->name, (char*)ib->name));
}

int		extern_cmp(const void *a, const void *b)
{
	register extern_symbols_t	*ea = (extern_symbols_t *)a;
	register extern_symbols_t	*eb = (extern_symbols_t *)b;
	return (strcmp((char*)ea->name, (char*)eb->name));
}

void	intern_destroy(void *a)
{
	register intern_symbols_t	*ia = (intern_symbols_t *)a;

	if (ia->pos) free(ia->pos);
}

void	extern_destroy(void *a)
{
	register extern_symbols_t	*ea = (extern_symbols_t *)a;

	if (ea->pos) free(ea->pos);
}

int		intern_match(const void *a, const void *b)
{
	register intern_symbols_t	*ia = (intern_symbols_t *)b;
	register char				*comp = *(char **)a;

	return (strcmp((char*)ia->name, comp));
}

int		extern_match(const void *a, const void *b)
{
	register extern_symbols_t	*ea = (extern_symbols_t *)b;
	register char				*comp = *(char **)a;

	return (strcmp((char*)ea->name, comp));
}

void	add_intern_labels(vector_t *intern_, vector_t *label)
{
	intern_symbols_t	new;
	register label_t	*lab = (label_t *)label->data;

	for (uint32_t i = 0; i < label->nitems; i++, lab += sizeof(label_t))
	{
		if (vector_search(intern_, (void*)&lab->name) != -1)
			continue;
		new.name = (uint8_t*)lab->name;
		new.type = LABEL;
		new.data1 = lab->base_or_status;
		VEC_SORTED_INSERT(intern_, lab->name, new);
	}
}

void	add_intern_memblocks(vector_t *intern_, vector_t *memblock)
{
	intern_symbols_t	new;
	register memblock_t	*block = (memblock_t *)memblock->data;

	for (uint32_t i = 0; i < memblock->nitems; i++, block += sizeof(memblock_t))
	{
		new.name = (uint8_t*)block->name;
		new.type = MEMBLOCK;
		new.data1 = block->start;
		new.data2 = block->end;
		VEC_SORTED_INSERT(intern_, block->name, new);
	}
}

void	__attribute__((always_inline))
	add_intern_symbols(vector_t *intern_, loc_sym_t *local_symbol)
{
	add_intern_labels(intern_, local_symbol->label);
	add_intern_memblocks(intern_, local_symbol->memblock);
}

/*
 *	instruction size = 1, 2 (+1) or 3 (+1)
 */
void __attribute__((always_inline))	
		add_no_symbol_instruction(code_t *inst, uint8_t *code[], uint32_t *i, uint32_t *allocsize)
{
	if (*allocsize - *i < 5)
	{
		*allocsize += 128;
		*code = realloc(*code, *allocsize);
	}


	(*code)[(*i)++] = (uint8_t)inst->size;
	if (inst->size == 3)
	{
		memcpy(*code + *i, inst->opcode, 4);
		*i += 4;
	}
	else if (inst->size == 2)
	{
		memcpy(*code + *i, inst->opcode, 2);
		*i += 2;
		(*code)[(*i)++] = inst->opcode[3];
	}
	else
	{
		(*code)[(*i)++] = inst->opcode[0];
	}
}

void __attribute__((always_inline))
		add_external_symbol_instruction(vector_t *extern_, code_t *inst, uint8_t *code[],
										uint32_t *i, uint32_t *allocsize, symbol_t *symbol)
{
	if (*allocsize - *i < 5)
	{
		*allocsize += 128;
		*code = realloc(*code, *allocsize);
	}

	// ajout du symbol dans extern_ (header du .o)
	ssize_t				index = vector_search(extern_, (void*)&symbol->name);

	if (index == -1)
	{
		extern_symbols_t	new = {(uint8_t*)symbol->name, symbol->type, 1, NULL};
		new.pos = malloc(sizeof(uint32_t) * 8);
		new.pos[0] = *i;
		VEC_SORTED_INSERT(extern_, symbol->name, new);
	}
	else
	{
		extern_symbols_t	*elem = VEC_ELEM(extern_symbols_t, extern_, index);
		if ((elem->quantity & 0x7) == 0)
			elem->pos = realloc(elem->pos, elem->quantity + 8);
		elem->pos[elem->quantity++] = *i;
	}

	// copy de l'opcode dans code[]
	(*code)[(*i)++] = (uint8_t)inst->size;
	if (inst->size == 3)
	{
		memcpy(*code + *i, inst->opcode, 4);
		*i += 4;
	}
	else if (inst->size == 2)
	{
		memcpy(*code + *i, inst->opcode, 2);
		*i += 2;
		(*code)[(*i)++] = inst->opcode[3];
	}
	else
	{
		fprintf(stderr, "forbidden size %u !\n", inst->size);
		exit(1);
	}
}

void __attribute__((always_inline))
		add_internal_var_instruction(vector_t *intern_, code_t *inst, uint8_t *code[], uint32_t *i,
									uint32_t *allocsize, vector_t *memblock, const char *symbol)
{
	int32_t	index, block_index;


	if (*allocsize - *i < 5)
	{
		*allocsize += 128;
		*code = realloc(*code, *allocsize);
	}

	index = variables_match_name(memblock, symbol, &block_index);
	if (index != -1)
	{
		memblock_t	*block = VEC_ELEM(memblock_t, memblock, block_index);
		variable_t	*var = VEC_ELEM(variable_t, block->var, index);
		ssize_t		intern_index = vector_search(intern_, (void*)&symbol);

		if (intern_index == -1)
		{
			intern_symbols_t	new = {(uint8_t*)var->name, VAR, 1, NULL, (uint8_t*)block->name, var->size};
			new.pos = malloc(sizeof(uint32_t) * 8);
			new.pos[0] = *i;
			VEC_SORTED_INSERT(intern_, symbol, new);
		}
		else
		{
			//data1 == quantity
			intern_symbols_t	*elem = VEC_ELEM(intern_symbols_t, intern_, intern_index);
			if ((elem->data1 & 0x7) == 0)
				elem->pos = realloc(elem->pos, elem->data1 + 8);
			elem->pos[elem->data1++] = *i;
		}
	}
	else
	{
		fprintf(stderr, "fatal error: undeclared symbol \"%s\"\n", symbol);
		exit(1);
	}


	// copy de l'opcode dans code[]
	(*code)[(*i)++] = (uint8_t)inst->size;
	if (inst->size == 3)
	{
		memcpy(*code + *i, inst->opcode, 4);
		*i += 4;
	}
	else
	{
		// if size == 2, error (jr var illegal)
		fprintf(stderr, "forbidden size %u !\n", inst->size);
		exit(1);
	}
}

int		create_object_file(vector_t *code_area, loc_sym_t *local_symbol, vector_t *extern_symbol, char *filename)
{
	uint8_t		*code;
	uint32_t	i = 0;
	uint32_t	allocsize;
	uint32_t	j = 0;
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

	printf("\e[1;31m(5)\e[0mcode_area->addr = %x\n", VEC_ELEM_FIRST(code_area_t, code_area)->addr);
	printf("&code_area->addr = %p\n", &VEC_ELEM_FIRST(code_area_t, code_area)->addr);
	if (code_area->nitems == 0)
	{
		fprintf(stderr, "No instructions on the file\n");
		exit(1);
	}
	printf("\e[1;36mcode_area->nitems = %u\e[0m\n", code_area->nitems);
	for (code_area_t *area = VEC_ELEM_FIRST(code_area_t, code_area); j < code_area->nitems; j++, area += sizeof(code_area_t))
	{
		printf("&code_area->addr = %p\n", &area->addr);
		if ((allocsize - i) < 10)
		{
			allocsize += 128;
			code = realloc(code, allocsize);
		}

		// ([start_addr]) * 2, ([length]) * 4
		code[i] = (uint8_t)(area->addr >> 8);
		code[i + 1] = (uint8_t)area->addr;
		len_pos = i + 2;
		i += 6;

		for (register code_t *inst = area->data; inst; inst = inst->next)
		{
			// .byte
			if (inst->size & BYTE_DIRECTIVE)
			{
				register uint32_t	n_bytes = (inst->size & 0xffffff00u) >> 8;
				if (allocsize - i < n_bytes + 5)
				{
					allocsize += n_bytes - (allocsize - i) + 128;
					code = realloc(code, allocsize);
				}
				code[i++] = 0x0;
				code[i++] = n_bytes >> 24;
				code[i++] = n_bytes >> 16;
				code[i++] = n_bytes >> 8;
				code[i++] = (uint8_t)n_bytes;
				memcpy(code + i, inst->symbol, n_bytes);
				i += n_bytes;
			}
			else if (inst->symbol == NULL)
			{
				add_no_symbol_instruction(inst, &code, &i, &allocsize);
			}
			else
			{
				ssize_t	index;
				if ((index = vector_search(extern_symbol, (void*)&inst->symbol)) != -1)
					add_external_symbol_instruction(extern_, inst, &code, &i, &allocsize, VEC_ELEM(symbol_t, extern_symbol, index));
				else
					add_internal_var_instruction(intern_, inst, &code, &i, &allocsize, local_symbol->memblock, inst->symbol);
			}
		}

		*(uint32_t*)(code + len_pos) = (uint32_t)(i - (len_pos + 4));
	}

	// fonction a coder
	add_intern_symbols(intern_, local_symbol);	// symbols dans le vector local_symbol (autres que VAR)

// symbols internes -> ne contient pas les variables inutilisées
// symbols externes -> ne contient pas les symbols inutilisés




	// ecrire dans le fichier .o
	FILE *file = fopen(filename, "w+");

	uint32_t	header[4] =
	{
		extern_->nitems * sizeof(extern_symbols_t) + intern_->nitems * sizeof(intern_symbols_t),
		extern_->nitems,
		intern_->nitems,
		i
	};

	j = 0;
	fwrite(header, sizeof(uint32_t), 4, file);
	for (extern_symbols_t *ext = (extern_symbols_t *)extern_->data; j < extern_->nitems; j++, ext += sizeof(extern_symbols_t))
	{
		uint32_t	len = strlen((const char*)ext->name) + 1;

		fwrite(ext->name, 1, len, file);
		fwrite(&ext->type, sizeof(uint32_t), 1, file);
		fwrite(&ext->quantity, sizeof(uint32_t), 1, file);
		fwrite(ext->pos, sizeof(uint32_t), ext->quantity, file);
	}
	j = 0;
	for (intern_symbols_t *in = (intern_symbols_t *)intern_->data; j < intern_->nitems; j++, in += sizeof(intern_symbols_t))
	{
		uint32_t	len = strlen((const char*)in->name) + 1;

		fwrite(in->name, 1, len, file);
		fwrite(&in->type, sizeof(uint32_t), 1, file);

		if (in->type == VAR)
		{
			fwrite(&in->data1, sizeof(uint32_t), 1, file);
			fwrite(in->pos, sizeof(uint32_t), in->data1, file);
			len = strlen((const char*)in->blockname) + 1;
			fwrite(in->blockname, 1, len, file);
			fwrite(&in->data2, sizeof(uint32_t), 1, file);
		}
		else if (in->type == LABEL)
		{
			fwrite(&in->data1, sizeof(uint32_t), 1, file);
		}
		else //memblock
		{
			fwrite(&in->data1, sizeof(uint32_t), 1, file);
			fwrite(&in->data2, sizeof(uint32_t), 1, file);
		}
	}
	fwrite(code, sizeof(uint8_t), i, file);

	fclose(file);
	return (0);
}
