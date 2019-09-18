/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   get_symbols_and_cartridge_info.c                 .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <fcordon@le-101.fr>                +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/09/09 14:55:09 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/18 13:45:06 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"

static void	get_cartridge_info(uint8_t *buf)
{
	uint16_t	info;

	memcpy(&info, buf, 2);
	buf += 2;

/*
	c = a & b
	c = c ^ b
	a != c : error

	a = cart_info;
	b = info;
*/

	{
		register uint16_t	c;

		c = info & cart_info;
		c = c ^ info;
		if (c != info)
			goto __multiple_declaration;
	}

	if (info & PROGRAM_START)
	{
		memcpy(cartridge.program_start, buf, 2);
		buf += 2;
	}
	if (info & GAME_TITLE)
	{
		memcpy(cartridge.game_title, buf, 11);
		buf += 11;
	}
	if (info & GAME_CODE)
	{
		memcpy(cartridge.game_code, buf, 4);
		buf += 4;
	}
	if (info & CGB_SUPPORT)
	{
		cartridge.cgb_support = *(buf++);
	}
	if (info & MAKER_CODE)
	{
		memcpy(cartridge.maker_code, buf, 2);
		buf += 2;
	}
	if (info & SGB_SUPPORT)
	{
		cartridge.sgb_support = *(buf++);
	}
	if (info & CART_TYPE)
	{
		cartridge.cart_type = *(buf++);
	}
	if (info & ROM_SIZE)
	{
		cartridge.rom_size = *(buf++);
	}
	if (info & RAM_SIZE)
	{
		cartridge.ram_size = *(buf++);
	}
	if (info & DESTINATION)
	{
		cartridge.destination = *(buf++);
	}
	if (info & VERSION)
	{
		cartridge.version = *(buf++);
	}
	return;

	__multiple_declaration:
	if ((info & PROGRAM_START) && (cart_info & PROGRAM_START))
		fprintf(stderr, "duplicate program_start\n");
	if ((info & GAME_TITLE) && (cart_info & GAME_TITLE))
		fprintf(stderr, "duplicate game_title\n");
	if ((info & GAME_CODE) && (cart_info & GAME_CODE))
		fprintf(stderr, "duplicate game_code\n");
	if ((info & CGB_SUPPORT) && (cart_info & CGB_SUPPORT))
		fprintf(stderr, "duplicate cgb_support\n");
	if ((info & MAKER_CODE) && (cart_info & MAKER_CODE))
		fprintf(stderr, "duplicate cart_info\n");
	if ((info & SGB_SUPPORT) && (cart_info & SGB_SUPPORT))
		fprintf(stderr, "duplicate sgb_support\n");
	if ((info & CART_TYPE) && (cart_info & CART_TYPE))
		fprintf(stderr, "duplicate cart_type\n");
	if ((info & ROM_SIZE) && (cart_info & ROM_SIZE))
		fprintf(stderr, "duplicate rom_size\n");
	if ((info & RAM_SIZE) && (cart_info & RAM_SIZE))
		fprintf(stderr, "duplicate ram_size\n");
	if ((info & DESTINATION) && (cart_info & DESTINATION))
		fprintf(stderr, "duplicate destination\n");
	if ((info & VERSION) && (cart_info & VERSION))
		fprintf(stderr, "duplicate version\n");
}


