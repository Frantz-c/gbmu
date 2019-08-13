

cartridge_info_section
{
	uint32_t	cartridge_info_length;

	{
		uint8_t		addr;	//0x0 - 0x50
		uint8_t		info[];	//create translation table
	}
};


header
{
	uint32_t	header_length;
	uint32_t	intern_symbols_length;
	uint32_t	code_length;

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
	}
};



//uint32_t	n_blocks

code
{
	uint32_t	start_addr;
	uint32_t	length;
	
	if (.byte)
	{
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




/*********** COMPILATION DES .O ************/

