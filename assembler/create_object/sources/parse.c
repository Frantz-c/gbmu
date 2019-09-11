/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   parse.c                                          .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/08/13 14:04:54 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/11 15:22:13 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "macro_func.h"
#include "macro.h"
#include "keywords.h"
#include "parse_instruction.h"


/* ###################################
** ######### DEBUG FUNCTIONS #########
** ###################################
*/
STATIC_DEBUG void			header_print(void)
{
	puts("==> cartridge:");
	printf("program_start = 0x%hhx%hhx\n", cartridge.program_start[1], cartridge.program_start[0]);
	printf("game_title    = \"%.11s\"\n", cartridge.game_title);
	printf("game_code     = \"%.4s\"\n", cartridge.game_code);
	printf("cgb_support   = 0x%x\n", cartridge.cgb_support);
	printf("maker_code    = \"%.2s\"\n", cartridge.maker_code);
	printf("sgb_support   = 0x%x\n", cartridge.sgb_support);
	printf("cart_type     = 0x%x\n", cartridge.cart_type);
	printf("rom_size      = 0x%x\n", cartridge.rom_size);
	printf("ram_size      = 0x%x\n", cartridge.ram_size);
	printf("destination   = 0x%x\n", cartridge.destination);
	printf("version       = 0x%x\n\n", cartridge.version);
}

STATIC_DEBUG void			area_print(const void *a)
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
				uint32_t	size = (c->size >> 8);
				printf("byte size = %u\n", size);
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

STATIC_DEBUG void			symbol_print(const void *a)
{
	symbol_t	*s = (symbol_t *)a;

	printf("\t%s\n", s->name);
}

STATIC_DEBUG void			label_print(const void *a)
{
	label_t	*l = (label_t *)a;

	printf("\t%s\n", l->name);
}

STATIC_DEBUG void			macro_print(const void *a)
{
	register macro_t	*p = (macro_t *)a;

	printf("%%define %s(%u) %s\n", p->name, p->argc, p->content);
}

STATIC_DEBUG void			memblock_print(const void *a)
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

STATIC_DEBUG void			vector_print(vector_t *vec, char *name, void (*print)(const void *))
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
/* #######################################
** ######### END DEBUG FUNCTIONS #########
** #######################################
*/

static int	__attribute__((always_inline))
file_included(const char *file)
{
	for (uint32_t i = 0; included_list[i]; i++)
	{
		printf("CMP(\"%s\", \"%s\")\n", included_list[i], file);
		if (strcmp(included_list[i], file) == 0)
			return (1);
	}
	return (0);
}


// prendre en charge les doubles quotes
// ajouter le chemin du fichier passé en argument
static char	*get_include_filename(char **s, data_t *data)
{
	char			*fullname;
//	char			*name;
	uint32_t		i = base_length;
	const uint32_t	max_length = base_length + 512; // include argument max length = 512

	fullname = malloc(max_length);
	if (base)
		strncpy(fullname, base, base_length);
	printf("base = \"%s\"\n", base);
	if (**s != '"')
	{
		while (!is_space(**s) && !is_endl(**s))
		{
			if (**s == '\\')
			{
				s++;
				if (is_endl(**s))
					goto __unexpected_char_endl;
				if (**s == 't')
					fullname[i++] = '\t';
				else
					fullname[i++] = **s;
			}
			else
				fullname[i++] = **s;
			if (i == max_length)
				goto __too_long_file_name;
			(*s)++;
		}
	}
	else
	{
		(*s)++;
		while (1)
		{
			if (**s == '"') {
				(*s)++;
				break;
			}
			if (is_endl(**s))
				goto __missing_double_quotes;
			if (**s == '\\')
			{
				(*s)++;
				if (is_endl(**s))
					goto __unexpected_char_endl;
			}

			if (i == max_length)
				goto __too_long_file_name;
			fullname[i++] = **s;

			(*s)++;
		}
	}
	if (i == base_length)
		goto __empty_file_name;
	while (is_space(**s)) (*s)++;
	if (!is_endl(**s))
		goto __unexpected_char;
	fullname[i] = 0;
	return (fullname);

__empty_file_name:
	print_error(data->filename, data->lineno, data->line, "empty file name");
	return (NULL);
__too_long_file_name:
	print_error(data->filename, data->lineno, data->line, "too long file name");
	return (NULL);
__missing_double_quotes:
	print_error(data->filename, data->lineno, data->line, "missing double quotes at end of string");
	return (NULL);
__unexpected_char_endl:
	print_error(data->filename, data->lineno, data->line, "unexpected character at end of string");
	return (NULL);
__unexpected_char:
	sprintf(data->buf, "(#0) unexpected character `%c`", **s);
__print_error:
	print_error(data->filename, data->lineno, data->line, data->buf);
	free(fullname);
	return (NULL);
}

