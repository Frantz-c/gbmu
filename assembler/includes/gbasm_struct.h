#ifndef GBASM_STRUCT_H
# define GBASM_STRUCT_H

# include "std_includes.h"

struct	macro_s
{
	char		*name;
	char		*content;
	uint32_t	argc;
	uint32_t	allocated;
};

struct	symbol_s
{
	char		*name;
	uint32_t	type;
};

struct	label_s
{
	char		*name;
	uint32_t	addr;
};

struct	unkwn_sym_s
{
	char		*name;
	uint32_t	lineno;	// for print an error if symbol
						// is not declared
	char		*filename;
	// next ??
};

struct	code_s
{
	uint8_t			opcode[2];
	uint8_t			operand1[2];
	uint8_t			operand2[2];
	uint8_t			size;	// instruction size (1 - 3)
							// if .byte, 1 - 252 (0xffu - 3)
	uint8_t			ope_size;	// 0xf0 -> ope1
								// 0x0f -> ope2
	void			*unkwn;	// not allocated
							// unkwn_sym_t, uint8_t[]
							// symbol   or  .bytes
	struct code_s	*next;
};

struct	code_area_s
{
	uint32_t		addr;
	struct code_s	*data;
	struct code_s	*cur;
};

struct	memblock_s
{
	uint32_t			start;
	uint32_t			end;
	uint32_t			space;
	char				*name;
	vector_t			*var;
};

struct	variable_s
{
	char				*name;
	uint32_t			addr;
	uint32_t			size;
};

struct	data_s
{
	char		*s;
	char		*line;
	char		*filename;
	uint32_t	length;
	uint32_t	lineno;
	int32_t		cur_area;
	char		buf[128];
};

/*
struct	error_s
{
	uint32_t	error;
	uint32_t	total;
	int32_t		type[5];
	uint32_t	info[5];
};
*/
typedef struct symbol_s		symbol_t;
typedef struct unkwn_sym_s	unkwn_sym_t;
typedef struct code_s		code_t;
typedef struct macro_s		macro_t;
typedef struct code_area_s	code_area_t;
typedef struct label_s		label_t;
typedef struct data_s		data_t;
typedef struct variable_s	variable_t;
typedef struct memblock_s	memblock_t;
//typedef struct error_s		error_t;

#endif
