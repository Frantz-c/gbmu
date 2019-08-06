/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_main.c                                     .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/11 10:36:42 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/06 11:36:30 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "gbasm_tools.h"
#include "gbasm_macro.h"
#include "gbasm_keywords.h"
#include "gbasm_callback.h"
#include "gbasm_error.h"
#include "gbasm_struct.h"
#include "gbasm_instruction_or_label.h"
#include "gbasm_create_object_file.h"

#define BEG						do{
#define END						}while(0)
#define	SKIP_SPACES(ptr)		BEG while (is_space(*(ptr))) (ptr)++; END

#define CHECK_ERROR_DIRECTIVE(n)	\
	if (!is_space(s[n]))\
	{\
		s += n;\
		while (!is_endl(*s)) s++;\
		print_directive_arg_error(get_keyword(data.line), &data);\
		continue;\
	}

#define CHECK_ERROR_KEYWORD(n)	\
	if (!is_space(s[n]))\
	{\
		s += n;\
		while (!is_endl(*s)) s++;\
		print_keyword_arg_error(get_keyword(data.line), &data);\
		continue;\
	}


uint32_t		g_error;
uint32_t		g_warning;
cart_info_t		cartridge_info;


void	check_code_area_overflow(vector_t *area)
{
	register code_area_t	*a;
	register uint32_t	end;
	
	if (area->nitems == 0)
		return;
	a = (code_area_t *)area->data;
	end = a->addr + a->size;
	a++;

	for (uint32_t i = 1; i < area->nitems; i++, a++)
	{
		if (end >= a->addr)
		{
			g_error++;
			fprintf(stderr, "chevauchement zone memoire (end = 0x%x, start = 0x%x)\n", end, a->addr);
		}
	}
}

/*
 *	replace labels only
 */
void		replace_internal_symbols(vector_t *area, loc_sym_t *local_symbol)
{
	register code_area_t	*a;

	a = (code_area_t *)area->data;
	for (uint32_t i = 0; i < area->nitems; i++, a++)
	{
		register code_t	*c = a->data;
		for (; c; c = c->next)
		{
			if (c->symbol && (c->size & 0xffffff00u) == 0)
			{
				// remplacement + calcul
				register ssize_t	index;

				if ((index = vector_search(local_symbol->label, (void*)&c->symbol)) != -1)
				{
					register label_t	*lab = VEC_ELEM(label_t, local_symbol->label, index);
					register uint32_t	val;
					register uint32_t	lab_addr = lab->base_or_status;
					
					if (lab_addr >= 0x8000)
						lab_addr = (lab_addr % 0x4000) + 0x4000;
					val = c->opcode[1] | (c->opcode[2] << 8);
					printf("val = 0x%x, lab->base = 0x%x\n", val, lab->base_or_status);
					val = (c->opcode[3] == '-') ? lab_addr - val: lab_addr + val;
					c->opcode[1] = (uint8_t)val;
					c->opcode[2] = (val >> 8);
					if (val > 0xffffu)
					{
						g_error++;
						fprintf(stderr, "overflow label\n");
					}
					if (*c->opcode == JR || *c->opcode == JRZ || *c->opcode == JRNZ
						|| *c->opcode == JRC || *c->opcode == JRNC)
					{
						if (val & 0xff00u)
						{
							g_error++;
							fprintf(stderr, "too big jump\n");
						}
					}
					free(c->symbol);
					c->symbol = NULL;
				}
			}
		}
	}
}

void		print_directive_arg_error(char *keyword, data_t *data)
{
	sprintf(data->buf, "`%s` keyword expects arguments", keyword);
	free(keyword);
	print_error(data->filename, data->lineno, data->line, data->buf);
	fprintf(stderr, "\e[1;31m%u errors\e[0m\n", g_error);
}

static char	*get_include_filename(char **s, data_t *data)
{
	char	*name;

	while (is_space(**s)) (*s)++;
	name = *s;
	while (!is_space(**s) && !is_endl(**s)) (*s)++;
	name = strndup(name, *s - name);
	while (is_space(**s)) (*s)++;
	if (!is_endl(**s))
		goto __unexpected_char;
	return (name);

__unexpected_char:
	sprintf(data->buf, "unexpected character `%c`", **s);
	print_error(data->filename, data->lineno, data->line, data->buf);
	fprintf(stderr, "\e[1;31m%u errors\e[0m\n", g_error);
	return (NULL);
}