/*
**	refaire le systeme de types
*/
static __attribute__((always_inline))
uint32_t	get_keywords_and_arguments(char *keyword_start, char **s, arguments_t args[5], data_t *data, vector_t *area)
{
	char		*arg_start;
	uint32_t	length;

	if (!is_alpha(**s))
		goto __unexpected_char;
	keyword_start = (*s)++;

	while (is_alnum(**s) || **s == '_') (*s)++;
	length = *s - keyword_start;

	// .ascii : creer une fonction
	if (length == 5 && strncmp(keyword_start, "ascii", length) == 0)
	{
		uint32_t	byte;
		uint32_t	n_bytes = 0;

		if (n_bytes == 0)
		{
			register code_area_t	*tmp = VEC_ELEM(code_area_t, area, data->cur_area);
			if (tmp->cur == NULL)
			{
				tmp->cur = calloc(1, sizeof(code_area_t));
				tmp->data = tmp->cur;
			}
			else
			{
				tmp->cur->next = calloc(1, sizeof(code_area_t));
				tmp->cur = tmp->cur->next;
			}
			tmp->cur->symbol = (void *)malloc(sizeof(uint8_t) * /*BYTE_ALLOC_SIZE*/ 8);
		}

		while (is_space(**s)) (*s)++;
		if (**s != '"')
			goto __unexpected_charX;
		(*s)++;

		while (1)
		{
			if (**s == '"')
			{
				(*s)++;
				while (is_space(**s)) (*s)++;
				if (!is_endl(**s))
					goto __unexpected_charX;
				break;
			}
			if (is_endl(**s))
				goto __unexpected_charX;

			if (**s == '\\')
			{
				(*s)++;
				if (**s == 'n')
					byte = '\n';
				else if (**s == 't')
					byte = '\t';
				else if (**s >= '0' && **s <= '9')
					byte = **s - '0';
				else
					byte = **s;
			}
			else
				byte = **s;

			if (n_bytes > 0xffffu)
				goto __too_many_bytesX;

			register code_area_t	*tmp = VEC_ELEM(code_area_t, area, data->cur_area);
			push_byte(tmp, byte & 0xffu);
			n_bytes++;
			(*s)++;
		}
		printf("NBYTES = %u\n", n_bytes);
		if (n_bytes == 0)
			print_warning(data->filename, data->lineno, data->line, "0 bytes specified");
		else
			VEC_ELEM(code_area_t, area, data->cur_area)->cur->size <<= 8;


		args[0].type = BYTE_KEYWORD;
		return (0);

	__too_many_bytesX:
		print_error(data->filename, data->lineno, data->line, "more than 0xffffff bytes");
		return (0);
	__unexpected_charX:
		sprintf(data->buf, "unexpected character `%c`", **s);
		print_error(data->filename, data->lineno, data->line, data->buf);
		while (!is_endl(**s)) (*s)++;
		return (0);
	}
	// .byte end


	// .byte : creer une fonction
	if (length == 4 && strncmp(keyword_start, "byte", length) == 0)
	{
		uint32_t	byte;
		uint32_t	len;
		uint32_t	n_bytes = 0;

		if (n_bytes == 0)
		{
			register code_area_t	*tmp = VEC_ELEM(code_area_t, area, data->cur_area);
			if (tmp->cur == NULL)
			{
				tmp->cur = calloc(1, sizeof(code_area_t));
				tmp->data = tmp->cur;
			}
			else
			{
				tmp->cur->next = calloc(1, sizeof(code_area_t));
				tmp->cur = tmp->cur->next;
			}
			tmp->cur->symbol = (void *)malloc(sizeof(uint8_t) * /*BYTE_ALLOC_SIZE*/ 8);
		}

		while (1)
		{
			while (is_space(**s)) (*s)++;
			if (is_endl(**s))
				break;
			if (is_numeric_len(*s, NULL) == 0)
				goto __unexpected_char2;
			byte = atou_len(*s, &len);
			if (byte > 0xffu)
			{
				sprintf(data->buf, "overflow (%.*s): value truncated", len, *s);
				print_warning(data->filename, data->lineno, data->line, data->buf);
			}
			*s += len;
			if (n_bytes > 0xffffu)
				goto __too_many_bytes;

			// mettre dans push_byte()
			register code_area_t	*tmp = VEC_ELEM(code_area_t, area, data->cur_area);
			push_byte(tmp, byte & 0xffu);
			n_bytes++;

			while (is_space(**s)) (*s)++;
			if (**s == '\\')
			{
				(*s)++;
				// skip spaces after backslash
				while (1)
				{
					if (**s == '\n') {
						(*s)++;
						data->lineno++;
						break;
					}
					if (**s == '\0')
						goto __break_break;
					if (!is_space(**s))
						goto __unexpected_char2;
					(*s)++;
				}
			}
			else if (**s == ',')
				(*s)++;
			else if (is_endl(**s)) {
			__break_break:
				break;
			}
		}
		printf("NBYTES = %u\n", n_bytes);
		if (n_bytes == 0)
			print_warning(data->filename, data->lineno, data->line, "0 bytes specified");
		else
			VEC_ELEM(code_area_t, area, data->cur_area)->cur->size <<= 8;


		args[0].type = BYTE_KEYWORD;
		return (0);

	__too_many_bytes:
		print_error(data->filename, data->lineno, data->line, "more than 0xffffff bytes");
		return (0);
	__unexpected_char2:
		sprintf(data->buf, "unexpected character `%c`", **s);
		print_error(data->filename, data->lineno, data->line, data->buf);
		skip_macro(s, &data->lineno);
		return (0);
	}
	// .byte end

	uint8_t i = 0;
	while (1)
	{
		uint8_t		minus = 0;

		while (is_space(**s)) (*s)++;
		if (**s == '-')
		{
			minus++;
			(*s)++;
		}

		if (is_numeric_len(*s, NULL) != 0)
		{
			if (minus)
				goto __signed_not_expected;
			args[i].type = INTEGER_TYPE;
			args[i].value = malloc(sizeof(uint32_t));
			*(uint32_t*)(args[i].value) = atou_inc(s);
		}
		else if (**s == '"') // indirect string (ex: "string space")
		{
			args[i].type = (GB_STRING_TYPE | DB_QUOTE_STRING | STRING_TYPE | ID_STRING_TYPE);

			arg_start = ++(*s);
			if (!is_alpha(**s) && **s != '_')
				args[i].type &= ~(ID_STRING_TYPE);

			while (!is_endl(**s) && **s != '"')
			{
				if (**s == '\\')
					(*s)++;
				if (**s < ' ' || **s > '_')
					args[i].type &= ~(GB_STRING_TYPE);
				if (!is_alnum(**s) && **s != '_')
					args[i].type &= ~(ID_STRING_TYPE);
				(*s)++;
			}

			if (**s != '"')
				goto __unexpected_char;
			if (*s - arg_start > IDENTIFIER_MAX_LENGTH)
				goto __too_long_argument;
			args[i].value = (void*)strndup(arg_start, *s - arg_start);
			(*s)++;
		}
		else // direct string (ex: string\ space)
		{
			args[i].type = (GB_STRING_TYPE | STRING_TYPE | ID_STRING_TYPE);

			arg_start = *s;
			if (!is_alpha(**s) && **s != '_')
				args[i].type &= ~(ID_STRING_TYPE);

			while (!is_endl(**s) && !is_space(**s) && **s != ',')
			{
				if (**s == '\\')
				{
					(*s)++;
					if (is_endl(**s)) break;
				}
				if (**s < ' ' || **s > '_')
					args[i].type &= ~(GB_STRING_TYPE);
				if (!is_alnum(**s) && **s != '_')
					args[i].type &= ~(ID_STRING_TYPE);
				(*s)++;
			}

			if (*s - arg_start > IDENTIFIER_MAX_LENGTH)
				goto __too_long_argument;
			args[i].value = (void*)strndup(arg_start, *s - arg_start);
		}

		while (is_space(**s)) (*s)++;
		i++;
		if (is_endl(**s))
			break;
		if (i == 4)
			break;
		if (**s != ',')
			goto __unexpected_char;
		(*s)++;
	}
	args[i].value = NULL;
	printf("params = ");
	for (uint8_t i = 0; args[i].value; i++)
	{
		if (args[i].type & STRING_TYPE)
			printf("\"%s\" ", (char *)args[i].value);
		else
			printf("%u ", *(uint32_t *)args[i].value);
	}
	printf("\n");
	return (length);

/*
__too_many_arguments:
	sprintf(data->buf, "too many arguments");
	goto __print_error_and_free_all;
*/
__signed_not_expected:
	sprintf(data->buf, "signed numbers forbidden in keywords arguments");
	goto __print_error_and_free_all;
__not_gameboy_ascii:
	sprintf(data->buf, "not gameboy ascii character `%c`", **s);
	goto __print_error_and_free_all;
__too_long_argument:
	sprintf(data->buf, "too long argument (argument %u)", i + 1);
	goto __print_error_and_free_all;
__unexpected_char:
	sprintf(data->buf, "unexpected character `%c`", **s);

__print_error_and_free_all:
	for (uint8_t i = 0; args[i].value; i++)
		free(args[i].value);
	print_error(data->filename, data->lineno, data->line, data->buf);
	while (!is_endl(**s)) (*s)++;
	return (0);
}

