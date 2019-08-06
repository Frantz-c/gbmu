/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_struct.h                                   .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/08/06 11:38:18 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/06 18:44:29 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef GBASM_STRUCT_H
# define GBASM_STRUCT_H

# include "std_includes.h"

// C_FF00 = (C)
typedef enum	param_e
{
	UNKNOWN,NONE,A,B,C,D,E,F,H,L,AF,BC,DE,HL,SP,_NZ_,_Z_,_NC_,_C_,
	HLI,HLD,SP_ADDR,HL_ADDR,BC_ADDR,DE_ADDR,AF_ADDR,FF00_C,
	SP_IMM8,FF00_IMM8,IMM8,ADDR8,IMM16,ADDR16,SYMBOL
}
param_t;

typedef enum	insn_err_e
{
	ENONE, MISSING_PARAM, TOO_MANY_PARAMS, INVAL_DST, INVAL_SRC, OVERFLOW
}
insn_err_t;

// symbol types
#define UNUSED			0x0
#define	VAR_OR_LABEL	0x01
#define	VAR				0x11
#define LABEL			0x21
#define MEMBLOCK		0x02

#define	NOT_DECLARED	0xffffffffu
#define BYTE_DIRECTIVE	0xffffff00u

#define JR		0x18
#define JRZ		0x28
#define JRNZ	0x20
#define JRC		0x38
#define JRNC	0x30


// final binary generation
typedef struct	tmp_header_s
{
	uint32_t	header_length;
	uint32_t	intern_symbols_length;
	uint32_t	code_length;
}
tmp_header_t;

typedef struct	tmp_label_s
{
	uint32_t	value;
}
tmp_label_t;

typedef struct	tmp_variable_s
{
	uint32_t	quantity;
	uint32_t	pos[];
}
tmp_variable_t;

struct	memblock2_s
{
	uint32_t	start;
	uint32_t	end;
	uint32_t	space;
	uint32_t	garbage;
	char		*name;
}
memblock2_t;

typedef struct	var_data_s
{
	uint32_t	quantity;	
	uint32_t	file_number;
	char		*block;		
	char		*pos;		
	struct var_data_t	*next;
}
var_data_t;

/*
 *	if extern_symbol: 
 *		- data1 -> quantity
 *		- data2 -> unused
 *		- data	-> pos
 */
typedef struct	all_ext_sym_s
{
	char		*name;
	uint32_t	type;
	uint32_t	quantity;
	uint32_t	*pos;
	uint32_t	file_number;
}
all_ext_sym_t;

typedef struct	all_sym_s
{
	char		*name;
	uint32_t	type;
	uint32_t	data1;		// value	if LABEL
							// addr		if VAR
							// start	if MEMBLOCK
	uint32_t	data2;		// size		if VAR
							// end		if MEMBLOCK
	var_data_t	*var_data;
}
all_sym_t;

typedef struct	all_code_s
{
	uint32_t	start;
	uint32_t	end;
	uint8_t		*code;
	uint32_t	file_number;
}
all_code_t;

// object files generation
typedef struct	intern_symbols_s
{
	uint8_t		*name;
	uint32_t	type;
	uint32_t	data1;		// value	if LABEL
							// quantity	if VAR
							// start	if MEMBLOCK
	uint32_t	*pos;		//			if VAR
	uint8_t		*blockname;	//			if VAR
	uint32_t	data2;		// end		if MEMBLOCK
							// size		if VAR
}
intern_symbols_t;

typedef struct	extern_symbols_s
{
	uint8_t		*name;
	uint32_t	type;
	uint32_t	quantity;
	uint32_t	*pos;
}
extern_symbols_t;


// assembly files
typedef struct	cart_info_s
{
	uint8_t		_0x00c3[2];
	uint8_t		start_addr[4];
	uint8_t		title[11];
	uint8_t		game_code[4];
	uint8_t		cgb_support;
	uint8_t		maker_code[2];
	uint8_t		sgb_support;
	uint8_t		game_pack_type;
	uint8_t		rom_size;
	uint8_t		ram_size;
	uint8_t		destination;
	uint8_t		_0x33;	//0x33
	uint8_t		mask_rom_version;
	uint8_t		complement_check;
	uint8_t		check_sum[2];
}
cart_info_t;

typedef struct	instruction_s
{
	const char *const	name;
	const void *const	addr;
}
instruction_t;

typedef struct	param_error_s
{
	uint32_t	p1;
	uint32_t	p2;
}
param_error_t;

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
	uint32_t	line;
	char		*filename;
};

struct	label_s
{
	char		*name;
	uint32_t	pos;
	uint32_t	base_or_status;	// 0xffffffffu = NOT_DECLARED, all others = DECLARED
	uint32_t	line;			// pour les symbols doubles
	char		*filename;
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
	uint32_t		size;
	struct code_s	*data;
	struct code_s	*cur;
};

struct	memblock_s
{
	uint32_t	start;
	uint32_t	end;
	uint32_t	space;
	uint32_t	line;
	char		*name;
	char		*filename;
	vector_t	*var;
};

struct	variable_s
{
	char		*name;
	uint32_t	addr;	// extern memblocks not allowed
	uint32_t	size;
	uint32_t	line;
	char		*filename;
};

struct	data_s
{
//	char		*s;
	char		*line;
	char		*filename;
	uint32_t	length;
	uint32_t	lineno;
	uint32_t	cur_area;
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
