
header
{
	uint32_t	header_length;				// header
	uint32_t	cartridge_info_length;		// header
	uint32_t	intern_symbols_length;		// header
	uint32_t	code_length;

	cartridge_info
	{
		uint16_t	info_map;
		info...
	}

	symbols locaux
	{
		char		*name;		// nom du symbol
		uint32_t	type;

		if		(type == LABEL)
		{
			uint32_t	value;	// adresse du symbol
		}
		else if	(type == VARIABLE)
		{
			uint32_t	quantity;	// nombre d'utilisations
			uint32_t	pos[];		// positions des instructions qui l'utilisent
			uint32_t	offset[];	// offset (code start : .bank)
			char		*blockname;	// nom du block contenant la variable
			uint32_t	size;		// taille de la variable
		}
		else if	(type == MEMBLOCK)
		{
			uint32_t	start;		// adresse de depart du block
			uint32_t	end;		// adresse de fin du block
		}
	}

	symbols externes
	{
		char		*name;		// nom du symbol
		uint32_t	type;		// var, label, block
		uint32_t	quantity;	// nombre d'appels
		uint32_t	pos[]; ...	// position relative au debut du code de la valeur (apres le header)
		uint32_t	offset[];	// offset de la portion de code
	}
};



code
{
	uint32_t	start_addr;
	uint32_t	length;
	
	if (.byte)
	{
		unit8_t		0xddU;
		unit8_t		0xddU;
		unit8_t		0xddU;
		uint32_t	n_bytes;
		uint8_t		bytes[];
	}
	else
	{
		uint8_t		byte[n_elem];	// n_elem = 1, 2 ou 4 (1 = pas d'operande, 2 = prefix 0xcb, 3 = jr, 4 = avec operande + calcul potentiel (byte[3]))
									// (byte[3] = '-', '+' ou '\0')
	}
};
(x times)


===========> code detail:


cp		A
ret		NZ
pop		BC
jpnz	0x1234
callnz	ext_label + 0x1234
push	BC
add		A,	0x42
add		A,	ext_label - (0x42 / 2)

jr		0x42
jr		ext_label + 0x42


===== file.o ==== (0x2B = '+', 0x2D = '-')
0xBF
0xC0
0xC1
0xC2 0x34 0x12 0x00
0xC4 0x34 0x12 0x2B
0xC5
0xC6 0x42 0x00
0xC6 0x21 0x2D

0x18 0x42 0x00
0x18 0x42 0x2B
