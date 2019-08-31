/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_main.c                                     .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/11 10:36:42 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/31 20:42:08 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"

uint32_t		g_error;
uint32_t		g_warning;
cart_info_t		cartridge_info;

void	*get_cartridge_header_code(void)
{
	void	*buf;

	buf = malloc(0x50);
	
	memcpy(buf, &cartridge_info, 4);
	memcpy(buf + 4, NINTENDO_LOGO, 48);
	memcpy(buf + 52, &cartridge_info.title, 28);

	return (buf);
}

char	*get_object_name(const char *s)
{
	const char	*end = s + strlen(s) - 1;

	while (*end != '.')
	{
		if (end == s)
		{
			g_error++;
			fprintf(stderr, "fichier sans extention !\n");
			exit(1);
		}
		end--;
	}

	char	*new = malloc((end - s) + 5);
	sprintf(new, "%.*s.gbo", (int)(end - s), s);
	return (new);
}

uint8_t	get_complement_check(void)
{
	uint8_t	*n = cartridge_info.title;
	uint8_t total = 0;

	for (uint32_t i = 0; i < 25; i++)
	{
		total += n[i];
	}
	total += 0x19u;
	return ((uint8_t)(0x100u - total));
}

int			all_code_cmp(const void *a, const void *b)
{
	register all_code_t	*aa = (all_code_t *)a;
	register all_code_t	*bb = (all_code_t *)b;

	if (aa->start > bb->start)
		return (1);
	else if (aa->start < bb->start)
		return (-1);
	return (0);
}

int			all_code_search(const void *b, const void *a)
{
	register all_code_t	*aa = (all_code_t *)a;
	register uint32_t	bb = *(uint32_t *)b;

	if (aa->start > bb)
		return (1);
	if (aa->start < bb)
		return (-1);
	return (0);
}

void		all_code_destroy(void *a)
{
	register all_code_t	*aa = (all_code_t *)a;

	free(aa->code);
}

int			all_ext_sym_cmp(const void *a, const void *b)
{
	register all_ext_sym_t	*aa = (all_ext_sym_t *)a;
	register all_ext_sym_t	*bb = (all_ext_sym_t *)b;

	return (strcmp(aa->name, bb->name));
}

int			all_ext_sym_search(const void *b, const void *a)
{
	register all_ext_sym_t	*aa = (all_ext_sym_t *)a;
	register char			*bb = *(char **)b;

	return (strcmp(aa->name, bb));
}

void		all_ext_sym_destroy(void *a)
{
	register all_ext_sym_t	*aa = (all_ext_sym_t *)a;

	free(aa->name);
	free(aa->pos);
}

int			all_sym_cmp(const void *a, const void *b)
{
	register all_sym_t	*aa = (all_sym_t *)a;
	register all_sym_t	*bb = (all_sym_t *)b;

	return (strcmp(aa->name, bb->name));
}

int			all_sym_search(const void *b, const void *a)
{
	register all_sym_t	*aa = (all_sym_t *)a;
	register char		*bb = *(char **)b;

	return (strcmp(aa->name, bb));
}

void		all_sym_destroy(void *a)
{
	register all_sym_t	*aa = (all_sym_t *)a;

	free(aa->name);
	if (aa->var_data)
	{
		register var_data_t	*p = aa->var_data;
		while (p)
		{
			register var_data_t	*tmp = p;
			free(p->block);
			free(p->pos);
			p = p->next;
			free(tmp);
		}
		free(aa->var_data);
	}
}

int			memblock2_cmp(const void *a, const void *b)
{
	register memblock2_t	*aa = (memblock2_t *)a;
	register memblock2_t	*bb = (memblock2_t *)b;

	return (strcmp(aa->name, bb->name));
}

int			memblock2_search(const void *b, const void *a)
{
	register memblock2_t	*aa = (memblock2_t *)a;
	register char			*bb = *(char **)b;

	printf("\e[0;33msearch\e[0m(%s, %s)\n", aa->name, bb);
	return (strcmp(aa->name, bb));
}