void			area_print(const void *a)
{
	code_area_t	*area = (code_area_t *)a;

	printf("addr = 0x%04x\n", area->addr);
	if (area->data)
	{
		for (code_t *c = area->data; c; c = c->next)
		{
			if (c->size & 0xffffff00u)
			{
				uint8_t		*bytes = (uint8_t*)c->symbol;
				uint32_t	size = (c->size & 0xffffff00u) >> 8;
				for (uint8_t i = 0; i < size; i++)
				{
					if (i % 8 == 7)
						puts("");
					if (i == 0)
						printf("\tbytes[%u] : 0x%hhx", size, bytes[i]);
					else
						printf(", 0x%hhx", bytes[i]);
				}
			}
			else
			{
				uint8_t	size = c->size;
				printf("\t");
				for (uint8_t i = 0; i < size; i++)
				{
					printf("0x%hhX, ", c->opcode[i]);
				}
			}
			printf("\n");
		}
	}
	else
		printf("\tno data\n");
}

void			symbol_print(const void *a)
{
	symbol_t	*s = (symbol_t *)a;

	printf("\t%s\n", s->name);
}

void			label_print(const void *a)
{
	label_t	*l = (label_t *)a;

	printf("\t%s\n", l->name);
}

void			macro_print(const void *a)
{
	register macro_t	*p = (macro_t *)a;

	printf("%%define %s(%u) %s\n", p->name, p->argc, p->content);
}

void			memblock_print(const void *a)
{
	memblock_t *b = (memblock_t *)a;
	vector_t	*v = ((memblock_t *)a)->var;

	printf("block %s, 0x%x - 0x%x\n", b->name, b->start, b->end);

	if (v)
	{
		for (uint32_t i = 0; i < v->nitems * sizeof(variable_t); i += sizeof(variable_t))
		{
			variable_t	*var = (variable_t *)(v->data + i);
			printf("\tvar %s, 0x%x, %u octet(s)\n", var->name, var->addr, var->size);
		}
	}
}


void			vector_print(vector_t *vec, char *name, void (*print)(const void *))
{
	printf("\n==> %s:\n", name);
	if (vec == NULL)
	{
		puts("NULL\n");
		return;
	}
	for (uint32_t i = 0; i < vec->nitems; i++)
	{
		print(vec->data + (i * vec->elemsize));
	}
	printf("\n");
}

static char	*get_keyword(char *s)
{
	char	*start = s;

	while (*s != '\0' && *s != ' ' && *s != '\t' && *s != '\n')
		s++;
	return (strndup(start, s - start));
}

uint8_t		get_params(char **s, char *param[10])
{
	char	*start;
	uint8_t	nparam = 0;

	for (;;)
	{
		while (is_space(**s)) s++;
		start = *s;
		while (**s != ',' && !is_endl(**s) && **s != ')') (*s)++;
		if (!is_endl(**s))
		{
			char	*end = *s;

			if (nparam == 10)
				return (0xffu); // too many params
			if (end == start)
				return (0xffu); // empty param
			while (is_space(end[-1])) {
				end--;
				if (end == start)
					return (0xffu); // empty param
			}
			param[nparam++] = strndup(start, end - start);
			if (**s == ')')
				break;
			(*s)++;
		}
		if (is_endl(**s))
		{
			while (nparam)
				free(param[--nparam]); // missing ')'
			return (0xffu);
		}
	}
	return (nparam);
}

char	*replace_content(char *content, uint32_t argc, char *param[10])
{
	char		*new = NULL;
	int32_t		diff = 0;
	uint32_t	len[10];
	char		*pos;
	char		*tofree = content;

	for (uint32_t i = 0; i != argc; i++)
	{
		len[i] = strlen(param[i]);
		diff += len[i] - 2;
	}

	new = malloc(strlen(content) + diff + 1);
	new[0] = '\0';
	pos = content;
	while ((pos = strchr(content, '#')))
	{
		register uint8_t	i = pos[1] - '0';

		strncat(new, content, pos - content);
		strncat(new, param[i], len[i]);
		content = pos + 2;
	}
	strcat(new, content);
	free(tofree);
	return (new);
}


