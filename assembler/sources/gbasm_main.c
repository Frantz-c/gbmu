/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_main.c                                     .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/11 10:36:42 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/13 23:06:57 by fcordon     ###    #+. /#+    ###.fr     */
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

#define	INPUT	0
#define OUTPUT	1
#define CGB		2
#define DEFINE	3
#define	SKIP_SPACES(ptr)		do { while (*ptr == ' ' || *ptr == '\t') ptr++; } while (0);
#define LOCAL_MEMBLOCK		0
#define GLOBAL_MEMBLOCK		1

const char *const	inst[] = {
	"adc", "add", "and", "bit", "call", "callc", "callnc", "callnz", "callz", "ccf", "cmp", "cp",
	"cpl", "daa", "dec", "di", "ei", "halt", "inc", "jp", "jpc", "jpnc", "jpnz", "jpz", "jr", "jrc",
	"jrnc", "jrnz", "jrz", "ld", "ldd", "ldhl", "ldi", "mov", "nop", "not", "or", "pop", "push",
	"res", "reset", "ret", "retc", "reti", "retnc", "retnz", "retz", "rl", "rla", "rlc", "rlca",
	"rr", "rra", "rrc", "rrca", "rst", "sar", "sbb", "sbc", "scf", "set", "shl", "shr", "sla",
	"sra", "srl", "stop", "sub", "swap", "testb", "xor", NULL
};

uint32_t		g_error = 0;
uint32_t		g_warning = 0;
vector_t		*g_memblock = NULL;

static char	*get_include_filename(char **s, data_t *data)
{
	char	*name;

	while (**s == ' ' || **s == '\t') (*s)++;
	name = *s;
	while (**s != ' ' && **s != '\t' && **s != '\n') (*s)++;
	name = strndup(name, *s - name);
	while (**s == ' ' || **s == '\t') (*s)++;
	if (**s != '\n')
		goto __unexpected_char;
	
	return (name);

__unexpected_char:
	sprintf(data->buf, "unexpected character `%c`", **s);
	print_error(data->filename, data->lineno, data->line, data->buf);
	fprintf(stderr, "\e[1;31m%u errors\e[0m\n", g_error);
	exit(1);
	return (NULL);
}

void			area_print(const void *a)
{
	code_area_t	*area = (code_area_t *)a;

	printf("addr = 0x%04x\n", area->addr);
	if (area->data)
	{
		for (mnemonics_t *m = area->data; m; m = m->next)
		{
			printf("\t\"%s\": ", m->name);
			for (operands_t *o = m->operands; o; o = o->next)
			{
				if (m->name[0] == '$')
					printf("0x%x ", (uint32_t)o->name);
				else
					printf("%s ", o->name);
			}
			printf("\n");
		}
	}
	else
	{
		printf("\tno data\n");
	}
}

char	*get_keyword(char *s)
{
	char	*start = s;

	while (*s != '\0' && *s != ' ' && *s != '\t' && *s != '\n')
		s++;
	return (strndup(start, s - start));
}

void			macro_print(const void *a)
{
	register macro_t	*p = (macro_t *)a;

	printf("%%define %s(%u) %s\n", p->name, p->argc, p->content);
}