const char	*get_type_str(uint32_t type)
{
	if (type == LABEL)
		return ("label");
	if (type == VAR)
		return ("variable");
	if (type == MEMBLOCK)
		return ("memblock");
	return ("unknown");
}

int		add_intern_symbols_bin(vector_t *sym, char *buf, uint32_t len, uint32_t file_number)
{
	all_sym_t		tmp;

	for (uint32_t i = 0; i < len; )
	{
		register char	*name;
		
		name = buf + i;
		while (buf[i]) i++;
		tmp.name = strndup(name, i);
		i++;
		printf("INTERN SYMBOL FOUND = %s\n", tmp.name);

		memcpy(&tmp.type, (buf + i), sizeof(uint32_t));
		i += sizeof(uint32_t);

		switch (tmp.type)
		{
			case LABEL:
			{
//				tmp.data1 = *(uint32_t*)(buf + i);
				memcpy(&tmp.data1, buf + i, sizeof(uint32_t));
				i += sizeof(uint32_t);
				tmp.data2 = 0;
				tmp.var_data = NULL;
				if (vector_search(sym, (void*)&tmp.name) != -1)
				{
					free(tmp.name);
					goto __loop_end;
				}
				break;
			}
			case VAR:
			{
				var_data_t	*var_data = malloc(sizeof(var_data_t));

//				var_data->quantity = *(uint32_t *)(buf + i);
				memcpy(&var_data->quantity, buf + i, sizeof(uint32_t));
				i += sizeof(uint32_t);
				var_data->pos = malloc(sizeof(uint32_t) * var_data->quantity);
				memcpy(var_data->pos, buf + i, var_data->quantity * sizeof(uint32_t));
				i += (sizeof(uint32_t) * var_data->quantity);
				name = buf + i;
				while (buf[i]) i++;
				var_data->block = strndup(name, i);
				i++;
				var_data->file_number = file_number;
				var_data->next = NULL;

//				tmp.data2 = *(uint32_t*)(buf + i);
				memcpy(&tmp.data2, buf + i, sizeof(uint32_t));
				i += sizeof(uint32_t);

				tmp.data1 = 0;	// addr
				tmp.var_data = var_data;

				ssize_t	index;
				if ((index = vector_search(sym, (void*)&tmp.name)) != -1)
				{
					register all_sym_t	*p = VEC_ELEM(all_sym_t, sym, index);

					if (strcmp(p->name, tmp.name) != 0 || p->type != tmp.type
							|| p->data2 != tmp.data2 || strcmp(p->var_data->block, tmp.var_data->block) != 0)
					{
						g_error++;
						fprintf(stderr, "duplicate symbol %s (file %u, %u)\n", p->name, file_number, p->var_data->file_number);

						free(tmp.var_data->block);
						free(tmp.var_data->pos);
						free(tmp.var_data);
					}
					free(tmp.name);
					
					//push var_data
					
					register var_data_t *v = p->var_data;
					for (; v->next; v = v->next);
					v->next = var_data;

					goto __loop_end;
				}
				break;
			}
			case MEMBLOCK:
			{
				tmp.data1 = *(uint32_t*)(buf + i);
				i += sizeof(uint32_t);

				tmp.data2 = *(uint32_t*)(buf + i);
				i += sizeof(uint32_t);

				tmp.var_data = NULL;

				ssize_t	index;
				if ((index = vector_search(sym, (void*)&tmp.name)) != -1)
				{
					register all_sym_t	*p = VEC_ELEM(all_sym_t, sym, index);

					if (strcmp(p->name, tmp.name) != 0 || p->type != tmp.type
							|| p->data2 != tmp.data2 || memcmp(&p->var_data, &tmp.var_data, sizeof(void*)) != 0)
					{
						g_error++;
						fprintf(stderr, "duplicate symbol %s (file %u, %u)\n", p->name, file_number, p->var_data->file_number);
					}
					free(tmp.name);
					goto __loop_end;
				}
				break;
			}
			default:
			{
				puts(">>>>> UNKNOWN TYPE <<<<<");
				return (-1);
			}
		}

		if (sym->nitems == 0)
			vector_push(sym, (void*)&tmp);
		else
			VEC_SORTED_INSERT(sym, tmp.name, tmp);
__loop_end:
		continue;
	}
	return (0);
}