/*
 *	A recoder proprement
 */
char	*parse_instruction(char *s, vector_t *area, vector_t *ext_symbol, loc_sym_t *loc_symbol, vector_t *macro, data_t *data)
{
	char		*name = s;
	uint32_t	macro_index = 0xffffffffu;
	char		*macro_param[10] = {NULL};
	uint8_t		n_params = 0;
	char		*content;
	uint32_t	argc;
	char		*search = NULL;

	if (!is_alpha(*s) && *s != '_')
		goto __unexpected_char;
	s++;
	while (is_alnum(*s) || *s == '_') s++;

	// LABEL
	if (*s == ':')
	{
		// start = name
		char *end = s;
		s++;
		if (!is_space(*s) && !is_endl(*s))
			goto __unexpected_char;
		add_label(strndup(name, end - name), area, ext_symbol, loc_symbol, data); //test if not mnemonic
		return (s);
	}
	else if (!is_space(*s) && *s != '(' && !is_endl(*s))
		goto __unexpected_char;
	/*
	if (!is_endl(*s))
		goto __add_instruction;
*/
	search = strndup(name, s - name);
	while (is_space(*s)) s++;
	printf("\e[1;44mvector_search(\"%s\")\e[0m\n", search);
	if ((macro_index = (uint32_t)vector_search(macro, (void*)&search)) != 0xffffffffu)
	{
		char		*content_ptr = NULL;
		data_t		new_data;
		content = VEC_ELEM(macro_t, macro, macro_index)->content;
		argc = VEC_ELEM(macro_t, macro, macro_index)->argc;

		puts("\e[1;44mREPLACE_MACRO\e[0m\n");
		if (*s == '(')
		{
			s++;
			n_params = get_params(&s, macro_param);
			if (n_params == 0xffu)
				goto __unexpected_char;
			if (n_params != argc)
				goto __argc_error;
			if (n_params)
			{
				content = replace_content(content, argc, macro_param);
				content_ptr = content;
			}
			for (uint8_t i = 0; macro_param[i]; i++)
				free(macro_param[i]);
		}
		char	*tmp = content;
		while (!is_space(*tmp) && !is_endl(*tmp)) tmp++;
		if (!is_endl(*tmp))
		{
			while (is_space(*tmp)) tmp++;
			if (!is_endl(*tmp))
				goto __next;
		}
		
		name = (argc) ? content : strdup(content);
		s = add_instruction(name, area, ext_symbol, loc_symbol, macro, s, data);
		free(name);
		return (s);

		
__next:
		new_data.filename = malloc(strlen(data->filename) + strlen(name) + 10);
		sprintf(new_data.filename, "%s in macro %s", data->filename, name);
		new_data.line = content;
		new_data.lineno = 1;
		new_data.cur_area = data->cur_area;


		for (;;)
		{
			char *inst = content;
			while (!is_space(*content) && !is_endl(*content)) content++;
			inst = strndup(inst, content - inst);
			content = add_instruction(inst, area, ext_symbol, loc_symbol, NULL, content, &new_data);
			free(inst);
			if (*content == '\0')
				break;
			content += 1;
			new_data.line = content;
			new_data.lineno++;
		}
		if (content_ptr) free(content_ptr);
		free(new_data.filename);
		free(search);
		return (s);
	}
	else if (*s == '(' && !is_space(s[-1]))
		goto __unexpected_char;

//__add_instruction:
	printf("name = \"%s\"\n", search);
	s = add_instruction(search, area, ext_symbol, loc_symbol, macro, s, data);
	free(search);
	return (s);

__argc_error:
	for (uint8_t i = 0; macro_param[i]; i++)
		free(macro_param[i]);
	sprintf(data->buf, "arguments expected: %hhu, %u given", (uint8_t)argc, n_params);
	goto __print_error;
	

__unexpected_char:
	sprintf(data->buf, "unexpected character `%c`", *s);
	if (search) free(search);
__print_error:
	print_error(data->filename, data->lineno, data->line, data->buf);
	while (!is_endl(*s)) s++;
	return (s);
}

