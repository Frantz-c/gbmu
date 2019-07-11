/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_main.c                                     .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/11 10:36:42 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/11 19:16:51 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */


#include "std_includes.h"
#include "gbasm_tools.h"
#include "gbasm_macro.h"
#include "gbasm_keywords.h"
#include "gbasm_add_instruction.h"

#define	INPUT	0
#define OUTPUT	1
#define CGB		2
#define DEFINE	3
#define	SKIP_SPACES(ptr)		do { while (*ptr == ' ' || *ptr == '\t') ptr++; } while (0);


#define FILE_MAX_LENGTH	0x800000u


const char *const	inst[] = {
	"adc", "add", "and", "bit", "call", "callc", "callnc", "callnz", "callz", "ccf", "cmp", "cp",
	"cpl", "daa", "dec", "di", "ei", "halt", "inc", "jp", "jpc", "jpnc", "jpnz", "jpz", "jr", "jrc",
	"jrnc", "jrnz", "jrz", "ld", "ldd", "ldhl", "ldi", "mov", "nop", "not", "or", "pop", "push",
	"res", "reset", "ret", "retc", "reti", "retnc", "retnz", "retz", "rl", "rla", "rlc", "rlca",
	"rr", "rra", "rrc", "rrca", "rst", "sar", "sbb", "sbc", "scf", "set", "shl", "shr", "sla",
	"sra", "srl", "stop", "sub", "swap", "testb", "xor", NULL
};

uint32_t		g_error = 0;
t_memblocks		*g_memblock = NULL;
t_vector		*g_defines = NULL;

static char	*get_include_filename(char **s, error_t *err)
{
	char	*name;

	while (**s == ' ' || **s == '\t') (*s)++;
	name = *s;
	while (**s != ' ' && **s != '\t' && **s != '\n') (*s)++;
	name = strndup(name, *s - name);
	while (**s == ' ' || **s == '\t') (*s)++;
	if (**s != '\n')
	{
		err->error++;
		fprintf(stderr, "error #3\n");
		while (**s != '\n') (*s)++;
	}
	return (name);
}

void			print_warning(char *filename, uint32_t lineno, char *line, char *error)
{
	char	*p = line;

	g_error++;
	while (*p && *p != '\n') p++;
	if (p - line > 70)
		p = line + 70;

	fprintf
	(
		stderr,
		"\e[1;33m[WARNING]\e[0m \e[1min file \e[0;33m\"%s\"\e[0m\e[1m:\t"
		"line %u: \e[0;33m%s\e[0m\n\t"
		"\e[1;35m> \e[0m%.*s",
		filename,
		lineno, error,
		(int)(p - line), line
	);
}

/*
 *	format d'erreur:
 *
 *		in file "myfile.gs":	l-231: Bad token '$'
 *			> %include "file$"
 */
void			print_error(char *filename, uint32_t lineno, char *line, char *error)
{
	char	*p = line;

	g_error++;
	while (*p && *p != '\n') p++;
	if (p - line > 70)
		p = line + 70;

	fprintf
	(
		stderr,
		"\e[1;31m[ERROR]\e[0m \e[1min file \e[0;33m\"%s\"\e[0m\e[1m:\t"
		"line %u: \e[0;31m%s\e[0m\n\t"
		"\e[1;31m> \e[0m%.*s",
		filename,
		lineno, error,
		(int)(p - line), line
	);
}

char	*get_keyword(char *s)
{
	char	*start = s;

	while (*s != '\0' && *s != ' ' && *s != '\t' && *s != '\n')
		s++;
	return (strndup(start, s - start));
}

