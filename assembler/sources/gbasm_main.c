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

#include "../includes/gbasm_macro.h"
//#include "../includes/gbasm_.h"
//#include "../includes/gbasm_.h"

#define	INPUT	0
#define OUTPUT	1
#define CGB		2
#define DEFINE	3
#define	SKIP_SPACES(ptr)		do { while (*ptr == ' ' || *ptr == '\t') ptr++; } while (0);
#define	SKIP_CHARACTERS(ptr)	do { while (is_authorized(*ptr)) ptr++; } while (0);

int		compile_file(char *filename, zones_t **zon, defines_t *def[])
{
	error_t			err = {0, 0, {0}, {0}};
	char			*file;
	char			*line;
	unsigned int	lineno = 1;
	int				ret_value = 0;
	char			*include_file;

	file = get_file_contents(filename);
	if (file == NULL)
	{
		fprintf(stderr, "%s: file doesn't exist\n", filename);
		exit(1);
	}

	line = file;
	while (file)
	{
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
				file = add_macro(&def, file + 8, &err);
			else if (strncmp(file + 1, "undef", 5) == 0 && (file[6] == ' ' || file[6] == '\t'))
				file = del_macro(&def, file + 7, &err);
			else if (strncmp(file + 1, "include", 7) == 0 && (file[8] == ' ' || file[8] == '\t'))
			{
				include_file = get_include_file(file + 9, &err);
				compile_file(include_file, zon, def);
				free(include_file);
			}
			else
			{
				//error
			}
		}
		else if (*file == '.')
		{
			// .bank		bank, offset
			// .data		byte, ...
			if (strncmp(file + 1, "bank", 4) == 0 && (file[5] == ' ' || file[5] == '\t'))
				switch_bank(zon, file + 6, &err); //supprimer les zones vides au retour de la fonction (et de ses recursives)
			else if (strncmp(file + 1, "byte", 4) == 0 && (file[5] == ' ' || file[5] == '\t'))
				add_bytes(zon, file + 6, &err);
			else
				//error
		}
		else
			file = add_instruction(zon, def, file, &err);

		if (err.error == true)
		{
			ret_value = 1;
			display_error(&err, lineno, line);
			err.error = false;
			err.total = 0;
		}
	}
	return (ret_value);
}

// --cartridge_spec		mbc, n_ram_banks, n_rom_banks
// --cartridge_name		name
// --japanese_version
// --gameboy_type		{dmg, cdg, both}
int		main(int argc, char *argv[])
{
	char	*file[3];
	defines_t	*def[53] = {NULL};
	zones_t		*zon;

	file[0] = "";
	file[1] = "";
	file[2] = NULL;

	for (char **p = file; *p; p++)
	{
		zon = NULL;
		set_builtin_macro(def);
		if (compile_file(*p, &zon, def) == -1)
			return (1);
		save_file_object(zon, *p);
	}
	return (0);
}
/*
 *	./prog -c file.gs -o file.go
 *	./prog -o bin.gb file.go
 *	./prog -o bin.gb file.gs   // supprimer les .o
 */
