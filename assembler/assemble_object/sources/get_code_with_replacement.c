/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   get_code_with_replacement.c                      .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <fcordon@le-101.fr>                +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/09/10 11:12:37 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/12 13:20:51 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"

static const uint8_t	inst_length[256] =
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

static uint8_t	get_complement_check(void)
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

static void	*get_cartridge_header_code(void)
{
	void	*buf;

	buf = malloc(0x50);
	
	cartridge.complement_check = get_complement_check();
	memcpy(buf, &cartridge, 4);
	memcpy(buf + 4, NINTENDO_LOGO, 48);
	memcpy(buf + 52, &cartridge.game_title, 28);

	return (buf);
}

extern void		get_code_with_replacement(loc_symbols_t *loc, vector_t *ext, vector_t *code, char *files[])
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
				fprintf(stderr, "error code_length\n");
				goto __error;
			}

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

				for (uint32_t j = 0; j < ext->nitems; j++, elem++)
				{

					for (var_data_t *data = elem->data; data; data = data->next)
					{


						if (data->file_number == i)
						{
							for (uint32_t l = 0; l < data->quantity; l++)
							{
								if (code_start != data->offset[l])
									continue;

								uint8_t		*inst = buf + data->pos[l];
								uint8_t		symbol_size = inst_length[*inst];
								

								uint8_t		operation = inst[symbol_size];
								inst++;
								uint32_t	value = inst[0] | (inst[1] << 8);

								if (operation == '-')
									value = elem->value - value;
								else if (operation == '+')
									value = elem->value + value;
								else if (operation == '/')
									value = elem->value / value;
								else if (operation == '%')
									value = elem->value % value;
								else if (operation == '*')
									value = elem->value * value;
								else if (operation == '^')
									value = elem->value ^ value;
								else if (operation == '&')
									value = elem->value & value;
								else if (operation == '|')
									value = elem->value | value;
								else if (operation == '>')
									value = (elem->value >> value);
								else if (operation == '<')
									value = (elem->value << value);
								else
									value = elem->value;

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
						j += 5;
						memcpy(new_buf + z, buf + j, l);
						j += l;
						z += l;
					}
					else
					{
						fprintf(stderr, "\e[0;41;1;37mGROS PROBLEME, ILLEGAL INSTRUCTION !!\e[0m\n");
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
	elem++;
	for (uint32_t i = 1; i < code->nitems; i++, elem++)
	{
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
