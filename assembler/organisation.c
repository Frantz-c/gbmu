1) verification caracteres interdits, decoupage dans une liste chainee,
	remplacement des defines
	exemple:
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
		NULL
	}

	|
	v

	addr = 0x4100
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

struct	defines_s
{
	char	*name;
	char	*content;
};

struct	operands_s
{
	char				*name;
	struct operands_s	*next;
};

struct	mnemonic_s
{
	char				*name;
	unsigned int		n_operand;
	char				*line;
	struct mnemonics_s	*next;
	struct operands_s	*operands;
};

struct	zones_s
{
	unsigned int		addr;
	struct mnemonics_s	*data;
	struct zones_s		*next;
};

typedef struct operands_s	operands_t;
typedef struct mnemonics_s	mnemonics_t;
typedef struct defines_s	defines_t;
typedef struct zones_s		zones_t;

#define	INPUT	0
#define OUTPUT	1
#define CGB		2
#define DEFINE	3

t_cli_arg	*get_arguments(int argc, char *argv[])
{
	t_cli_arg	*arg;
	const t_cli_argdef	argdef[] = {
		{CLI_STRING_ARRAY},
		{CLI_STRING,		(t_clidef)"a.gbc",	"o",	"output",	CLI_NEXT},
		{CLI_BOOL,			(t_clidef)0,		NULL,	"cgb"},
		{CLI_STRING_ARRAY,	(t_clidef)0,		"d",	"define",	CLI_NEXT},
		{CLI_END}
	};

	if ((arg = cli_get(argc, argv, argdef)) == NULL)
	{
		for (t_cli_error *e = g_cli_error; e; e = e->next)
			fprintf(stderr, "\e[0;31merror\e[0m : %s %s\n", e->arg, cli_get_error(e));
		fprintf(stderr, "program exit\n");
		exit(1);
	}
	return (arg);
}

#define	SKIP_SPACES(ptr)		do { while (is_space(*ptr)) ptr++; } while (0);
#define	SKIP_CHARACTERS(ptr)	do { while (is_authorized(*ptr)) ptr++; } while (0);

int		compile_file(char *filename)
{
	zones_t			*zon = NULL;
	defines_t		*def = NULL;
	char			*file;
	char			*ptr;
	char			*line;
	unsigned int	lineno = 1;

	file = get_file_contents(filename);

	while (file)
	{
		line = file;
		SKIP_SPACES(file);

		if (*file == '\n')
			goto increment;
		if (*file == '%')
		{
			// define, undef directives
			file = add_preproc_directive(&def, file);
			goto increment;
		}
		if (*file == '.')
		{
			// .bank			no, offset
			// .cartridge_spec	mbc, n_ram_banks, n_rom_banks
			// .cartridge_name	name
			// .japanese_version
			// .gameboy_type	{dmg, cdg, both}
			// .data			byte, ...
		}

		ptr = file;
		SKIP_CHARACTERS(file);
		if (*file != '\n' && !is_space(*file))
			display_error(line, lineno)

	increment:
		while (*file != '\n' && *file != '\0') file++;
	}

	save_file_object(zon);
	return (0);
}

int		main(int argc, char *argv[])
{
	t_cli_arg	*arg;

	arg = get_arguments(argc, argv);

	for (char **p = arg[INPUT].val.sa; *p; p++)
	{
		if (compile_file(*p) == -1)
			return (1);
	}
	return (0);
}
