header
{

	uint32_t	header_length;
//	uint32_t	extern_symbols_length;
//	uint32_t	local_symbols_length;
//	uint32_t	code_length;

	symbols externes
	{
		char		*name;		// nom du symbol
		uint32_t	type;		// var, label, block

		if		!block
		uint32_t	quantity;	// nombre d'appels
		uint32_t	pos; ...	// position relative au debut du code de la valeur (apres le header)
	}

	symbols locaux
	{
		char		*name;
		uint8_t		type;

		if		labels
		uint32_t	quantity;
		uint32_t	pos; ...

		else if	variables
		char		*blockname;
		uint32_t	size;
		uint32_t	quantity;
		uint32_t	pos; ...

		else if	block
		uint32_t	start;
		uint32_t	end;
	}
};



uint32_t	n_blocks

code
{
	uint16_t	start_addr;
	uint32_t	length;
	
	uint8_t		byte; ...
	// code
}.time(n_blocks);




/*********** COMPILATION DES .O ************/

recuperation de tous les symbols locaux
