/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   create_object_file.c                             .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/27 19:25:27 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/18 12:13:51 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "struct.h"
#include "tools.h"
#include "struct_tools.h"

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

	for (uint32_t i = 0; i < label->nitems; i++, lab++)
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
	char				buf[256];
	intern_symbols_t	new;
	memblock_t			*block = (memblock_t *)memblock->data;

	for (uint32_t i = 0; i < memblock->nitems; i++, block++)
	{
		new.name = (uint8_t*)block->name;
		new.type = MEMBLOCK;
		new.data1 = block->start;
		new.data2 = block->end;
		new.blockname = NULL;
		new.pos = NULL;
		new.offset = NULL;
		VEC_SORTED_INSERT(intern_, block->name, new);

		variable_t *var = VEC_ELEM_FIRST(variable_t, block->var);
		for (uint32_t i = 0; i < block->var->nitems; i++, var++)
		{
			if (vector_search(intern_, (void*)&var->name) == -1)
			{
				new.name = (uint8_t*)var->name;
				new.type = VAR;
				new.data1 = 0;
				new.pos = NULL;
				new.data2 = var->size;
				new.blockname = (uint8_t*)block->name;
				VEC_SORTED_INSERT(intern_, var->name, new);
				sprintf(buf, "unused variable `%s` (in block `%s`)", var->name, block->name);
				print_warning_dont_show(var->filename, var->line, buf);
			}
		}
	}
}



void	__attribute__((always_inline))
	add_intern_symbols(vector_t *intern_, loc_sym_t *local_symbol)
{
	add_intern_labels(intern_, local_symbol->label);
	add_intern_memblocks(intern_, local_symbol->memblock); // and unused variables
}



/*
 *	instruction size = 1, 2 (+1) or 3 (+1)
 */
void __attribute__((always_inline))	
		add_no_symbol_instruction(code_t *inst, uint8_t *code[], uint32_t *i, uint32_t *relative_index, uint32_t *allocsize)
{
	uint32_t	i_backup = *i;

	if (*allocsize - *i < 5)
	{
		*allocsize += 128;
		*code = realloc(*code, *allocsize);
	}

	// copy de l'opcode dans code[]
	uint8_t	instruction_len = inst_length[inst->opcode[0]];

	if (instruction_len > 1 && inst->opcode[0] != 0xCBu)
		instruction_len++;
	memcpy(*code + *i, inst->opcode, instruction_len);
	*i += instruction_len;
	*relative_index += *i - i_backup;
}

void __attribute__((always_inline))
		add_external_symbol_instruction(vector_t *extern_, code_t *inst, uint8_t *code[], uint32_t *i, uint32_t *relative_index,
										uint32_t *allocsize, symbol_t *symbol, uint32_t offset)
{
	uint32_t	i_backup = *i;

	if (*allocsize - *i < 5)
	{
		*allocsize += 128;
		*code = realloc(*code, *allocsize);
	}

	// ajout du symbol dans extern_ (header du .o)
	ssize_t				index = vector_search(extern_, (void*)&symbol->name);

	if (index == -1)
	{
		extern_symbols_t	new = {(uint8_t*)symbol->name, symbol->type, 1, NULL, NULL};
		new.pos = malloc(sizeof(uint32_t) * 8);
		new.pos[0] = *relative_index;
		new.offset = malloc(sizeof(uint32_t) * 8);
		new.offset[0] = offset;
		VEC_SORTED_INSERT(extern_, symbol->name, new);
	}
	else
	{
		extern_symbols_t	*elem = VEC_ELEM(extern_symbols_t, extern_, index);
		if ((elem->quantity & 0x7) == 0)
		{
			elem->pos = realloc(elem->pos, (elem->quantity + 8) * sizeof(uint32_t));
			elem->offset = realloc(elem->offset, (elem->quantity + 8) * sizeof(uint32_t));
		}
		elem->pos[elem->quantity] = *relative_index;
		elem->offset[elem->quantity++] = offset;
	}


	// copy de l'opcode dans code[]
	uint8_t	instruction_len = inst_length[inst->opcode[0]];

	if (instruction_len > 1 && inst->opcode[0] != 0xCBu)
		instruction_len++;
	memcpy(*code + *i, inst->opcode, instruction_len);
	*i += instruction_len;
	*relative_index += *i - i_backup;
}




