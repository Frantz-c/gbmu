/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_main.c                                     .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/11 10:36:42 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/09 20:22:44 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "assign_addr_to_var.h"
#include "get_symbols_and_cartridge_info.h"
#include "all_extern_symbols_exist.h"
#include "callback.h"

uint16_t	cart_info = 0;
cart_data_t	cartridge = {{0x0, 0xC3},{0},{0},{0},0,{0},0,0,0,0,0,0x33,0,0,{0}};
uint32_t	g_error = 0;
uint32_t	g_warning = 0;


void	*get_cartridge_header_code(void)
{
	void	*buf;

	buf = malloc(0x50);
	
	memcpy(buf, &cartridge, 4);
	memcpy(buf + 4, NINTENDO_LOGO, 48);
	memcpy(buf + 52, &cartridge.game_title, 28);

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
	uint8_t	*n = cartridge.game_title;
	uint8_t total = 0;

	for (uint32_t i = 0; i < 25; i++)
	{
		total += n[i];
	}
	total += 0x19u;
	return ((uint8_t)(0x100u - total));
}
/*
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
*/

const char	*get_type_str(uint32_t type)
{
	if (type == LABEL)
		return ("label");
	if (type == VAR)
		return ("variable");
	if (type == MEMBLOCK)
		return ("block");
	return ("unknown");
}