/*
** /!\ end_word = pointer after the spaces
*/
#define	PREPROCESSOR_DIRECTIVES()	\
do{\
	register char	*end_word = ++s;\
	while (!is_endl(*end_word) && !is_space(*end_word))\
		end_word++;\
\
	register uint32_t	cmp_len = end_word - s;\
\
	while (is_space(*end_word))\
		end_word++;\
	if (is_endl(*end_word))\
	{\
		print_error(data.filename, data.lineno, data.line, "directive without arguments");\
		goto __end_directive;\
	}\
\
	if		(cmp_len == 6 && strncmp(s, "define", cmp_len) == 0)\
		s = define_macro(macro, end_word, &data);\
	else if (cmp_len == 5 && strncmp(s, "undef", cmp_len) == 0)\
		s = undef_macro(macro, end_word, &data);\
	else if (cmp_len == 2 && strncmp(s, "if", cmp_len) == 0)\
	{\
		fprintf(stderr, "Preprocessor conditions not implemented\n");\
		exit(1);\
	}\
	else if (cmp_len == 7 && strncmp(s, "include", cmp_len) == 0)\
	{\
		s = end_word;\
		include_filename = get_include_filename(&s, &data);\
		printf("INCLUDE \"%s\"\n", include_filename);\
		if (file_included(include_filename))\
		{\
			puts("FILE ALREADY INCLUDED");\
			goto __free_filename;\
		}\
		if (parse_file(include_filename, area, macro, ext_symbol, loc_symbol, data.cur_area) == -1)\
		{\
			sprintf(data.buf, "can't open included file `%s`", include_filename);\
			print_error(data.filename, data.lineno, data.line, data.buf);\
		}\
	__free_filename:\
		free(include_filename);\
	}\
	else\
	{\
		sprintf(data.buf, "unknown directive `%%%.*s`", cmp_len, s);\
		print_error(data.filename, data.lineno, data.line, data.buf);\
		while (!is_endl(*s)) s++;\
	}\
}while(0)\

