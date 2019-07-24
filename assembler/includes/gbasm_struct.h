#ifndef GBASM_STRUCT_H
# define GBASM_STRUCT_H

# include "std_includes.h"

// C_FF00 = (C)
typedef enum	param_e
{
	UNKNOWN,NONE,A,B,C,D,E,F,H,L,AF,BC,DE,HL,SP,_NZ_,_Z_,_NC_,_C_,HLI,HLD,SP_ADDR,HL_ADDR,BC_ADDR,DE_ADDR,AF_ADDR,FF00_C,FF00_IMM8,IMM8,ADDR8,IMM16,ADDR16,SYMBOL
}
param_t;

// symbol types
#define UNUSED			0x0
#define	VAR_OR_LABEL	0x01
#define	VAR				0x11
#define LABEL			0x21
#define MEMBLOCK		0x02

typedef struct	loc_sym_s
{
	vector_t	*memblock;
	vector_t	*label;
}
loc_sym_t;

typedef struct	value_s
{
	uint32_t	value;
	uint16_t	is_signed;
	uint8_t		sign;
}
value_t;


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

struct	code_s
{
	uint8_t			opcode[4];	// opcode + value(2) + sign
			// sign = '+', '-' ou '\0'
			// if (sign) value is the complement
	uint32_t		size;	// instruction size (1 - 3)
			// if .byte, length = ((size & 0xffffff00) >> 8)
	void			*symbol; // symbol   or  .bytes
	struct code_s	*next;
};

struct	code_area_s
{
	uint32_t		addr;
	uint32_t		count;
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
//	char		*s;
	char		*line;
	char		*filename;
	uint32_t	length;
	uint32_t	lineno;
	int32_t		cur_area;
	char		buf[128];
	uint32_t	inst_count;
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
