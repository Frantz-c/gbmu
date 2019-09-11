/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_struct.h                                   .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/08/06 11:38:18 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/11 14:39:29 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef GBASM_STRUCT_H
# define GBASM_STRUCT_H

# include "vector.h"

# define PROGRAM_START	0x1
# define GAME_TITLE		0x2
# define GAME_CODE		0x4
# define CGB_SUPPORT	0x8
# define MAKER_CODE		0x10
# define SGB_SUPPORT	0x20
# define CART_TYPE		0x40
# define ROM_SIZE		0x80
# define RAM_SIZE		0x100
# define DESTINATION	0x200
# define VERSION		0x300

# define NINTENDO_LOGO		"\xCE\xED\x66\x66\xCC\x0D\x00\x0B\x03\x73\x00\x83"\
							"\x00\x0C\x00\x0D\x00\x08\x11\x1F\x88\x89\x00\x0E"\
							"\xDC\xCC\x6E\xE6\xDD\xDD\xD9\x99\xBB\xBB\x67\x63"\
							"\x6E\x0E\xEC\xCC\xDD\xDC\x99\x9F\xBB\xB9\x33\x3E"

# define VEC_ELEM(_struct, _var, _index)	((_struct *)((_var)->data + ((_index) * sizeof(_struct))))
# define VEC_ELEM_LAST(_struct, _var)		((_struct *)((_var)->data + (((_var)->nitems - 1) * sizeof(_struct))))
# define VEC_ELEM_FIRST(_struct, _var)		((_struct *)((_var)->data))

# define VEC_SORTED_INSERT(_vector, _string, _new)	\
{\
	register size_t		vindex = vector_index(_vector, (void*)&_string);\
	vector_insert(_vector, (void*)&_new, vindex);\
}

# define STATIC_DEBUG		static
# define EXTERN_DEBUG		extern

// symbol types
# define UNUSED				0x0
# define	VAR_OR_LABEL	0x01
# define	VAR				0x11
# define LABEL				0x21
# define MEMBLOCK			0x02

#define	NOT_DECLARED	0xffffffffu
#define BYTE_DIRECTIVE	0xffffff00u

#define JR		0x18
#define JRZ		0x28
#define JRNZ	0x20
#define JRC		0x38
#define JRNC	0x30

typedef struct	cart_data_s
{
	uint8_t		_0x00c3[2];
	uint8_t		program_start[2];
	// logo position
	uint8_t		game_title[11];
	uint8_t		game_code[4];
	uint8_t		cgb_support;
	uint8_t		maker_code[2];
	uint8_t		sgb_support;
	uint8_t		cart_type;
	uint8_t		rom_size;
	uint8_t		ram_size;
	uint8_t		destination;
	uint8_t		_0x33;	//0x33
	uint8_t		version;
	uint8_t		complement_check;
	uint8_t		check_sum[2];
}
cart_data_t;

typedef struct	cart_info_s
{
	uint16_t	program_start:1;
	uint16_t	game_title:1;
	uint16_t	game_code:1;
	uint16_t	cgb_support:1;
	uint16_t	maker_code:1;
	uint16_t	sgb_support:1;
	uint16_t	cart_type:1;
	uint16_t	rom_size:1;
	uint16_t	ram_size:1;
	uint16_t	destination:1;
	uint16_t	version:1;
}
cart_info_t;


// final binary generation
typedef struct	tmp_header_s
{
	uint32_t	header_length;
	uint32_t	cart_info_length;
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

typedef struct	memblock_s
{
	uint32_t	start;
	uint32_t	end;
	uint32_t	space;
	uint32_t	garbage;
	char		*name;
}
memblock_t;

typedef struct	var_data_s
{
	uint32_t	quantity;	
	uint32_t	file_number;
	uint32_t	*pos;		
	uint32_t	*offset;		
	struct var_data_s	*next;
}
var_data_t;

typedef struct	ext_sym_s
{
	char		*name;
	var_data_t	*data;
	uint32_t	value;
	uint32_t	type;
}
ext_sym_t;

typedef struct	loc_symbols_s
{
	vector_t	*var;
	vector_t	*block;
	vector_t	*label;
}
loc_symbols_t;

// intern_symbols
typedef struct	loc_sym_s
{
	char		*name;
//	uint32_t	type;
	uint32_t	data1;		// value	if LABEL
							// addr		if VAR
							// start	if MEMBLOCK
	uint32_t	data2;		// size		if VAR
							// end		if MEMBLOCK
	var_data_t	*data;
	char		*blockname;
}
loc_sym_t;

typedef struct	loc_label_s
{
	char		*name;
	uint32_t	value;
}
loc_label_t;

typedef struct	loc_var_s
{
	char		*name;
	uint32_t	addr;
	uint32_t	size;
	var_data_t	*data;
	char		*blockname;
}
loc_var_t;

typedef struct	loc_block_s
{
	char		*name;
	uint32_t	start;
	uint32_t	end;
}
loc_block_t;

typedef struct	all_code_s
{
	uint32_t	start;
	uint32_t	end;
	uint8_t		*code;
	uint32_t	file_number;
}
all_code_t;

/*
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

typedef struct	cart_info_s
{
	uint8_t		_0x00c3[2];
	uint8_t		start_addr[2];
	// logo position
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
*/

// assembly files

typedef struct	instruction_s
{
	const char *const	name;
	const void *const	addr;
}
instruction_t;

/*
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
	uint32_t		addr;
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

struct	error_s
{
	uint32_t	error;
	uint32_t	total;
	int32_t		type[5];
	uint32_t	info[5];
};
typedef struct symbol_s		symbol_t;
typedef struct unkwn_sym_s	unkwn_sym_t;
typedef struct code_s		code_t;
typedef struct macro_s		macro_t;
typedef struct code_area_s	code_area_t;
typedef struct label_s		label_t;
typedef struct data_s		data_t;
typedef struct variable_s	variable_t;
typedef struct memblock_s	memblock_t;
*/
//typedef struct error_s		error_t;

#endif
