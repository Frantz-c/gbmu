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

struct	defines_s
{
	char		*name;
	char		*content;
	uint32_t	count;
	uint32_t	length;
	struct defines_s	*next;
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
	struct mnemonics_s	*cur;
	struct zones_s		*next;
	struct zones_s		*prev;
};

struct	error_s
{
	uint32_t	error;
	uint32_t	total;
	int			type[5];
	uint32_t	info[5];
};

typedef struct operands_s	operands_t;
typedef struct mnemonics_s	mnemonics_t;
typedef struct defines_s	defines_t;
typedef struct zones_s		zones_t;
typedef struct error_s		error_t;

#define	INPUT	0
#define OUTPUT	1
#define CGB		2
#define DEFINE	3
#define	SKIP_SPACES(ptr)		do { while (*ptr == ' ' || *ptr == '\t') ptr++; } while (0);
#define	SKIP_CHARACTERS(ptr)	do { while (is_authorized(*ptr)) ptr++; } while (0);

void	push_macro(defines_t **def, char *name, char *content, uint32_t count)
{
	defines_t	*new;
	defines_t	*p;
	uint32_t	len = strlen(name);

	new = malloc(sizeof(defines_t));
	new->name = name;
	new->content = content;
	new->count = count;
	new->length = len;
	new->next = NULL;

	if (!*def)
	{
		*def = new;
		return;
	}
	p = *def;
	
	uint32_t	i = 1;
	int32_t		diff;
	defines_t	*prev = NULL;
	while (1)
	{
		if (p->name[i] == name[i])
			i++;
		else if (p->name[i] < name[i])
		{
			if (p->next)
			{
				do {
					prev = p;
					p = p->next;
					for (uint32_t j = 0; j < i; j++)
						diff += p->name[j] - name[j];
					if (diff)
						break;
				}
				while (p->name[i] < name[i] && p->next);
			}
			else
				break;
		}
		else
		{
			p = prev;
			break;
		}
	}
	if (p == NULL)
	{
		p = *def;
		*def = new;
		new->next = p;
	}
	else
	{
		prev = p->next;
		p->next = new;
		new->next = prev;
	}
}

char	*add_macro_without_param(char *name, defines_t **def, char *s, error_t *err)
{
	fprintf(stderr, "MACRO... exit\n");
	exit(1);
	return (s);
}

void		copy_content(char *dest, char *s)
{
	char		*newline = s;

	do
	{
		for (s = newline; (*dest = *s) != '\n'; s++, dest++);
		newline = s + 1;
		for (s--; *s == ' ' || *s == '\t'; s--);
	}
	while (*s == '\\');
}

uint32_t	get_content_length(char *s)
{
	char		*newline = s;
	uint32_t	count = 2;

	do
	{
		count--;
		for (s = newline; *s != '\n'; s++, count++);
		newline = s + 1;
		count++;
		for (s--; *s == ' ' || *s == '\t'; s--);
	}
	while (*s == '\\');

	return (count);
}

/* verifier qu'il n'y a pas de caracteres alpha-numeriques autour de la chaine a remplacer*/
int		string_replace(char *content, char *replace, char number)
{
	char			*chr = content;
	const uint32_t	replace_length = strlen(replace);
	int				count = 0;

	while ((chr = strchr(content, *replace)) != NULL)
	{
		if (strncmp(chr, replace, replace_length) == 0)
		{
			chr[0] = '#';
			if (replace_length < 2)
				memmove(chr, chr + 1, strlen(chr) + 1);
			chr[1] = number;
			chr += 2;
			if (replace_length > 2)
				memmove(chr, chr + replace_length - 2, strlen(chr) + 1);
			count++;
		}
		else
			chr++;
	}
	return (count);
}