static void		parse_file(char *filename, vector_t *area, vector_t *macro, vector_t *ext_label, vector_t *memblock)
{
	data_t		data;
	uint32_t	len;
	char		*s;
	char		*include_filename;

	if ((s = get_file_contents(filename, &len)) == NULL)
	{
		fprintf(stderr, "%s: file doesn't exist\n", filename);
		exit(1);
	}
	s[data.len] = '\0';

	data.filename = filename;
	data.line = s;
	data.lineno = 1;
	while (*s)
	{
		SKIP_SPACES(s);

		if (*s == '\n')
		{
			data.lineno++;
			line = ++s;
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
				include_filename = get_include_filename(&s, &err);
				compile_file(include_filename, zon, curzon, def, memblock);
				free(include_filename);
			}
			else
			{
				keyword = get_keyword(data.line);
				sprintf(data.buf, "Unknown directive `%s`", keyword);
				free(keyword);
				print_error(data->filename, data->lineno, data->line, data->buf);
			}
		}
		else if (*s == '.')
		{
			// .bank		bank, offset
			// .data		byte, ...

			if (strncmp(s + 1, "bank", 4) == 0 && (s[5] == ' ' || s[5] == '\t'))
				s = bank_switch(area, s + 6, &data);
			else if (strncmp(s + 1, "byte", 4) == 0 && (s[5] == ' ' || s[5] == '\t'))
				s = add_bytes(area, s + 6, &data);
			else if (strncmp(s + 1, "memlock", 7) == 0 && (s[8] == ' ' || s[8] == '\t'))
				s = set_memlock_area(memblock, s + 9, &err);
				// .memlock uram_b0, 0xc000, end=0xca00
				// .memlock uram_b0, 0xc000, len=1024
			else if (strncmp(s + 1, "var", 3) == 0 && (s[4] >= '0' && s[4] <= '9'))
				s = get_var_mem_space(*memblock, s + 4);
			else if (strncmp(s + 1, "extern", 6) == 0 && (s[7] == ' ' || s[7] == '\t'))
				s = set_external_label_or_memblock(extlab, s + 8, &err);
//			else
				//error

		}
		else
		{
//			if (!*zon)
//				create_default_bank(zon);
			s = add_instruction_or_label(zon, curzon, def, s, &err);
		}

		if (err.error)
		{
			ret_value = -1;
//			display_error(&err, data.lineno, line);
			err.total = 0;
		}
	}

	// STRUCTURE DES DEFINES
	defines_t **p = def;
	defines_t **end = def + 53;
	while (p != end)
	{
		if (*p)
		{
			defines_t *p2 = *p;
			do
			{
				printf("name = \"%s\" -> content = \"%s\"\n\n", p2->name, p2->content);
				p2 = p2->next;
			}
			while (p2);
		}
		p++;
	}

	// STRUCTURE DES INSTRUCTIONS
	zones_t *z = *zon;
	while (z)
	{
		printf(">> zone start -> %x\n", z->addr);
		mnemonics_t *m = z->data;
		while (m)
		{
			printf("  \"%s\" -> ", m->name);
			operands_t *o = m->operands;
			int			byte_kw = (m->name[0] == '$') ? 1 : 0;
			while (o)
			{
				if (byte_kw == 0)
					printf("%s, ", o->name);
				else
					printf("%hhx, ", (uint8_t)o->name);
				o = o->next;
			}
			printf("\n");
			m = m->next;
		}
		z = z->next;
	}


	if (ret_value) {
		fprintf(stderr, "%u errors\n", err.error);
		return (-1);
	}
	return (0);
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

void	destroy_memblock(void *a)
{
	variables_t	*v;

	free(((memblocks_t *)a)->name);
	v = (((memblocks_t *)a)->var);

	while (v)
	{
		variables_t	*tmp = v;
		v = v->next;
		free(tmp->name);
		free(tmp);
	}
}

void	destroy_label(void *a)
{
	free(((label_t *)a)->name);
}

void	destroy_macro(void *a)
{
	macro_t	*macro = (macro_t *)a;

	if (a->allocated)
	{
		free(a->name);
		free(a->content);
	}
}

void	destroy_area(void *a)
{
	mnemonics_t	*m = ((code_area_t*)a)->data;

	while (m)
	{
		operands_t	*o = m->operands;
		while (o)
		{
			operands_t	*tmp = o;
			o = o->next;
			free(tmp->name);
			free(tmp);
		}
		mnemonics_t *tmp = m;
		m = m->next;
		free(m->name);
		free(m);
	}
}

int		string_cmp(const void *a, const void *b)
{
	return (strcmp(*(char**)a, *(char**)b));
}