void __attribute__((always_inline))
		add_internal_var_instruction(vector_t *intern_, code_t *inst, uint8_t *code[], uint32_t *i, uint32_t *relative_index,
									uint32_t *allocsize, vector_t *memblock, const char *symbol, uint32_t offset)
{
	int32_t		index, block_index;
	uint32_t	i_backup = *i;


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
			intern_symbols_t	new = {(uint8_t*)var->name, VAR, 1, NULL, NULL, (uint8_t*)block->name, var->size};
			new.pos = malloc(sizeof(uint32_t) * 8);
			new.pos[0] = *relative_index;
			new.offset = malloc(sizeof(uint32_t) * 8);
			new.offset[0] = offset;
			VEC_SORTED_INSERT(intern_, symbol, new);
		}
		else
		{
			//data1 == quantity
			intern_symbols_t	*elem = VEC_ELEM(intern_symbols_t, intern_, intern_index);
			if ((elem->data1 & 0x7) == 0)
			{
				elem->pos = realloc(elem->pos, (elem->data1 + 8) * sizeof(uint32_t));
				elem->offset = realloc(elem->offset, (elem->data1 + 8) * sizeof(uint32_t));
			}
			elem->pos[elem->data1] = *relative_index;
			elem->offset[elem->data1++] = offset;
		}
	}
	else
	{
		fprintf(stderr, "fatal error: undeclared symbol \"%s\"\n", symbol);
		exit(1);
	}

	// copy de l'opcode dans code[]
	uint8_t	instruction_len = inst_length[inst->opcode[0]];

	if (instruction_len > 1 && inst->opcode[0] != 0xCBu)
		instruction_len++;
	memcpy(*code + *i, inst->opcode, instruction_len);
	*i += instruction_len;
	*relative_index += *i - i_backup;
}

static void	set_cartridge_data(uint8_t cartridge_part[], uint8_t *cartridge_part_length)
{
	if (cart_info.all)
	{
		if (cart_info.member.program_start)
		{
			cartridge_part[0] = cartridge.program_start[0];
			cartridge_part[1] = cartridge.program_start[1];
			*cartridge_part_length = 2;
		}
		if (cart_info.member.game_title)
		{
			memcpy(cartridge_part + *cartridge_part_length, cartridge.game_title, 11);
			*cartridge_part_length += 11;
		}
		if (cart_info.member.game_code)
		{
			memcpy(cartridge_part + *cartridge_part_length, cartridge.game_code, 4);
			*cartridge_part_length += 4;
		}
		if (cart_info.member.cgb_support)
		{
			cartridge_part[(*cartridge_part_length)++] = cartridge.cgb_support;
		}
		if (cart_info.member.maker_code)
		{
			cartridge_part[(*cartridge_part_length)++] = cartridge.maker_code[0];
			cartridge_part[(*cartridge_part_length)++] = cartridge.maker_code[1];
		}
		if (cart_info.member.sgb_support)
		{
			cartridge_part[(*cartridge_part_length)++] = cartridge.sgb_support;
		}
		if (cart_info.member.cart_type)
		{
			cartridge_part[(*cartridge_part_length)++] = cartridge.cart_type;
		}
		if (cart_info.member.rom_size)
		{
			cartridge_part[(*cartridge_part_length)++] = cartridge.rom_size;
		}
		if (cart_info.member.ram_size)
		{
			cartridge_part[(*cartridge_part_length)++] = cartridge.ram_size;
		}
		if (cart_info.member.destination)
		{
			cartridge_part[(*cartridge_part_length)++] = cartridge.destination;
		}
		if (cart_info.member.version)
		{
			cartridge_part[(*cartridge_part_length)++] = cartridge.version;
		}
	}
}

int		create_object_file(vector_t *code_area, loc_sym_t *local_symbol, vector_t *extern_symbol, char *filename)
{
	uint8_t		*code;
	uint32_t	i = 0;
	uint32_t	relative_index;
	uint32_t	allocsize;
	uint32_t	j = 0;
	uint32_t	len_pos = 0;
	vector_t	*intern_, *extern_;
	uint8_t		cartridge_part[25];
	uint8_t		cartridge_part_length = 0;
	
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

	if (code_area->nitems == 0)
	{
		fprintf(stderr, "No instructions in the file\n");
		exit(1);
	}

	set_cartridge_data(cartridge_part, &cartridge_part_length);


	for (code_area_t *area = VEC_ELEM_FIRST(code_area_t, code_area); j < code_area->nitems; j++, area++)
	{
		relative_index = 0;
		if (area->size == 0)
			continue;
		if ((allocsize - i) < 16)
		{
			allocsize += 128;
			code = realloc(code, allocsize);
		}

		// ([start_addr]) * 4, ([length]) * 4
		memcpy(code + i, &area->addr, sizeof(uint32_t));
		len_pos = i + sizeof(uint32_t);
		i += sizeof(uint32_t) * 2;

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
				code[i++] = 0xddu;
				code[i++] = 0xddu;
				code[i++] = 0xddu;
				code[i++] = n_bytes >> 8;
				code[i++] = (uint8_t)n_bytes;
				memcpy(code + i, inst->symbol, n_bytes);
				i += n_bytes;
				relative_index += 5 + n_bytes;
			}
			else if (inst->symbol == NULL)
			{
				add_no_symbol_instruction(inst, &code, &i, &relative_index, &allocsize);
			}
			else
			{
				ssize_t	index;
				if ((index = vector_search(extern_symbol, (void*)&inst->symbol)) != -1)
					add_external_symbol_instruction(extern_, inst, &code, &i, &relative_index, &allocsize, VEC_ELEM(symbol_t, extern_symbol, index), area->addr);
				else
					add_internal_var_instruction(intern_, inst, &code, &i, &relative_index, &allocsize, local_symbol->memblock, inst->symbol, area->addr);
			}
		}