// integrer le systeme de double syntax de chaines ("string", string) + ajout du type GB_STRING_TYPE
#define KEYWORDS()	\
do {\
	arguments_t	args[4] = {{0, NULL}};\
	char		*keyword = ++s;\
	uint32_t	keyword_len;\
\
	if (!is_alnum(*s)) {\
		print_error(data.filename, data.lineno, data.line, "empty keyword");\
		goto __end_keyword;\
	}\
\
	keyword_len = get_keywords_and_arguments(keyword, &s, args, &data, area);\
	if (keyword_len == 0 || args->type == BYTE_KEYWORD)\
		goto __end_keyword;\
\
	uint32_t	len;\
	uint32_t	type;\
	printf("keyword = \"%.*s\"\n", keyword_len, keyword);\
\
	if (keyword_len == 4 && strncmp(keyword, "bank", keyword_len) == 0)\
		bank_switch(area, args, &data);\
	else if (keyword_len == 7 && strncmp(keyword, "memlock", keyword_len) == 0)\
		set_memlock_area(loc_symbol->memblock, args, &data);\
	else if (strncmp(keyword, "var", 3) == 0 && (type = is_numeric(keyword + 3, &len)) && is_space(keyword[len + 3]) && len == (keyword_len - 3))\
		assign_var_to_memory(loc_symbol, ext_symbol, atou_type(keyword + 3, NULL, type), args, &data);\
	else if (keyword_len == 6 && strncmp(keyword, "extern", keyword_len) == 0)\
		set_extern_symbol(ext_symbol, args, &data);\
	else if (keyword_len == 13 && strncmp(keyword, "program_start", keyword_len) == 0)\
		set_program_start(args, &data);\
	else if (keyword_len == 10 && strncmp(keyword, "game_title", keyword_len) == 0)\
		set_game_title(args, &data);\
	else if (keyword_len == 9 && strncmp(keyword, "game_code", keyword_len) == 0)\
		set_game_code(args, &data);\
	else if (keyword_len == 11 && strncmp(keyword, "cgb_support", keyword_len) == 0)\
		set_cgb_support(args, &data);\
	else if (keyword_len == 10 && strncmp(keyword, "maker_code", keyword_len) == 0)\
		set_maker_code(args, &data);\
	else if (keyword_len == 11 && strncmp(keyword, "sgb_support", keyword_len) == 0)\
		set_sgb_support(args, &data);\
	else if (keyword_len == 9 && strncmp(keyword, "cart_type", keyword_len) == 0)\
		set_cartridge_type(args, &data);\
	else if (keyword_len == 8 && strncmp(keyword, "rom_size", keyword_len) == 0)\
		set_rom_size(args, &data);\
	else if (keyword_len == 8 && strncmp(keyword, "ram_size", keyword_len) == 0)\
		set_ram_size(args, &data);\
	else if (keyword_len == 11 && strncmp(keyword, "destination", keyword_len) == 0)\
		set_code_dest(args, &data);\
	else if (keyword_len == 7 && strncmp(keyword, "version", keyword_len) == 0)\
		set_version(args, &data);\
	else\
	{\
		sprintf(data.buf, "unknown keyword `.%.*s`", keyword_len, keyword);\
		print_error(data.filename, data.lineno, data.line, data.buf);\
	}\
}while(0)\


