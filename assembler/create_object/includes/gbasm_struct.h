/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_struct.h                                   .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/08/06 11:38:18 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/13 15:19:05 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef GBASM_STRUCT_H
# define GBASM_STRUCT_H

# include "std_includes.h"

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
}
macro_t;

struct	symbol_s
{
	char		*name;
	uint32_t	type;
	uint32_t	line;
	char		*filename;
}
symbol_t;

struct	label_s
{
	char		*name;
	uint32_t	pos;
	uint32_t	base_or_status;	// 0xffffffffu = NOT_DECLARED, all others = DECLARED
	uint32_t	line;			// pour les symbols doubles
	char		*filename;
}
label_t;

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
}
code_t;

struct	code_area_s
{
	uint32_t		addr;
	uint32_t		size;
	struct code_s	*data;
	struct code_s	*cur;
}
code_area_t;

struct	memblock_s
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

struct	variable_s
{
	char		*name;
	uint32_t	addr;	// extern memblocks not allowed
	uint32_t	size;
	uint32_t	line;
	char		*filename;
}
variable_t;

struct	data_s
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