int		add_extern_symbols_bin(vector_t *sym, char *buf, uint32_t len, uint32_t file_number)
{
	all_ext_sym_t		tmp;

	for (uint32_t i = 0; i < len; )
	{
		register char	*name;
		
		name = buf + i;
		while (buf[i]) i++;
		tmp.name = strndup(name, i);
		i++;
		printf("EXTERN SYMBOL FOUND = %s\n", tmp.name);

//		tmp.type = *(uint32_t*)(buf + i);
		memcpy(&tmp.type, buf + i, sizeof(uint32_t));
		i += sizeof(uint32_t);

		if ((tmp.type & 0xfu) != 1) //tmp.type != VAR && tmp.type != LABEL && tmp.type != VAR_OR_LABEL)
		{
			printf("symbol criminel ==> \"%s\"\n", tmp.name);
			return (-1);
		}

//		tmp.quantity = *(uint32_t*)(buf + i);
		memcpy(&tmp.quantity, buf + i, sizeof(uint32_t));
		i += sizeof(uint32_t);

		tmp.pos = malloc(sizeof(uint32_t) * tmp.quantity);
		memcpy(tmp.pos, buf + i, tmp.quantity * sizeof(uint32_t));
		i += (sizeof(uint32_t) * tmp.quantity);

		tmp.file_number = file_number;

		if (sym->nitems == 0)
			vector_push(sym, (void*)&tmp);
		else
			VEC_SORTED_INSERT(sym, tmp.name, tmp);
	}
	return (0);
}

void	get_symbols(const char *filename, vector_t *sym, vector_t *ext_sym, uint32_t file_number)
{
	tmp_header_t	header;
	FILE			*file;
	char			*buf;

	if ((file = fopen(filename, "r")) == NULL)
	{
		fprintf(stderr, "file %s does not exist\n", filename);
		exit(1);
	}

	if (fread(&header, sizeof(tmp_header_t), 1, file) != 1)
		goto __file_error;

	printf("{%u, %u, %u}\n", header.header_length, header.intern_symbols_length, header.code_length);
	if (header.header_length < header.intern_symbols_length)
		goto __file_error;
	
	if (header.intern_symbols_length != 0)
	{
		buf = malloc(header.intern_symbols_length);
		if (fread(buf, 1, header.intern_symbols_length, file) != header.intern_symbols_length)
		{
			free(buf);
			goto __file_error;
		}
		if (add_intern_symbols_bin(sym, buf, header.intern_symbols_length, file_number) == -1)
		{
			free(buf);
			puts("#xxx");
			// free(var)
			goto __file_error;
		}
	}


	register uint32_t	extern_length = header.header_length - header.intern_symbols_length;

	if (extern_length != 0)
	{
		if (buf == NULL)
			buf = malloc(extern_length);
		else if (header.intern_symbols_length < extern_length)
			buf = realloc(buf, extern_length);

		if (fread(buf, 1, extern_length, file) != extern_length)
		{
			free(buf);
			goto __file_error;
		}
		if (add_extern_symbols_bin(ext_sym, buf, extern_length, file_number) == -1)
		{
			free(buf);
			puts("#yyy");
			// free(var)
			goto __file_error;
		}
	}

	free(buf);
	fclose(file);
	return;

__file_error:
	fprintf(stderr, "object file not well formated\n");
	fclose(file);
	exit(1);
}

void		all_extern_symbols_exist(vector_t *sym, vector_t *ext_sym, char *file[])
{
	register all_ext_sym_t	*elem = VEC_ELEM_FIRST(all_ext_sym_t, ext_sym);

	for (uint32_t i = 0; i < ext_sym->nitems; i++, elem++)
	{
		if (vector_search(sym, (void*)&elem->name) == -1)
		{
			g_error++;
			fprintf(stderr, "undefined symbol %s (file %s)\n", elem->name, file[elem->file_number]);
		}
	}
}

