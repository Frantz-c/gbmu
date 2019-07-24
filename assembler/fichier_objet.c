header
{

	uint32_t	header_length;
	uint32_t	extern_symbols_length;
	uint32_t	local_symbols_length;
	uint32_t	code_length;

	// symbols externes
	char		*name;		// nom du symbol
	uint8_t		type;		// var, label, block
	uint32_t	quantite;	// nombre d'appels
	uint32_t	pos;		// position relative au debut du code (apres le header)
	uint32_t	pos2; ...

	// symbols locaux
	char		*name;
	uint8_t		type;
	uint32_t	quantite;
	uint32_t	pos;
	uint32_t	pos2; ...

};

code
{
	// code
}