static void		parse_file(char *filename, vector_t *area, vector_t *macro, vector_t *ext_symbol, loc_sym_t *loc_symbol, uint32_t cur_area)
{
	data_t		data;
	char		*s, *content_start;
	char		*include_filename;

	if ((s = get_file_contents(filename, &data.length)) == NULL)
	{
		fprintf(stderr, "%s: file doesn't exist\n", filename);
		exit(1);
	}
	content_start = s;
	s[data.length] = '\0';

	data.filename = filename;
	data.line = s;
	data.lineno = 1;
	data.cur_area = cur_area;

	while (*s)
	{
		SKIP_SPACES(s);

		if (*s == '\n')
		{
			data.lineno++;
			data.line = ++s;
			continue;
		}
		if (*s == '%')
		{
			if (strncmp(s + 1, "define", 6) == 0 && !is_alnum(s[7])) {
				CHECK_ERROR_DIRECTIVE(7);
				s = define_macro(macro, s + 8, &data);
			}
			else if (strncmp(s + 1, "undef", 5) == 0 && !is_alnum(s[6])) {
				CHECK_ERROR_DIRECTIVE(6);
				s = undef_macro(macro, s + 7, &data);
			}
			else if (strncmp(s + 1, "include", 7) == 0 && !is_alnum(s[8]))
			{
				CHECK_ERROR_DIRECTIVE(8);
				s += 9;
				include_filename = get_include_filename(&s, &data);
				parse_file(include_filename, area, macro, ext_symbol, loc_symbol, data.cur_area);
				free(include_filename);
			}
			else
			{
				char *keyword = get_keyword(data.line);
				sprintf(data.buf, "unknown directive `%s`", keyword);
				free(keyword);
				print_error(data.filename, data.lineno, data.line, data.buf);
				while (!is_endl(*s)) s++;
			}
		}
		else if (*s == '.')
		{
			uint32_t	len;

			if (strncmp(s + 1, "bank", 4) == 0 && !is_alnum(s[5]))
				s = bank_switch(area, s + 6, &data);
			else if (strncmp(s + 1, "byte", 4) == 0 && !is_alnum(s[5]))
				s = add_bytes(area, s + 6, &data);
			else if (strncmp(s + 1, "memlock", 7) == 0 && !is_alnum(s[8]))
				s = set_memlock_area(loc_symbol->memblock, s + 9, &data);
			else if (strncmp(s + 1, "var", 3) == 0 && is_numeric(s + 4, &len) && is_space(s[len + 4]))
				s = assign_var_to_memory(loc_symbol, ext_symbol, s + 4, &data);
			else if (strncmp(s + 1, "extern", 6) == 0 && !is_alnum(s[7]))
				s = set_extern_symbol(ext_symbol, s + 8, &data);
			else
			{
				char *keyword = get_keyword(data.line);
				sprintf(data.buf, "unknown keyword `%s`", keyword);
				free(keyword);
				print_error(data.filename, data.lineno, data.line, data.buf);
				while (!is_endl(*s)) s++;
			}
		}
		else
		{
			s = parse_instruction(s, area, ext_symbol, loc_symbol, macro, &data);
		}
	}
	
	puts("");
	vector_print(macro, "macro", &macro_print);
	vector_print(area, "code area", &area_print);
	vector_print(ext_symbol, "extern symbol", &symbol_print);
	vector_print(loc_symbol->label, "label", &label_print);
	vector_print(loc_symbol->memblock, "block", &memblock_print);
	free(content_start);
}

/*
	1ere etape:

	"$"					-> bytes
	"[a-zA-Z0-9_]+"		-> instruction
*/


// --cartridge_spec		mbc, n_ram_banks, n_rom_banks
// --cartridge_name		name
// --japanese_version
// --gameboy_type		{dmg, cdg, both}
// --show-memlock		-> affiche l'espace disponible dans chaque zone de variables
// --show-remaind-mem	-> affiche l'espace disponible dans chaque banque de la ROM

#define	ADD_MACRO(_name, _content, _macro)		new.name = _name;\
											new.content = _content;\
											vector_push(_macro, &new);