void		assign_var_addr(vector_t *sym)
{
	vector_t	*memblock;

	memblock = vector_init(sizeof(memblock2_t));
	memblock->search = &memblock2_search;
	memblock->compar = &memblock2_cmp;

	// add all memory blocks
	{
		register all_sym_t	*elem = VEC_ELEM_FIRST(all_sym_t, sym);

		for (uint32_t i = 0; i < sym->nitems; i++, elem++)
		{

			if (elem->type == MEMBLOCK)
			{
				printf("memblock found ! (%s)\n", elem->name);
				memblock2_t	new = {elem->data1, elem->data2, elem->data2 - elem->data1, 0, elem->name};
				if (memblock->nitems == 0)
					vector_push(memblock, (void*)&new);
				else
					VEC_SORTED_INSERT(memblock, elem->name, new);
			}

		}
	}

	// add var in blocks
	{
		register all_sym_t	*elem = VEC_ELEM_FIRST(all_sym_t, sym);

		for (uint32_t i = 0; i < sym->nitems; i++, elem++)
		{
			register ssize_t	index;

			if (elem->type == VAR)
			{
				printf("search variable %s in all memory blocks\n", elem->var_data->block);
				if ((index = vector_search(memblock, (void*)&elem->var_data->block)) != -1)
				{
					register memblock2_t	*block = VEC_ELEM(memblock2_t, memblock, index);

					if (block->space < elem->data2)
					{
						fprintf(stderr, "too few space in %s memory block for variable %s\n", block->name, elem->name);
						g_error++;
					}
					else
					{
						printf("variable %s found !\n", elem->name);
						elem->data1 = block->end - block->space;
						block->space -= elem->data2;
					}
				}
			}
			
		}
	}

	// print remaind space in memblocks
	for (uint32_t i = 0; i < memblock->nitems; i++)
	{
		register memblock2_t	*block = VEC_ELEM(memblock2_t, memblock, i);

		if (block->space != 0)
		{
			fprintf(stderr, "WARNING: %u bytes left in %s memory block\n", block->space, block->name);
		}
	}

}

uint8_t	inst_length[256] =
{
//	0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F
	1,3,1,1,1,1,2,1,3,1,1,1,1,1,2,1,
	1,3,1,1,1,1,2,1,4,1,1,1,1,1,2,1,
	4,3,1,1,1,1,2,1,4,1,1,1,1,1,2,1,
	4,3,1,1,1,1,2,1,4,1,1,1,1,1,2,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,3,3,3,1,2,1,1,1,3,2,3,3,2,1,
	1,1,3,0,3,1,2,1,1,1,3,0,3,0,2,1,
	2,1,1,0,0,1,2,1,2,1,3,0,0,0,2,1,
	2,1,1,1,0,1,2,1,2,1,3,1,0,0,2,1
};