extern int		parse_file(char *filename, vector_t *area, vector_t *macro, vector_t *ext_symbol, loc_sym_t *loc_symbol, uint32_t cur_area)
{
	data_t		data;
	char		*s, *content_start;
	char		*include_filename;

	if ((s = get_file_contents(filename, &data.length)) == NULL)
		return (-1);
	content_start = s;
	s[data.length] = '\0';

	data.filename = filename;
	data.line = s;
	data.lineno = 1;
	data.cur_area = cur_area;

	included_list[included_index++] = filename;

	while (*s)
	{
		while (is_space(*s)) s++;

		if (*s == '\0')
			break;

		if (*s == '\n') {
			data.lineno++;
			data.line = ++s;
			continue;
		}


		if (*s == '%')
			PREPROCESSOR_DIRECTIVES();
		else if (*s == '.')
			KEYWORDS();
		else
			/* add instruction or label to [area] and
			** symbol reference to ext_symbol or loc_symbol if any
			** (parse_instruction.c)
			*/
			s = parse_instruction(s, area, ext_symbol, loc_symbol, macro, &data);

__end_keyword:
__end_directive:
		continue;
	}
	
	/* degug start */
	puts("");
	vector_print(macro, "macro", &macro_print);
	vector_print(area, "code area", &area_print);
	vector_print(ext_symbol, "extern symbol", &symbol_print);
	vector_print(loc_symbol->label, "label", &label_print);
	vector_print(loc_symbol->memblock, "block", &memblock_print);
	header_print();
	/* debug end */

	free(content_start);
	included_list[--included_index] = NULL;
	return (0);
}