vector_t	*set_builtin_macro(void)
{
	vector_t		*macro;
	macro_t			new = {NULL, NULL, 0, 0};

	macro = vector_init(sizeof(macro_t));
	macro->compar = &macro_cmp;
	macro->search = &macro_match;
	macro->destroy = &macro_destroy;
	ADD_MACRO("P1", "0x00", macro);
	ADD_MACRO("SB", "0x01", macro);
	ADD_MACRO("SC", "0x02", macro);
	ADD_MACRO("DIV", "0x04", macro);
	ADD_MACRO("TIMA", "0x05", macro);
	ADD_MACRO("TMA", "0x06", macro);
	ADD_MACRO("TAC", "0x07", macro);
	ADD_MACRO("LCDC", "0x40", macro);
	ADD_MACRO("STAT", "0x41", macro);
	ADD_MACRO("SCY", "0x42", macro);
	ADD_MACRO("SCX", "0x43", macro);
	ADD_MACRO("LY", "0x44", macro);
	ADD_MACRO("LYC", "0x45", macro);
	ADD_MACRO("DMA", "0x46", macro);
	ADD_MACRO("BGP", "0x47", macro);
	ADD_MACRO("OBP0", "0x48", macro);
	ADD_MACRO("OBP1", "0x49", macro);
	ADD_MACRO("WY", "0x4A", macro);
	ADD_MACRO("WX", "0x4B", macro);
	ADD_MACRO("KEY1", "0x4D", macro);
	ADD_MACRO("VBK", "0x4F", macro);
	ADD_MACRO("HDMA1", "0x51", macro);
	ADD_MACRO("HDMA2", "0x52", macro);
	ADD_MACRO("HDMA3", "0x53", macro);
	ADD_MACRO("HDMA4", "0x54", macro);
	ADD_MACRO("HDMA5", "0x55", macro);
	ADD_MACRO("RP", "0x56", macro);
	ADD_MACRO("BCPS", "0x68", macro);
	ADD_MACRO("BCPD", "0x69", macro);
	ADD_MACRO("OCPS", "0x6A", macro);
	ADD_MACRO("OCPD", "0x6B", macro);
	ADD_MACRO("SVBK", "0x70", macro);
	ADD_MACRO("IF", "0x0F", macro);
	ADD_MACRO("IE", "0xFF", macro);
	vector_sort(macro);

	return (macro);
}