void		get_code_with_replacement(vector_t *sym, vector_t *ext_sym, vector_t *code, char *files[])
{
	uint8_t			*buf;
	FILE			*file;
	tmp_header_t	header;
	uint32_t		length = 0;
	uint32_t		code_start, code_length;
	uint32_t		ret = 0;

	for (uint32_t i = 0; files[i]; i++)
	{
		file = fopen(files[i], "r");
		fread(&header, sizeof(tmp_header_t), 1, file);
		fseek(file, header.header_length + 12, SEEK_SET);

		while (fread(&code_start, sizeof(uint32_t), 1, file) == 1)
		{

			if (fread(&code_length, sizeof(uint32_t), 1, file) != 1)
			{
				printf("error code_length\n");
				goto __error;
			}
			printf("code_start = 0x%x, code_length = %u\n", code_start, code_length);

			if (length == 0)
				buf = malloc(code_length);
			else if (length < code_length)
				buf = realloc(buf, code_length);

			if ((ret = fread(buf, 1, code_length, file)) != code_length)
			{
			__error:
				fprintf(stderr, "erreur chargement du code fichier %s (%u/%u)\n", files[i], ret, code_length);
				exit(1);
			}
		
			// replace all extern symbols with their value
	//		k = 0;
			{
				register all_ext_sym_t	*elem = VEC_ELEM_FIRST(all_ext_sym_t, ext_sym);

				printf("ext_sym->nitems = %zu\n", ext_sym->nitems);
				for (uint32_t j = 0; j < ext_sym->nitems; j++, elem++)
				{

					printf("file_num = %u, i = %u\n", elem->file_number, i);
					if (elem->file_number == i)
					{
						register ssize_t	index = vector_search(sym, (void*)&elem->name);
						register all_sym_t	*p = VEC_ELEM(all_sym_t, sym, index);

						for (uint32_t l = 0; l < elem->quantity; l++)
						{
							uint8_t		*inst = buf + elem->pos[l] - 8;
							uint8_t		symbol_size = inst_length[*inst];
							
							printf(">>>> repace at inst 0x%x\n", *inst);

							uint8_t		operation = inst[symbol_size];
							inst++;
							uint32_t	value = inst[0] | (inst[1] << 8);

							printf("EXT SYM VALUE : 0x%x %c %u\n", p->data1, operation, value);
							if (operation == '-')
								value = p->data1 - value;
							else if (operation == '+')
								value = p->data1 + value;
							else
								value = p->data1;

							if (symbol_size == 3)
							{
								if (value > 0xffff)
									fprintf(stderr, "OVERFLOW inst ????? file ?????\n");

								inst[0] = (uint8_t)value;
								inst[1] = (uint8_t)(value >> 8);
								inst[2] = 0xdd;
							}
							else if (symbol_size == 2)
							{
								if (value > 0xff)
									fprintf(stderr, "OVERFLOW inst ????? file ?????\n");

								inst[0] = (uint8_t)value;
								inst[1] = 0xdd;
							}

						}
					}
				}
			}

			// replace all extern symbols with their value
	//		k = 0;
			{
				register all_sym_t	*elem = VEC_ELEM_FIRST(all_sym_t, sym);

				printf("int_sym->nitems = %zu\n", sym->nitems);
				for (uint32_t j = 0; j < sym->nitems; j++, elem++)
				{

					if (elem->type == VAR && elem->var_data->file_number == i)
					{
						register var_data_t	*var = elem->var_data;
						printf("file_num = %u\n", var->file_number);
						for (uint32_t l = 0; l < var->quantity; l++)
						{
							printf("var_pos = %u\n", var->pos[l] - 8);
							uint8_t		*inst = buf + var->pos[l] - 8;
							uint8_t		symbol_size = inst_length[*inst];
							
							printf(">>>> repace at inst 0x%x\n", *inst);

							uint8_t		operation = inst[symbol_size];
							inst++;
							uint32_t	value = inst[0] | (inst[1] << 8);

							printf("VAR ADDR = 0x%x\n", elem->data1);
							if (operation == '-')
								value -= elem->data1;
							else if (operation == '+')
								value += elem->data1;
							else
								value = elem->data1;

							if (symbol_size == 3)
							{
								if (value > 0xffff)
									fprintf(stderr, "OVERFLOW inst ????? file ?????\n");

								inst[0] = (uint8_t)value;
								inst[1] = (uint8_t)(value >> 8);
								inst[2] = 0xdd;
							}
							else if (symbol_size == 2)
							{
								if (value > 0xff)
									fprintf(stderr, "OVERFLOW inst ????? file ?????\n");

								inst[0] = (uint8_t)value;
								inst[1] = 0xdd;
							}

						}
					}
				}
			}

// cut info in code
		uint8_t		*new_buf = malloc(header.code_length - 8);
		uint32_t	z = 0;

		for (uint32_t j = 0; j < code_length;)
		{
			register uint8_t	len;
			register uint8_t	inc = 0;

			if ((len = inst_length[buf[j]]) != 0)
			{
				if ((len == 2 && buf[j] != 0xcbU) || len == 3)
					inc++;
				else if (len == 4)
				{
					inc = 1;
					len = 2;
				}

				printf("\e[0;45m > \e[0mlen = %hhu\n", len);

				while (len)
				{
					new_buf[z++] = buf[j++];
					if (--len == 0)
						break;
				//	j++;
				}
				j += inc;
			}
			else
			{
				if (buf[j] == 0xddu && buf[j+1] == 0xddu && buf[j+2] == 0xddu)
				{
					register uint32_t	l = (buf[j+3] << 8) | buf[j+4];
					printf("l = %u\n", l);
					j += 5;
					memcpy(new_buf + z, buf + j, l);
					j += l;
					z += l;
				}
				else
				{
					puts("\e[0;41;1;37mGROS PROBLEME, ILLEGAL INSTRUCTION EN DEHORS DE .BYTE\e[0m");
				}
			}
		}

		all_code_t	new = {code_start, code_start + z, new_buf, i};
		vector_push(code, (void*)&new);
		}
		fclose(file);
	}

	all_code_t	new = {0x100, 0x150, NULL, -1};
	new.code = get_cartridge_header_code();
	vector_push(code, (void*)&new);

	vector_sort(code);

	//check code overlap
	register all_code_t		*elem = VEC_ELEM_FIRST(all_code_t, code);
	uint32_t				end = elem->end;
	if ((elem->start > 0x100 && elem->start < 0x150) || (elem->end > 0x100 && elem->end < 0x150))
	{
		fprintf(stderr, "cartridge header overlap\n");
		g_error++;
	}
	printf("(0) start = 0x%x\n", elem->start);
	elem++;
	for (uint32_t i = 1; i < code->nitems; i++, elem++)
	{
		printf("(%u) start = 0x%x\n", i, elem->start);
		if (elem->start < end)
		{
			fprintf(stderr, "code overlap in 0x%x (current code start), 0x%x (previous code end)\n", elem->start, end);
			g_error++;
		}
		if ((elem->start > 0x100 && elem->start < 0x150) || (elem->end > 0x100 && elem->end < 0x150))
		{
			fprintf(stderr, "cartridge header overlap\n");
			g_error++;
		}
	}
}