//		*(uint32_t*)(code + len_pos) = (uint32_t)(i - (len_pos + 4));
		uint32_t	tmp = i - (len_pos + sizeof(uint32_t));
		memcpy(code + len_pos, &tmp, sizeof(uint32_t));
	}

	// fonction a coder
	add_intern_symbols(intern_, local_symbol);	// symbols dans le vector local_symbol (autres que VAR)

// symbols internes -> ne contient pas les variables inutilisées
// symbols externes -> ne contient pas les symbols inutilisés




	// ecrire dans le fichier .o
	FILE		*file = fopen(filename, "w+");
	uint32_t	header_size = cartridge_part_length;
	uint32_t	intern_size;

	if (header_size)
		header_size += 2; 
	// header_size, cart_info_size, intern_symbol size, code size
	uint32_t	header[4] = {0, header_size, 0, i};

	fwrite(header, sizeof(uint32_t), 4, file);

	if (cartridge_part_length)
	{
		fwrite(&cart_info.all, sizeof(uint16_t), 1, file);
		fwrite(cartridge_part, 1, cartridge_part_length, file);
	}

	j = 0;
	intern_size = 0;
	for (intern_symbols_t *in = (intern_symbols_t *)intern_->data; j < intern_->nitems; j++, in++)
	{
		uint32_t	len = strlen((const char*)in->name) + 1;

		fwrite(in->name, 1, len, file);
		fwrite(&in->type, sizeof(uint32_t), 1, file);
		intern_size += len + sizeof(uint32_t);

		if (in->type == VAR)
		{
#define	n_positions	in->data1
#define	var_size	in->data2
			fwrite(&n_positions, sizeof(uint32_t), 1, file);
			if (n_positions == 0)
			{
				fwrite(&n_positions, sizeof(uint32_t), 1, file);
				fwrite(&n_positions, sizeof(uint32_t), 1, file);
				n_positions++;
			}
			else
			{
				fwrite(in->pos, sizeof(uint32_t), n_positions, file);
				fwrite(in->offset, sizeof(uint32_t), n_positions, file);
			}
			len = strlen((const char*)in->blockname) + 1;
			fwrite(in->blockname, 1, len, file);
			fwrite(&var_size, sizeof(uint32_t), 1, file);
			intern_size += (sizeof(uint32_t) * (2 + (n_positions * 2))) + len;
#undef n_positions
#undef var_size
		}
		else if (in->type == LABEL)
		{
#define address	in->data1
			fwrite(&address, sizeof(uint32_t), 1, file);
			intern_size += sizeof(uint32_t);
#undef address
		}
		else //memblock
		{
#define start	in->data1
#define end		in->data2
			fwrite(&start, sizeof(uint32_t), 1, file);
			fwrite(&end, sizeof(uint32_t), 1, file);
			intern_size += (sizeof(uint32_t) * 2);
#undef	start
#undef	end
		}
	}
	header_size += intern_size;

	j = 0;
	for (extern_symbols_t *ext = (extern_symbols_t *)extern_->data; j < extern_->nitems; j++, ext++)
	{
		uint32_t	len = strlen((const char*)ext->name) + 1;

		fwrite(ext->name, 1, len, file);
		fwrite(&ext->type, sizeof(uint32_t), 1, file);
		fwrite(&ext->quantity, sizeof(uint32_t), 1, file);
		fwrite(ext->pos, sizeof(uint32_t), ext->quantity, file);
		fwrite(ext->offset, sizeof(uint32_t), ext->quantity, file);
		header_size += len + (sizeof(uint32_t) * (2 + (ext->quantity * 2)));
	}

	fwrite(code, sizeof(uint8_t), i, file);

	rewind(file);
	fwrite(&header_size, sizeof(uint32_t), 1, file);
	fseek(file, sizeof(uint32_t), SEEK_CUR);
	fwrite(&intern_size, sizeof(uint32_t), 1, file);

	fclose(file);
	return (0);
}