char	*get_object_name(const char *s)
{
	const char	*end = s + strlen(s) - 1;

	while (*end != '.')
	{
		if (end == s)
		{
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

	for (uint32_t i = 0; i < 24; i++)
	{
		total += n[i];
	}
	return ((uint8_t)(0x100u - total));
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

void	get_symbols(const char *filename, vector_t *sym)
{
	tmp_header_t	header;
	FILE			*file;
	all_sym_t		tmp;
	char			*buf;

	if ((file = fopen(filename, "r")) == NULL)
	{
		fprintf(stderr, "file %s does not exist\n", filename);
		exit(1);
	}

	if (fread(&header, sizeof header, 1, file) != sizeof header)
		goto __file_error;

	if (header.header_length < header.intern_symbols_length)
		goto __file_error;
	
	buf = malloc(header.intern_symbols_length);
	if (fread(buf, 1, header.intern_symbols_length, file) != header.intern_symbols_length)
	{
		free(buf);
		goto __file_error;
	}

	for (uint32_t i = 0; i < header.intern_symbols_length; i++)
	{
		register char	*name;
		
		name = buf + i;
		while (buf[i]) i++;
		tmp.name = strndup(name, i);
		i++;

		tmp.type = *(uint32_t*)(buf + i);
		i += sizeof(uint32_t);

		switch (tmp.type)
		{
			case LABEL:
			{
				tmp.data1 = *(uint32_t*)(buf + i);
				i += sizeof(uint32_t);
				break;
			}
			case VAR:
			{
				register uint32_t	quantity;

				quantity = *(uint32_t *)(buf + i);
				i += sizeof(uint32_t) * (quantity + 1);
				name = buf + i;
				while (buf[i]) i++;
				tmp.block = strndup(name, i);
				i++;

				tmp.data1 = *(uint32_t*)(buf + i);
				i += sizeof(uint32_t);

				tmp.data2 = 0;
				break;
			}
			case MEMBLOCK:
			{
				tmp.data1 = *(uint32_t*)(buf + i);
				i += sizeof(uint32_t);

				tmp.data2 = *(uint32_t*)(buf + i);
				i += sizeof(uint32_t);
				break;
			}
			default:
				goto __file_error;
		}

		if (sym->nitems == 0)
			vector_push(sym, (void*)&tmp);
		else
			VEC_SORTED_INSERT(sym, tmp.name, tmp);
	}

	free(buf);
	fclose(file);
	return;

__file_error:
	fprintf(stderr, "object file not well formated\n");
	fclose(file);
	exit(1);
}

/*
 *	bloquer la taille maximum des symbols, ...
 */
int		main(int argc, char *argv[])
{
	char			*file[3];
	vector_t		*macro = NULL;
	vector_t		*code_area = NULL;
	vector_t		*extern_symbol = NULL;
	loc_sym_t		local_symbol = {NULL, NULL};
	uint32_t		n_files;

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


	file[0]	= "test.gbs";
	file[1]	= NULL;
	n_files = 1;

	macro = set_builtin_macro();
	code_area = vector_init(sizeof(code_area_t));
	code_area->compar = &area_cmp;
	code_area->destroy = &area_destroy;
	code_area->search = &area_match;
	local_symbol.label = vector_init(sizeof(label_t));
	local_symbol.label->compar = &label_cmp;
	local_symbol.label->search = &label_match;
	local_symbol.label->destroy = &label_destroy;
	local_symbol.memblock = vector_init(sizeof(memblock_t));
	local_symbol.memblock->destroy = &memblock_destroy;
	local_symbol.memblock->compar = &memblock_cmp;
	local_symbol.memblock->search = &memblock_match;
	extern_symbol = vector_init(sizeof(symbol_t));
	extern_symbol->destroy = &ext_symbol_destroy;
	extern_symbol->compar = &ext_symbol_cmp;
	extern_symbol->search = &ext_symbol_match;

	for (char **p = file; *p; p++)
	{
		g_error = 0;
		g_warning = 0;
		// by default, code is writed in address 0x0000
		code_area_t		area_elem = {0, 0, NULL, NULL};
		vector_push(code_area, (void*)&area_elem);

		parse_file(*p, code_area, macro, extern_symbol, &local_symbol, 0);
		check_code_area_overflow(code_area);
		replace_internal_symbols(code_area, &local_symbol);
		if (g_error)
		{
			fprintf(stderr, "\e[1;31m%u errors\e[0m\n", g_error);
			break;
		}

		*p = get_object_name(*p);
		create_object_file(code_area, &local_symbol, extern_symbol, *p);

		vector_reset(code_area);
		vector_reset(local_symbol.label);
		vector_reset(local_symbol.memblock);
		vector_filter(macro, &macro_filter);
	}
	vector_destroy(code_area);
	vector_destroy(macro);
	vector_destroy(local_symbol.label);
	vector_destroy(local_symbol.memblock);

	vector_t	*sym = vector_init(sizeof(all_sym_t));
	vector_t	*code = vector_init(sizeof(all_code_t));

	if (g_error == 0)
	{
		for (uint32_t i = 0; file[i]; i++)
		{
			get_symbols(file[i], sym);
		}
		for (uint32_t i = 0; i < sym->nitems; i++)
		{
			register all_sym_t	*tmp = VEC_ELEM(all_sym_t, sym, i);

			printf("%s (%s) = %u, %u (%s)\n", tmp->name, get_type_str(tmp->type), tmp->data1, tmp->data2, tmp->block);
		}
		for (uint32_t i = 0; file[i]; i++)
			free(file[i]);
	}
	return (0);
}
/*
 *	./prog -c file.gs -o file.go
 *	./prog -o bin.gb file.go
 *	./prog -o bin.gb file.gs   // supprimer les .o
 *
 *
 *
 *	2 programmes ?
 *
 *	./prog -i file.gbs -o file.gbo   ||   ./prog -i file.gbs
 *
 *	./prog2 file.gbo [options]
 *		options = 
 *			--gb		: MGB, CGB	(MGB = CGB = MGL)
 *			--country	: JPN, OTH	(OTH = OTHER)
 *			--start		: (uint16_t)					// program start address
 *			--title		: 
 *			--fill		: (uint8_t)						// unused byte fill character
 */
