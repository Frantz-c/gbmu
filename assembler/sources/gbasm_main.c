/*
````````````````````````````````````````````````````````````````````
	.bank	1, 0

	ld	A, 0x12
	ld	(0x2000), A
	call	0x5000
	jr		-0x06

	%define	LD_CART_REG_1(reg)	ld (0x2000), reg

label:
	LD_CART_REG_1(A)
	ret
	.byte	0 3 67

	.bank	2, 0x100

	call	0x00 ; mais ou va-t-on ?

````````````````````````````````````````````````````````````````````
traduction en strucure
````````````````````````````````````````````````````````````````````

	addr = 0x0000
	{
		"ld"				-> "A"			-> "0x12"	-> NULL
		|
		v
		"ld"				-> "(0x2000)"	-> "A"		-> NULL
		|
		v
		"call"				-> "0x5000"		-> NULL
		|
		v
		"jr"				-> "-0x06"		-> NULL
		|
		v
		"label:"			-> NULL
		|
		v
		"ld (0x2000), A"	-> NULL
		|
		v
		"ret"				-> NULL
		|
		v
		"&"					-> "0"			-> "3"		-> "67"		-> NULL
		NULL
	}

	|
	v

	addr = 0x8100
	{
		"call"				-> "0x0"		-> NULL
	}

	|
	v

	NULL
``````````````````````````````````````````````````````````````````

fichiers objets:
``````````````````````````````````````````````````````````````````
en-tete:

	extern_label_header_addr(unsigned int)

	addr_header_length(unsigned short) --> (1 = sizeof(unsigned int) * 2)
	gb_addr(unsigned int), obj_addr(unsigned int),
	...

	extern_label_header_length(unsigned short)

	label_name(null terminated string), sizeof_operand(unsigned char) = 1 or 2, operand_addr(unsigned int)
	...

	code binaire.
*/

#include "std_includes.h"
#include "gbasm_macro.h"
#include "gbasm_keywords.h"
#include "gbasm_add_instruction.h"

#define	INPUT	0
#define OUTPUT	1
#define CGB		2
#define DEFINE	3
#define	SKIP_SPACES(ptr)		do { while (*ptr == ' ' || *ptr == '\t') ptr++; } while (0);
#define	SKIP_CHARACTERS(ptr)	do { while (is_authorized(*ptr)) ptr++; } while (0);


#define FILE_MAX_LENGTH	0x800000u

void	*get_file_contents(const char *path, uint32_t *length)
{
	void		*content;
	FILE		*f;

	f = fopen(path, "r");
	if (f == NULL)
		return (NULL);
	fseek(f, 0, SEEK_END);
	*length = (uint32_t)ftell(f);
	rewind(f);
	if (*length == 0)
	{
		fprintf(stderr, "Empty file\n");
		return (NULL);
	}
	if (*length > FILE_MAX_LENGTH)
	{
		fprintf(stderr, "Too Heavy file\n");
		return (NULL);
	}
	content = valloc(*length);
	if (content == NULL)
	{
		perror("allocation failed");
		return (NULL);
	}
	if (fread(content, 1, *length, f) != *length)
	{
		perror("read failed");
		free(content);
		return (NULL);
	}
	fclose(f);
	return (content);
}

char	*get_include_filename(char **s, error_t *err)
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

/*
	incrementer lineno dans les macros multilignes et le mot clé .byte
*/
int		compile_file(char *filename, zones_t **zon, zones_t **curzon, defines_t *def[])
{
	error_t		err = {0, 0, {0}, {0}};
	char		*file;
	char		*line;
	uint32_t	lineno = 1;
	uint32_t	length;
	int			ret_value = 0;
	char		*include_filename;

	file = get_file_contents(filename, &length);
	if (file == NULL)
	{
		fprintf(stderr, "%s: file doesn't exist\n", filename);
		exit(1);
	}
	file[length] = '\0';

	line = file;
	while (*file)
	{
		//printf("file = \e[0;33m\"%s\"\e[0m\n\n", file);
		SKIP_SPACES(file);

		if (*file == '\n')
		{
			lineno++;
			file++;
			line = file;
			continue;
		}
		if (*file == '%')
		{
			// define, undef, include  directives
			if (strncmp(file + 1, "define", 6) == 0 && (file[7] == ' ' || file[7] == '\t'))
				file = define_macro(def, file + 8, &err);
			else if (strncmp(file + 1, "undef", 5) == 0 && (file[6] == ' ' || file[6] == '\t'))
				file = undef_macro(def, file + 7, &err);
			else if (strncmp(file + 1, "include", 7) == 0 && (file[8] == ' ' || file[8] == '\t'))
			{
				file += 9;
				include_filename = get_include_filename(&file, &err);
				compile_file(include_filename, zon, curzon, def);
				free(include_filename);
			}
//			else
//			{
				//error
//			}
		}
		else if (*file == '.')
		{
			// .bank		bank, offset
			// .data		byte, ...

			if (strncmp(file + 1, "bank", 4) == 0 && (file[5] == ' ' || file[5] == '\t'))
				file = bank_switch(zon, curzon, file + 6, &err);
			else if (strncmp(file + 1, "byte", 4) == 0 && (file[5] == ' ' || file[5] == '\t'))
				file = add_bytes(*curzon, file + 6, &err);
			else if (strncmp(file + 1, "memlock", 7) == 0 && (file[8] == ' ' || file[8] == '\t'))
				file = set_data_block(data_block, file + 9, &err);
				// .memlock 0xc000 end=0xca00
				// .memlock 0xc000 len=1024
//			else
				//error

		}
		else
		{
//			if (!*zon)
//				create_default_bank(zon);
			file = add_instruction_or_label(zon, curzon, def, file, &err);
		}

		if (err.error)
		{
			ret_value = -1;
//			display_error(&err, lineno, line);
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

void	reset_macro(defines_t *macro[])
{
	defines_t	**end = macro + 53;
	defines_t	*prev;

	while (macro != end)
	{
		if (*macro)
		{
			defines_t *p2 = *macro;
			do
			{
				prev = p2;
				p2 = p2->next;
				free(prev->name);
				free(prev);
			}
			while (p2);
			*macro = NULL;
		}
		macro++;
	}
}

// --cartridge_spec		mbc, n_ram_banks, n_rom_banks
// --cartridge_name		name
// --japanese_version
// --gameboy_type		{dmg, cdg, both}
int		main(int argc, char *argv[])
{
	char			*file[3];
	defines_t		*def[53] = {NULL};
	zones_t			*zon = NULL, *curzon = NULL;

	file[0]	=	"testfile1";
	file[1]	=	"testfile2";
	file[2]	=	NULL;

	for (char **p = file; *p; p++)
	{
//		set_builtin_macro();
		if (compile_file(*p, &zon, &curzon, def) == -1) {
			return (1);
		}
//		save_file_object(zon, *p);
		reset_macro(def);
//		reset_zon(&zon);
	}
	return (0);
}
/*
 *	./prog -c file.gs -o file.go
 *	./prog -o bin.gb file.go
 *	./prog -o bin.gb file.gs   // supprimer les .o
 */
