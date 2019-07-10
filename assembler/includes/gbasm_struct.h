#ifndef GBASM_STRUCT_H
# define GBASM_STRUCT_H

# include "std_includes.h"

struct	defines_s
{
	char		*name;
	char		*content;
	uint32_t	count;
	uint32_t	length;
	struct defines_s	*next;
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

struct	zones_s
{
	uint32_t			addr;
	struct mnemonics_s	*data;
	struct mnemonics_s	*cur;
	struct zones_s		*next;
	struct zones_s		*prev;
};

struct	error_s
{
	uint32_t	error;
	uint32_t	total;
	int32_t		type[5];
	uint32_t	info[5];
};

typedef struct operands_s	operands_t;
typedef struct mnemonics_s	mnemonics_t;
typedef struct defines_s	defines_t;
typedef struct zones_s		zones_t;
typedef struct error_s		error_t;

#endif
