/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   struct.h                                         .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <fcordon@le-101.fr>                +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/08/31 19:36:13 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/31 19:56:41 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef STRUCT_H
# define STRUCT_H

# define UNKNOWN_INSTRUCTION	0xffffffffu

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

// if !INTEGER_TYPE: STRING_TYPE
#define INTEGER_TYPE	0x1
#define STRING_TYPE		0x2
#define BYTE_KEYWORD	0x4
#define ID_STRING_TYPE	0x8		// [a-zA-Z_][a-zA-Z0-9_]*
#define GB_STRING_TYPE	0x10	// gameboy ascii string (>= ' ' && <= '_')
#define DB_QUOTE_STRING	0x20	// "string"

typedef struct	arguments_s
{
	uint32_t	type;		// > 0xff  -> .byte
	void		*value;
}
arguments_t;

# define STATIC_DEBUG		static
# define EXTERN_DEBUG		extern

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

#define IDENTIFIER_MAX_LENGTH	64

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

typedef struct	cartridge_info_s
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
cartridge_info_t;

typedef union	cart_info_u
{
	uint16_t			all;
	cartridge_info_t	member;
}
cart_info_t;

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
	uint8_t		sign;
}
value_t;


typedef struct	macro_s
{
	char		*name;
	char		*content;
	uint32_t	argc;
	uint32_t	allocated;
}
macro_t;

typedef struct	symbol_s
{
	char		*name;
	uint32_t	type;
	uint32_t	line;
	char		*filename;
}
symbol_t;

typedef struct	label_s
{
	char		*name;
	uint32_t	pos;
	uint32_t	base_or_status;	// 0xffffffffu = NOT_DECLARED, all others = DECLARED
	uint32_t	line;			// pour les symbols doubles
	char		*filename;
}
label_t;

typedef struct	code_s
{
	uint8_t			opcode[4];	// opcode + value(2) + sign
			// sign = '+', '-' ou '\0'
			// if (sign) value is the complement
	uint32_t		size;	// instruction size (1 - 3)
			// if .byte, length = ((size & 0xffffff00) >> 8)
	void			*symbol; // symbol   or  .bytes
	struct code_s	*next;
	uint32_t		addr;
}
code_t;

typedef struct	code_area_s
{
	uint32_t		addr;
	uint32_t		size;
	struct code_s	*data;
	struct code_s	*cur;
}
code_area_t;

typedef struct	memblock_s
{
	uint32_t	start;
	uint32_t	end;
	uint32_t	space;
	uint32_t	line;
	char		*name;
	char		*filename;
	vector_t	*var;
}
memblock_t;

typedef struct	variable_s
{
	char		*name;
	uint32_t	addr;	// extern memblocks not allowed
	uint32_t	size;
	uint32_t	line;
	char		*filename;
}
variable_t;

typedef struct	data_s
{
//	char		*s;
	char		*line;
	char		*filename;
	uint32_t	length;
	uint32_t	lineno;
	uint32_t	cur_area;
	char		buf[128];
}
data_t;


#endif
