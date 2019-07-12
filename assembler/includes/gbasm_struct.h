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



struct	operands_s
{
	char				*name;
	struct operands_s	*next;
};

struct	mnemonics_s
{
	char				*name;
	uint32_t			n_operand;
	struct mnemonics_s	*next;
	struct operands_s	*operands;
};

struct	code_area_s
{
	uint32_t			addr;
	struct mnemonics_s	*data;
	struct mnemonics_s	*cur;
};

struct	memblocks_s
{
	uint32_t			start;
	uint32_t			end;
	uint32_t			space;
	char				*name;
	vector_t			*var;
};

struct	variables_s
{
	char				*name;
	uint32_t			addr;
	uint32_t			size;
};

struct	label_s
{
	char		*name;
	uint32_t	offset;	//code_area[offset] = label_position
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
typedef struct operands_s	operands_t;
typedef struct mnemonics_s	mnemonics_t;
typedef struct macro_s		macro_t;
typedef struct code_area_s	code_area_t;
typedef struct label_s		label_t;
typedef struct data_s		data_t;
typedef struct variables_s	variables_t;
typedef struct memblocks_s	memblocks_t;
//typedef struct error_s		error_t;

#endif