static int		add_intern_symbols(loc_symbols_t *loc, char *buf, uint32_t len, uint32_t file_number)
{
	loc_sym_t		tmp;
	uint32_t		type;

	for (uint32_t i = 0; i < len; )
	{
		tmp.name = buf + i;
		while (buf[i]) i++;
		tmp.name = strndup(tmp.name, i);
		i++;

		memcpy(&type, (buf + i), sizeof(uint32_t));
		i += sizeof(uint32_t);



		switch (type)
		{

			case LABEL:
			{
				memcpy(&tmp.data1, buf + i, sizeof(uint32_t));
				i += sizeof(uint32_t);
				tmp.data2 = 0;
				tmp.data = NULL;
				if (vector_search(loc->label, (void*)&tmp.name) != -1)
				{
					free(tmp.name);
					goto __loop_end;
				}

				if (loc->label->nitems == 0)
					vector_push(loc->label, (void*)&tmp);
				else
					VEC_SORTED_INSERT(loc->label, tmp.name, tmp);
				break;
			}


			case VAR:
			{
				var_data_t	*var_data = malloc(sizeof(var_data_t));

				memcpy(&var_data->quantity, buf + i, sizeof(uint32_t));
				i += sizeof(uint32_t);
				if (var_data->quantity == 0)
				{
					free(var_data);
					var_data = NULL;
					i += 2 * 4;
				}
				else
				{
					var_data->pos = malloc(sizeof(uint32_t) * var_data->quantity);
					var_data->offset = malloc(sizeof(uint32_t) * var_data->quantity);
					memcpy(var_data->pos, buf + i, var_data->quantity * sizeof(uint32_t));
					i += (sizeof(uint32_t) * var_data->quantity);
					memcpy(var_data->offset, buf + i, var_data->quantity * sizeof(uint32_t));
					i += (sizeof(uint32_t) * var_data->quantity);
				}
				tmp.blockname = buf + i;
				while (buf[i]) i++;
				tmp.blockname = strndup(tmp.blockname, i);
				i++;
				if (var_data)
				{
					var_data->file_number = file_number;
					var_data->next = NULL;
				}

				memcpy(&tmp.data2, buf + i, sizeof(uint32_t));
				i += sizeof(uint32_t);

				tmp.data1 = 0;	// addr
				tmp.data = var_data;

				ssize_t	index;
				if ((index = vector_search(loc->var, (void*)&tmp.name)) != -1)
				{
					register loc_sym_t	*p = VEC_ELEM(loc_sym_t, loc->var, index);

					if (strcmp(p->name, tmp.name) != 0 || p->data2 != tmp.data2
							|| strcmp(p->blockname, tmp.blockname) != 0)
					{
						g_error++;
						fprintf(stderr, "duplicate symbol %s (file %u, %u)\n", p->name, file_number, p->data->file_number);

						free(tmp.data->pos);
						free(tmp.data);
						free(tmp.blockname);
						goto __loop_end;
					}
					free(tmp.name);
					free(tmp.blockname);
					
					//push var_data
					register var_data_t *v = p->data;
					if (v == NULL)
					{
						p->data = var_data;
					}
					else
					{
						for (; v->next; v = v->next);
						v->next = var_data;
					}

					goto __loop_end;
				}

				if (loc->var->nitems == 0)
					vector_push(loc->var, (void*)&tmp);
				else
					VEC_SORTED_INSERT(loc->var, tmp.name, tmp);
				break;
			}


			case MEMBLOCK:
			{
				tmp.data1 = *(uint32_t*)(buf + i);
				i += sizeof(uint32_t);

				tmp.data2 = *(uint32_t*)(buf + i);
				i += sizeof(uint32_t);

				tmp.data = NULL;

				ssize_t	index;
				if ((index = vector_search(loc->block, (void*)&tmp.name)) != -1)
				{
					register loc_sym_t	*p = VEC_ELEM(loc_sym_t, loc->block, index);

					if (strcmp(p->name, tmp.name) != 0 || p->data2 != tmp.data2
							|| memcmp(&p->data, &tmp.data, sizeof(void*)) != 0)
					{
						g_error++;
						fprintf(stderr, "duplicate symbol %s (file %u, %u)\n", p->name, file_number, p->data->file_number);
					}
					free(tmp.name);
					goto __loop_end;
				}

				if (loc->block->nitems == 0)
					vector_push(loc->block, (void*)&tmp);
				else
					VEC_SORTED_INSERT(loc->block, tmp.name, tmp);
				break;
			}
			default:
			{
				fprintf(stderr, ">>>>> UNKNOWN TYPE <<<<<\n");
				return (-1);
			}
		}

__loop_end:
		continue;
	}
	return (0);
}