uint8_t	inst_length[256] =
{
//	0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F
	1,3,1,1,1,1,2,1,3,1,1,1,1,1,2,1,
	1,3,1,1,1,1,2,1,2,1,1,1,1,1,2,1,
	2,3,1,1,1,1,2,1,2,1,1,1,1,1,2,1,
	2,3,1,1,1,1,2,1,2,1,1,1,1,1,2,1,
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

void		get_code_with_replacement(loc_symbols_t *loc, vector_t *ext, vector_t *code, char *files[])
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
		fseek(file, header.header_length, SEEK_CUR);

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
				fprintf(stderr, "erreur de chargement du code dans le fichier %s (%u/%u)\n", files[i], ret, code_length);
				exit(1);
			}
		
			// replace all extern symbols with their value
	//		k = 0;
			{
				register ext_sym_t	*elem = VEC_ELEM_FIRST(ext_sym_t, ext);

				printf("ext->nitems = %zu\n", ext->nitems);
				for (uint32_t j = 0; j < ext->nitems; j++, elem++)
				{

					for (var_data_t *data = elem->data; data; data = data->next)
					{


						printf("file_num = %u, i = %u\n", data->file_number, i);
						if (data->file_number == i)
						{
							register loc_sym_t	*p;

							if (elem->type == VAR)
							{
								register ssize_t	index = vector_search(loc->var, (void*)&elem->name);
								p = VEC_ELEM(loc_sym_t, loc->var, index); // sizeof(loc_sym_t) == sizeof(loc_var_t);
							}
							else // if (elem->type == LABEL)
							{
								register ssize_t	index = vector_search(loc->label, (void*)&elem->name);
								p = (loc_sym_t *)VEC_ELEM(loc_label_t, loc->label, index); // sizeof(loc_sym_t) == sizeof(loc_var_t);
							}

							for (uint32_t l = 0; l < data->quantity; l++)
							{
								uint8_t		*inst = buf + data->pos[l] - 8; // - 8 = - code_header_length
								uint8_t		symbol_size = inst_length[*inst];
								
								printf(">>>> replace at inst 0x%x\n", *inst);

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
			}

			/*
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
			*/

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
				/*
				else if (len == 4)
				{
					inc = 1;
					len = 2;
				}
				*/

				printf("\e[0;45m > \e[0mlen = %hhu\n", len);

				while (len)
				{
					new_buf[z++] = buf[j++];
					if (--len == 0)
						break;
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
					puts("\e[0;41;1;37mGROS PROBLEME, ILLEGAL INSTRUCTION !!\e[0m");
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
	switch (cartridge.rom_size)
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

static void __attribute__((always_inline))	ext_init(vector_t **ext)
{
	(*ext) = vector_init(sizeof(ext_sym_t));

	(*ext)->destroy = &ext_destroy;
	(*ext)->compar = &loc_ext_compar;
	(*ext)->search = &loc_ext_search;
}

static void __attribute__((always_inline))	loc_init(loc_symbols_t *loc)
{
	loc->var = vector_init(sizeof(loc_var_t));
	loc->label = vector_init(sizeof(loc_label_t));
	loc->block = vector_init(sizeof(loc_block_t));

	loc->var->destroy = &loc_var_destroy;
	loc->var->compar = &loc_ext_compar;
	loc->var->search = &loc_ext_search;

	loc->label->destroy = &loc_destroy;
	loc->label->compar = &loc_ext_compar;
	loc->label->search = &loc_ext_search;

	loc->block->destroy = &loc_destroy;
	loc->block->compar = &loc_ext_compar;
	loc->block->search = &loc_ext_search;
}

static void __attribute__((always_inline))	code_init(vector_t **code)
{
	*code = vector_init(sizeof(all_code_t));
	(*code)->destroy = &code_destroy;
	(*code)->compar = &code_compar;
	(*code)->search = &code_search;
}
/*
	argv = {executable, object, ...}

	idee d'amelioration: separer les variables des blocks et des labels
*/
int main(int argc, char *argv[])
{
	if (argc < 2)
		exit(1);

	loc_symbols_t	loc;
	vector_t		*ext;
	vector_t		*code;
	char		*exe = argv[0];

	loc_init(&loc);
	ext_init(&ext);
	code_init(&code);


	argv++;
	for (uint32_t i = 0; argv[i]; i++)
		get_symbols_and_cartridge_info(argv[i], &loc, ext, i);


	all_extern_symbols_exist(&loc, ext, argv);
	if (g_error)
		exit(1);

	assign_addr_to_var(&loc);
	if (g_error)
		exit(2);

	// DEBUG
	puts("\e[1;34mintern labels:\e[0m");
	{
		register loc_label_t	*tmp = VEC_ELEM_FIRST(loc_label_t, loc.label);
		for (uint32_t i = 0; i < loc.label->nitems; i++, tmp++)
		{
			printf(
				"%s ==> addr = 0x%0.4x\n",
				tmp->name, tmp->value
			);
		}
	}
	puts("\e[1;34mintern variables:\e[0m");
	{
		register loc_var_t	*tmp = VEC_ELEM_FIRST(loc_var_t, loc.var);
		for (uint32_t i = 0; i < loc.var->nitems; i++, tmp++)
		{
			printf(
				"%s ==> addr = 0x%0.4x, size = %u, block = \"%s\"\n",
				tmp->name, tmp->addr, tmp->size, tmp->blockname
			);
		}
	}
	puts("\e[1;34mintern blocks:\e[0m");
	{
		register loc_block_t	*tmp = VEC_ELEM_FIRST(loc_block_t, loc.block);
		for (uint32_t i = 0; i < loc.block->nitems; i++, tmp++)
		{
			printf(
				"%s ==> start = 0x%0.4x, end = 0x%0.4x\n",
				tmp->name, tmp->start, tmp->end
			);
		}
	}

	puts("\e[1;36mextern symbols:\e[0m");
	{
		register ext_sym_t	*tmp = VEC_ELEM_FIRST(ext_sym_t, ext);
		for (uint32_t i = 0; i < ext->nitems; i++, tmp++)
		{
			printf(
				"%s (%s)\n",
				tmp->name, get_type_str(tmp->type)
			);
		}
	}
	// END DEBUG


	puts("\e[1;32m___EXIT___\e[0m");
	exit(0);
	get_code_with_replacement(&loc, ext, code, argv);
	if (g_error)
		exit(3);

	write_binary(code, exe);

	return (0);
}