/*
00h -  32KBytes (no ROM banking)
01h -  64KBytes (4 banks)
02h - 128KBytes (8 banks)
03h - 256KBytes (16 banks)
04h - 512KBytes (32 banks)
05h -   1MByte (64 banks)  - only 63 banks used by MBC1
06h -   2MBytes (128 banks) - only 125 banks used by MBC1
07h -   4MBytes (256 banks)
08h -   8MBytes (512 banks)
52h - 1.1MBytes (72 banks)
53h - 1.2MBytes (80 banks)
54h - 1.5MBytes (96 banks)
*/
uint32_t	get_rom_end(void)
{
	switch (cartridge_info.rom_size)
	{
		case 0x00: return (0x8000u);
		case 0x01: return (0x10000u);
		case 0x02: return (0x20000u);
		case 0x03: return (0x40000u);
		case 0x04: return (0x80000u);
		case 0x05: return (0x100000u); //if mc1 -1 bank
		case 0x06: return (0x200000u); //if mbc1 -1 bank
		case 0x07: return (0x400000u);
		case 0x08: return (0x800000u);
	}
	return (0);
}

void		write_binary(vector_t *code, const char *filename)
{
	register all_code_t		*elem = VEC_ELEM_FIRST(all_code_t, code);
	FILE					*file;
	uint8_t					fill[128] = {0};

	if ((file = fopen(filename, "w+")) == NULL)
	{
		fprintf(stderr, "cannot open the file %s\n", filename);
		exit(1);
	}

	register uint32_t	tmp = elem->start;
	register uint32_t	end = elem->end;
	
	printf("WRITE 0x%x\n", elem->start);
	while (tmp > 128)
	{
		fwrite(fill, 1, 128, file);
		tmp -= 128;
	}
	fwrite(fill, 1, tmp, file);
	fwrite(elem->code, 1, elem->end - elem->start, file);
	elem++;

	for (uint32_t i = 1; i < code->nitems; i++, elem++)
	{
	printf("WRITE 0x%x\n", elem->start);
		if (end < elem->start)
		{
			tmp = elem->start - end;
			
			while (tmp > 128)
			{
				fwrite(fill, 1, 128, file);
				tmp -= 128;
			}
			fwrite(fill, 1, tmp, file);
		}
		fwrite(elem->code, 1, elem->end - elem->start, file);
		end = elem->end;
	}

	if (end > get_rom_end())
	{
		fprintf(stderr, "ERROR: code too long\n");
		exit(1);
	}

	if (end < get_rom_end())
	{
		tmp = get_rom_end() - end;
		
		while (tmp > 128)
		{
			fwrite(fill, 1, 128, file);
			tmp -= 128;
		}
		fwrite(fill, 1, tmp, file);
	}
	fclose(file);
}