static int		add_extern_symbols(vector_t *ext, char *buf, uint32_t len, uint32_t file_number)
{
	ext_sym_t		tmp;
	var_data_t		*data;
	ssize_t			index;
	uint8_t			already_declared = 0;

	tmp.value = 0;
	for (uint32_t i = 0; i < len; )
	{
		tmp.name = buf + i;
		while (buf[i]) i++;
		tmp.name = strndup(tmp.name, i);
		i++;



		memcpy(&tmp.type, buf + i, sizeof(uint32_t));
		i += sizeof(uint32_t);

		if ((tmp.type & 0xfu) != 1) //tmp.type != VAR && tmp.type != LABEL && tmp.type != VAR_OR_LABEL)
			return (-1);
		if ((index = vector_search(ext, (void*)&tmp.name)) != -1)
		{
			register ext_sym_t	*sym = VEC_ELEM(ext_sym_t, ext, index);

			if (sym->type != tmp.type)
			{
				g_error++;
				fprintf(stderr, "multiple declaration of symbol `%s`\n", tmp.name);
				free(tmp.name);
				continue;
			}
			already_declared = 1;
			data = sym->data;
			while (data->next)
				data = data->next;
			data->next = malloc(sizeof(var_data_t));
			data = data->next;
		}
		else
			data = malloc(sizeof(var_data_t));
		data->next = NULL;


		
		memcpy(&data->quantity, buf + i, sizeof(uint32_t));
		i += sizeof(uint32_t);

		data->pos = malloc(sizeof(uint32_t) * data->quantity);
		data->offset = malloc(sizeof(uint32_t) * data->quantity);
		memcpy(data->pos, buf + i, data->quantity * sizeof(uint32_t));
		i += (sizeof(uint32_t) * data->quantity);
		memcpy(data->offset, buf + i, data->quantity * sizeof(uint32_t));
		i += (sizeof(uint32_t) * data->quantity);

		data->file_number = file_number;

		if (ext->nitems == 0)
		{
			tmp.data = data;
			vector_push(ext, (void*)&tmp);
		}
		else
		{
			if (already_declared)
			{
				already_declared = 0;
			}
			else
			{
				tmp.data = data;
				VEC_SORTED_INSERT(ext, tmp.name, tmp);
			}
		}
	}
	return (0);
}


/*
**	
**
*/
extern void	get_symbols_and_cartridge_info(const char *filename, loc_symbols_t *loc, vector_t *ext, uint32_t file_number)
{
	tmp_header_t	header;
	FILE			*file;
	char			*buf = NULL;

	if ((file = fopen(filename, "r")) == NULL)
	{
		fprintf(stderr, "cannot open `%s` file\n", filename);
		g_error++;
		return;
	}

	if (fread(&header, sizeof(tmp_header_t), 1, file) != 1)
		goto __file_error;

	if (header.header_length < header.intern_symbols_length + header.cart_info_length)
		goto __file_error;
	
	if (header.cart_info_length)
	{
		buf = malloc(header.cart_info_length);
		if (fread(buf, 1, header.cart_info_length, file) != header.cart_info_length)
			goto __cart_info_length_error;
		get_cartridge_info((uint8_t *)buf);
	}
	if (header.intern_symbols_length != 0)
	{
		if (header.intern_symbols_length > header.cart_info_length)
			buf = malloc(header.intern_symbols_length);
		if (fread(buf, 1, header.intern_symbols_length, file) != header.intern_symbols_length)
			goto __intern_symbols_len_error;
		if (add_intern_symbols(loc, buf, header.intern_symbols_length, file_number) == -1)
			goto __file_error;
	}


	register uint32_t	extern_length = 
		header.header_length - (header.intern_symbols_length + header.cart_info_length);

	if (extern_length != 0)
	{
		if (buf == NULL)
			buf = malloc(extern_length);
		else if (header.intern_symbols_length < extern_length)
			buf = realloc(buf, extern_length);

		if (fread(buf, 1, extern_length, file) != extern_length)
			goto __extern_symbols_len_error;
		if (add_extern_symbols(ext, buf, extern_length, file_number) == -1)
			goto __file_error;
	}

	free(buf);
	fclose(file);
	return;

__cart_info_length_error:
	fprintf(stderr, "cart_info_error -> ");
	goto __file_error;
__intern_symbols_len_error:
	fprintf(stderr, "intern_symbols_len -> ");
	goto __file_error;
__extern_symbols_len_error:
	fprintf(stderr, "extern_symbols_len -> ");
__file_error:
	fprintf(stderr, "object file not well formated\n");
	g_error++;
	free(buf);
	fclose(file);
	exit(1);
}