void			memblock_print(const void *a)
{
	memblocks_t *b = (memblocks_t *)a;
	vector_t	*v = ((memblocks_t *)a)->var;

	printf("block %s, 0x%x - 0x%x\n", b->name, b->start, b->end);

	if (v)
	{
		for (uint32_t i = 0; i < v->nitems * sizeof(variables_t); i += sizeof(variables_t))
		{
			variables_t	*var = v->data + i;
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

static void		parse_file(char *filename, vector_t *area, vector_t *macro, vector_t *ext_label, vector_t *memblock, int32_t *cur_area)
{
	data_t		data;
	char		*s;
	char		*include_filename;

	if ((s = get_file_contents(filename, &data.length)) == NULL)
	{
		fprintf(stderr, "%s: file doesn't exist\n", filename);
		exit(1);
	}
	s[data.length] = '\0';

	data.filename = filename;
	data.line = s;
	data.lineno = 1;
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
			if (strncmp(s + 1, "define", 6) == 0 && (s[7] == ' ' || s[7] == '\t'))
				s = define_macro(macro, s + 8, &data);
			else if (strncmp(s + 1, "undef", 5) == 0 && (s[6] == ' ' || s[6] == '\t'))
				s = undef_macro(macro, s + 7, &data);
			else if (strncmp(s + 1, "include", 7) == 0 && (s[8] == ' ' || s[8] == '\t'))
			{
				s += 9;
				include_filename = get_include_filename(&s, &data);
				parse_file(include_filename, area, macro, ext_label, memblock, cur_area);
				free(include_filename);
			}
			else
			{
				char *keyword = get_keyword(data.line);
				sprintf(data.buf, "unknown directive `%s`", keyword);
				free(keyword);
				print_error(data.filename, data.lineno, data.line, data.buf);
			}
		}
		else if (*s == '.')
		{
			if (strncmp(s + 1, "bank", 4) == 0 && (s[5] == ' ' || s[5] == '\t'))
				s = bank_switch(area, s + 6, &data);
			else if (strncmp(s + 1, "byte", 4) == 0 && (s[5] == ' ' || s[5] == '\t'))
				s = add_bytes(area, s + 6, &data);
			else if (strncmp(s + 1, "memlock", 7) == 0 && (s[8] == ' ' || s[8] == '\t'))
				s = set_memlock_area(memblock, s + 9, &data);
			else if (strncmp(s + 1, "global_memlock", 14) == 0 && (s[15] == ' ' || s[15] == '\t'))
				s = set_memlock_area(g_memblock, s + 16, &data);
				// .memlock uram_b0, 0xc000, end=0xca00
				// .memlock uram_b0, 0xc000, len=1024
			else if (strncmp(s + 1, "var", 3) == 0 && (s[4] > '0' && s[4] <= '9'))
				s = assign_var_to_memory(*memblock, s + 4);
/*			else if (strncmp(s + 1, "extern", 6) == 0 && (s[7] == ' ' || s[7] == '\t'))
				s = set_external_label_or_memblock(extlab, s + 8, &err);
*/
			else
			{
				char *keyword = get_keyword(data.line);
				sprintf(data.buf, "unknown keyword `%s`", keyword);
				free(keyword);
				print_error(data.filename, data.lineno, data.line, data.buf);
			}
		}
		else
		{
			exit(1);
			//s = add_instruction_or_label(zon, curzon, def, s, &err);
		}
	}
	
	puts("");
	vector_print(macro, "macro", &macro_print);
	vector_print(area, "code area", &area_print);
	//vector_print(ext_label, "label", &label_print);
	vector_print(g_memblock, "global block", &memblock_print);
	vector_print(memblock, "block", &memblock_print);
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

int		main(int argc, char *argv[])
{
	char			*file[3];
	vector_t		*macro = NULL;
	vector_t		*code_area = NULL;
	vector_t		*ext_label = NULL;
	vector_t		*memblock = NULL;
	int32_t			cur_area;
	code_area_t		area_elem = {0, NULL, NULL};

	file[0]	=	"testfile1";
	file[1]	=	"testfile2";
	file[2]	=	NULL;

	macro = set_builtin_macro(); //macro + cmp()
	code_area = vector_init(sizeof(code_area_t));
	code_area->compar = &area_cmp;
	code_area->destroy = &area_destroy;
	code_area->search = &area_match;
	ext_label = vector_init(sizeof(label_t));
	ext_label->compar = &label_cmp;
	ext_label->search = &label_match;
	ext_label->destroy = &label_destroy;
	g_memblock = vector_init(sizeof(memblocks_t));
	g_memblock->destroy = &memblock_destroy;
	g_memblock->compar = &memblock_cmp;
	g_memblock->search = &memblock_match;
	memblock = vector_clone(g_memblock);

	for (char **p = file; *p; p++)
	{
		g_error = 0;
		cur_area = 0;
		vector_push(code_area, (void*)&area_elem);

		parse_file(*p, code_area, macro, ext_label, memblock, &cur_area);
		if (g_error)
		{
			fprintf(stderr, "\e[1;31m%u errors\e[0m\n", g_error);
			return (1);
		}

//		save_file_object(code_area, ext_label, *p);
		vector_reset(code_area);
		vector_reset(ext_label);
		vector_reset(memblock);
		vector_filter(macro, &macro_destroy_allocated);
	}
	vector_destroy(code_area);
	vector_destroy(macro);
	vector_destroy(ext_label);
	vector_destroy(memblock);
	vector_destroy(g_memblock);
	return (0);
}
/*
 *	./prog -c file.gs -o file.go
 *	./prog -o bin.gb file.go
 *	./prog -o bin.gb file.gs   // supprimer les .o
 */