/*
	remplacer les parametres :
	macro(toto,__titi__)  ld toto, __titi__          ->  (int param = 2)  macro, ld #0, #1

ex:
	name = "macro( arg1 , arg2 )"
	name = "macro\0 arg1\0, arg2\0)", pos[0] = "arg1\0", pos[1] = "arg2\0"
*/
char	*add_macro_with_param(char *name, defines_t **def, char *s, error_t *err)
{
	char		*to_free = name;
	uint32_t	count = 0;
	uint32_t	cur = 0;
	char		*pos[11] = {NULL};
	char		*content;
	uint32_t	length;

	while (*name != '(') name++;
	do
	{
		if (cur == 10)
		{
			//error
			return (s);
		}
		*name = '\0';
		name++;
		while (*name == ' ' || *name == '\t') name++;
		pos[cur++] = name;

		// check if empty argument
		if (*name == ')' || *name == ',')
		{
			char	*newline = s;

			// error is like this -> fprintf(stderr, "line x: syntax argument x\n> $line", lineno, err.info);
			err->error++;
			err->type[err->total] = 1; //ERR_ARGUMENT;
			err->info[err->total] = count;
			err->total++;
			do
			{
				s = newline;
				while (*s != '\n') s++;
				newline = s + 1;
				s--;
				while (*s == ' ' || *s == '\t') s--;
			}
			while (*s == '\\');
			free(to_free);
			return (newline - 1);
		}
		while (*name != ')' && *name != ',') name++;
		count++;
	}
	while (*name != ')');

	while (*s == ' ' || *s == '\t') s++;
	length = get_content_length(s);
	length *= 2;
	content = malloc(length);
	copy_content(content, s);

	while (cur)
	{
		if (string_replace(content, pos[cur], cur + '0') == 0)
		{
			//WARNING unused argument pos[cur]
			fprintf(stderr, "WARNING: unused argument %s\n", pos[cur]);
		}
		cur--;
	}

	push_macro(def, name, content, count);
	return (s);
}

char	*add_macro(defines_t *def[], char	*s, error_t *err)
{
	char	*name;
	char	*name_copy;
	int		with_param = 0;

	while (*s == ' ' || *s == '\t') s++;
	if (*s == '\n') {
		// choisir l'erreur
		fprintf(stderr, "pas de valeur\n");
		return (s);
	}

	if (*s != '_' && (*s < 'A' || (*s > 'Z' && *s < 'a') || *s > 'z'))
	{
		// choisir l'erreur
		fprintf(stderr, "invalid variable name\n");
		goto __end;
	}
	for (name = s; *s && *s != ' ' && *s != '\t' && *s != '\n'; s++)
	{
		if ((with_param == 0 && *s == '(') || (with_param == 1 && *s == ')'))
			with_param++;
		else if (with_param == 2
				|| (*s != '_' && (*s < '0' || (*s > '9' && *s < 'A') || (*s > 'Z' && *s < 'a') || *s > 'z' || (with_param != 1 && *s == ','))))
		{
			// choisir l'erreur
			fprintf(stderr, "invalid variable name\n");
			goto __end;
		}
	}
	if (with_param == 1)
	{
		fprintf(stderr, "invalid syntax\n");
		// choisir l'erreur
		goto __end;
	}

	name_copy = strndup(name, s - name);
	if (*name >= 'a' && *name <= 'z')
	{
		if (with_param)
			s = add_macro_without_param(name_copy, def + (*name - ('a' + 26)), s, err);
		else
			s = add_macro_with_param(name_copy, def + (*name - ('a' + 26)), s, err);
	}
	else if (*name >= 'A' && *name <= 'Z')
	{
		if (with_param)
			s = add_macro_without_param(name_copy, def + (*name - 'A'), s, err);
		else
			s = add_macro_with_param(name_copy, def + (*name - 'A'), s, err);
	}
	else
	{
		if (with_param)
			s = add_macro_without_param(name_copy, def + 52, s, err);
		else
			s = add_macro_with_param(name_copy, def + 52, s, err);
	}

	__end:
	return (s);
}


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
//	t_cli_arg	*arg;
	defines_t	*def[53] = {NULL};
	zones_t		*zon;

//	arg = get_arguments(argc, argv);

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