int main(int argc, char *argv[])
{
	// nouveau fichier !!!!!!
	
	cartridge_info._0x00c3[0] = 0x00U;
	cartridge_info._0x00c3[1] = 0xc3U;
	cartridge_info._0x33 = 0x33U;


	/* infos temporaires */
	cartridge_info.start_addr[0] = 0x50u;
	cartridge_info.start_addr[1] = 0x01u;
	strncpy((char*)cartridge_info.title, "__TEST__\0\0\0", 11);
	strncpy((char*)&cartridge_info.game_code, "TOTO", 4);
	cartridge_info.cgb_support = 0x80;
	cartridge_info.maker_code[0] = '0';
	cartridge_info.maker_code[1] = '1';
	cartridge_info.sgb_support = 0;
	cartridge_info.game_pack_type = 0x1bU;	// mbc-5 + SRAM + BATTERY
	cartridge_info.rom_size = 0;			// 256 KBits (32 KBytes)
	cartridge_info.ram_size = 2;			// 64 Kbit (8 KBytes)
	cartridge_info.destination = 1;			// All others
	cartridge_info.mask_rom_version = 0;	// version du jeu
	cartridge_info.complement_check = get_complement_check();
	/* end */




	vector_t	*sym = vector_init(sizeof(all_sym_t));
	vector_t	*ext_sym = vector_init(sizeof(all_ext_sym_t));
	vector_t	*code = vector_init(sizeof(all_code_t));

	ext_sym->destroy = &all_ext_sym_destroy;
	ext_sym->compar = &all_ext_sym_cmp;
	ext_sym->search = &all_ext_sym_search;
	sym->destroy = &all_sym_destroy;
	sym->compar = &all_sym_cmp;
	sym->search = &all_sym_search;
	code->destroy = &all_code_destroy;
	code->compar = &all_code_cmp;
	code->search = &all_code_search;

	if (g_error == 0)
	{
		g_error = 0;
		for (uint32_t i = 0; file[i]; i++)
		{
			get_symbols(file[i], sym, ext_sym, i);
		}

		all_extern_symbols_exist(sym, ext_sym, file);
		if (g_error)
			goto __end_compilation;

		assign_var_addr(sym);
		if (g_error)
			goto __end_compilation;

		puts("intern symbols");
		for (uint32_t i = 0; i < sym->nitems; i++)
		{
			register all_sym_t	*tmp = VEC_ELEM(all_sym_t, sym, i);

			printf("%s (%s) = 0x%x, 0x%x\n", tmp->name, get_type_str(tmp->type), tmp->data1, tmp->data2);
		}
		puts("extern symbols");
		for (uint32_t i = 0; i < ext_sym->nitems; i++)
		{
			register all_ext_sym_t	*tmp = VEC_ELEM(all_ext_sym_t, ext_sym, i);

			printf("%s (%s) : quantity 0x%x\n", tmp->name, get_type_str(tmp->type), tmp->quantity);
		}

		get_code_with_replacement(sym, ext_sym, code, file);
		if (g_error)
			goto __end_compilation;

		write_binary(code, "binary.gb");

		//free tout le bordel

	__end_compilation:
		if (g_error)
		{
			fprintf(stderr, "\e[1;31m%u errors\e[0m\n", g_error);
			return (1);
		}
		for (uint32_t i = 0; file[i]; i++)
			free(file[i]);
	}
	return (0);
}