int		area_cmp(const void *a, const void *b)
{
	if (((code_area_t *)a)->start > ((code_area_t *)b)->star)
		return (1);
	else if (((code_area_t *)a)->start == ((code_area_t *)b)->star)
		return (0);
	return (-1);
}

int		macro_cmp(const void *a, const void *b)
{
	return (strcmp(((macro_t *)a)->name, ((macro_t *)b)->name));
}

#define	ADD_MACRO(name, content)	new->name = name;\
									new->content = content;\
									new->argc = 0;\
									new->allocated = 0;\
									vector_push(g_macro, &new);

void	set_builtin_macro(void)
{
	macro_t		new;

	g_macro = vector_init(sizeof(macro_t));
	g_macro->compar = &macro_cmp;
	ADD_MACRO("P1", "0x00");
	ADD_MACRO("SB", "0x01");
	ADD_MACRO("SC", "0x02");
	ADD_MACRO("DIV", "0x04");
	ADD_MACRO("TIMA", "0x05");
	ADD_MACRO("TMA", "0x06");
	ADD_MACRO("TAC", "0x07");
	ADD_MACRO("LCDC", "0x40");
	ADD_MACRO("STAT", "0x41");
	ADD_MACRO("SCY", "0x42");
	ADD_MACRO("SCX", "0x43");
	ADD_MACRO("LY", "0x44");
	ADD_MACRO("LYC", "0x45");
	ADD_MACRO("DMA", "0x46");
	ADD_MACRO("BGP", "0x47");
	ADD_MACRO("OBP0", "0x48");
	ADD_MACRO("OBP1", "0x49");
	ADD_MACRO("WY", "0x4A");
	ADD_MACRO("WX", "0x4B");
	ADD_MACRO("KEY1", "0x4D");
	ADD_MACRO("VBK", "0x4F");
	ADD_MACRO("HDMA1", "0x51");
	ADD_MACRO("HDMA2", "0x52");
	ADD_MACRO("HDMA3", "0x53");
	ADD_MACRO("HDMA4", "0x54");
	ADD_MACRO("HDMA5", "0x55");
	ADD_MACRO("RP", "0x56");
	ADD_MACRO("BCPS", "0x68");
	ADD_MACRO("BCPD", "0x69");
	ADD_MACRO("OCPS", "0x6A");
	ADD_MACRO("OCPD", "0x6B");
	ADD_MACRO("SVBK", "0x70");
	ADD_MACRO("IF", "0x0F");
	ADD_MACRO("IE", "0xFF");
	
}

int		main(int argc, char *argv[])
{
	char			*file[3];
	vector_t		*macro = NULL;
	vector_t		*code_area = NULL;
	vector_t		*ext_label = NULL;
	vector_t		*memblock = NULL;
	int32_t			cur_area;

	file[0]	=	"testfile1";
	file[1]	=	"testfile2";
	file[2]	=	NULL;

	set_builtin_macro(); //macro + cmp()
	code_area = vector_init(sizeof(code_area_t));
	code_area->compar = &area_cmp;
	ext_label = vector_init(sizeof(char*));
	ext_label->compar = &string_cmp;
	g_memblock = vector_init(sizeof(memblocks_t));
	memblock = vector_init(sizeof(memblocks_t));

	for (char **p = file; *p; p++)
	{
		g_error = 0;
		cur_area = 0;
		macro = vector_clone(g_macro);

		parse_file(*p, code_area, macro, ext_label, memblock, &cur_area);
		if (g_error)
		{
			fprintf(stderr, "\e[1;31m%u errors\e[0m\n", g_error);
			return (1);
		}

//		save_file_object(code_area, ext_label, *p);
		vector_reset_callback(code_area, &destroy_area);
		vector_reset_callback(ext_label, &destroy_label);
		vector_reset_callback(memblock, &destroy_memblock);
		vector_destroy_callback(macro, &destroy_macro);
	}
	vector_destroy_callback(code_area, &destroy_area);
	vector_destroy(g_macro);
	return (0);
}
/*
 *	./prog -c file.gs -o file.go
 *	./prog -o bin.gb file.go
 *	./prog -o bin.gb file.gs   // supprimer les .o
 */
