/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   disassemble_table.c                              .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/22 20:03:07 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/05/22 20:55:13 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

enum	e_operand_type
{
	NONE, IMM8, IMM16, ADDR8, ADDR16
};

struct	s_strcon
{
	char				*inst;
	enum e_operand_type	operand_type;
};

typedef struct s_strconv	t_strconv;

t_strconv	opcodes[] = {
	{"nop\n", NONE}, //0x00
	{"ld   BC, *\n", IMM16},
	{"ld   (BC), A\n", NONE},
	{"inc  BC\n", NONE},
	{"inc  B\n", NONE},
	{"dec  B\n", NONE},
	{"ld   B, *\n", IMM8},
	{"rlca\n", NONE},
	{"ld   (*), SP\n", ADDR16}, //0x08
	{"add  HL, BC\n", NONE},
	{"ld   A, (BC)\n", NONE},
	{"dec  BC\n", NONE},
	{"inc  C\n", NONE},
	{"dec  C\n", NONE},
	{"ld   C, *\n", IMM8},
	{"rrca\n", NONE},
	{"stop\n", NONE}, //0x10
	{"ld   DE, *\n", IMM16},
	{"ld   (DE), A\n", NONE},
	{"inc  DE\n", NONE},
	{"inc  D\n", NONE},
	{"dec  D\n", NONE},
	{"ld   D, *\n", IMM8},
	{"rla\n", NONE},
	{"jr   *\n", IMM8}, //0x18
	{"add  HL, DE\n", NONE},
	{"ld   A, (DE)\n", NONE},
	{"dec  DE\n", NONE},
	{"inc  E\n", NONE},
	{"dec  E\n", NONE},
	{"ld   E, *\n", IMM8},
	{"rra\n", NONE},
	{"jr   NZ, *\n", IMM8}, //0x20   alt="jrnz   *\n"
	{"ld   HL, *\n", IMM16},
	{"ld   (HL++), A\n", NONE},
	{"inc  HL\n", NONE},
	{"inc  H\n", NONE},
	{"dec  H\n", NONE},
	{"ld   H, *\n", IMM8},
	{"daa\n", NONE},
	{"jr   Z, *\n", IMM8}, //0x28   alt="jrz   *\n"
	{"add  HL, HL\n", NONE},
	{"ld   A, (HL++)\n", NONE},
	{"dec  HL\n", NONE},
	{"inc  L\n", NONE},
	{"dec  L\n", NONE},
	{"ld   L, *\n", IMM8},
	{"cpl\n", NONE},
	{"jr   NC, *\n", IMM8}, //0x30   alt="jrnc   *\n"
	{"ld   SP, *\n", IMM16},
	{"ld   (HL--), A\n", NONE},
	{"inc  SP\n", NONE},
	{"inc  (HL)\n", NONE},
	{"dec  (HL)\n", NONE},
	{"ld   (HL), *\n", IMM8},
	{"scf\n", NONE},
	{"jr   C, *\n", IMM8}, //0x38   alt="jrc   *\n"
	{"add  HL, SP\n", NONE},
	{"ld   A, (HL--)\n", NONE},
	{"dec  SP\n", NONE},
	{"inc  A\n", NONE},
	{"dec  A\n", NONE},
	{"ld   A, *\n", IMM8},
	{"ccf\n", NONE},
	{"", }, //0x40
	{"", },
	{"", },
	{"", },
	{"", },
	{"", },
	{"", },
	{"", },
	{"", },
	{"", },
	{"", },
	{"", },
	{"", },
	{"", },
	{"", },
	{"", },
	{"", },
	{"", },
	{"", },
	{"", },
	{"", },
	{"", },
	{"", },
	{"", },
	{"", },
};
