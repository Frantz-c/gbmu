/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   disassemble_table.c                              .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/22 21:51:26 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/17 17:57:31 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

enum	e_operand_type
{
	NONE, IMM8, IMM16
};

struct	s_strconv
{
	char				*inst;
	enum e_operand_type	optype;
};

typedef struct s_strconv	t_strconv;

/*
 * STOP -> 0x1000 (skip 2 bytes)
 */

t_strconv	opcodes[] = {
	{"nop\n", NONE}, //0x00
	{"ld   BC, 0x%0.4hX\n", IMM16},
	{"ld   [BC], A\n", NONE},
	{"inc  BC\n", NONE},
	{"inc  B\n", NONE},
	{"dec  B\n", NONE},
	{"ld   B, 0x%0.2hhX\n", IMM8},
	{"rlca\n", NONE},
	{"ld   [0x%0.4hX], SP\n", IMM16}, //0x08
	{"add  HL, BC\n", NONE},
	{"ld   A, [BC]\n", NONE},
	{"dec  BC\n", NONE},
	{"inc  C\n", NONE},
	{"dec  C\n", NONE},
	{"ld   C, 0x%0.2hhX\n", IMM8},
	{"rrca\n", NONE},
	{"stop\n", NONE}, //0x10
	{"ld   DE, 0x%0.4hX\n", IMM16},
	{"ld   [DE], A\n", NONE},
	{"inc  DE\n", NONE},
	{"inc  D\n", NONE},
	{"dec  D\n", NONE},
	{"ld   D, 0x%0.2X\n", IMM8},
	{"rla\n", NONE},
	{"jr   %hhd\n", IMM8}, //0x18
	{"add  HL, DE\n", NONE},
	{"ld   A, [DE]\n", NONE},
	{"dec  DE\n", NONE},
	{"inc  E\n", NONE},
	{"dec  E\n", NONE},
	{"ld   E, 0x%0.2hhX\n", IMM8},
	{"rra\n", NONE},
	{"jr   NZ, %hhd\n", IMM8}, //0x20   alt="jrnz   *\n"
	{"ld   HL, 0x%0.4hX\n", IMM16},
	{"ld   [HL++], A\n", NONE},
	{"inc  HL\n", NONE},
	{"inc  H\n", NONE},
	{"dec  H\n", NONE},
	{"ld   H, 0x%0.2hhX\n", IMM8},
	{"daa\n", NONE},
	{"jr   Z, %hhd\n", IMM8}, //0x28   alt="jrz   *\n"
	{"add  HL, HL\n", NONE},
	{"ld   A, [HL++]\n", NONE},
	{"dec  HL\n", NONE},
	{"inc  L\n", NONE},
	{"dec  L\n", NONE},
	{"ld   L, 0x%0.2hhX\n", IMM8},
	{"cpl\n", NONE},
	{"jr   NC, %hhd\n", IMM8}, //0x30   alt="jrnc   *\n"
	{"ld   SP, 0x%0.4hX\n", IMM16},
	{"ld   [HL--], A\n", NONE},
	{"inc  SP\n", NONE},
	{"inc  [HL]\n", NONE},
	{"dec  [HL]\n", NONE},
	{"ld   [HL], 0x%0.2hhX\n", IMM8},
	{"scf\n", NONE},
	{"jr   C, %hhd\n", IMM8}, //0x38   alt="jrc   *\n"
	{"add  HL, SP\n", NONE},
	{"ld   A, [HL--]\n", NONE},
	{"dec  SP\n", NONE},
	{"inc  A\n", NONE},
	{"dec  A\n", NONE},
	{"ld   A, 0x%0.2hhX\n", IMM8},
	{"ccf\n", NONE},
	{"ld   B, B\n", NONE}, //0x40
	{"ld   B, C\n", NONE},
	{"ld   B, D\n", NONE},
	{"ld   B, E\n", NONE},
	{"ld   B, H\n", NONE},
	{"ld   B, L\n", NONE},
	{"ld   B, [HL]\n", NONE},
	{"ld   B, A\n", NONE},
	{"ld   C, B\n", NONE}, //0x48
	{"ld   C, C\n", NONE},
	{"ld   C, D\n", NONE},
	{"ld   C, E\n", NONE},
	{"ld   C, H\n", NONE},
	{"ld   C, L\n", NONE},
	{"ld   C, [HL]\n", NONE},
	{"ld   C, A\n", NONE},
	{"ld   D, B\n", NONE}, //0x50
	{"ld   D, C\n", NONE},
	{"ld   D, D\n", NONE},
	{"ld   D, E\n", NONE},
	{"ld   D, H\n", NONE},
	{"ld   D, L\n", NONE},
	{"ld   D, [HL]\n", NONE},
	{"ld   D, A\n", NONE},
	{"ld   E, B\n", NONE}, //0x58
	{"ld   E, C\n", NONE},
	{"ld   E, D\n", NONE},
	{"ld   E, E\n", NONE},
	{"ld   E, H\n", NONE},
	{"ld   E, L\n", NONE},
	{"ld   E, [HL]\n", NONE},
	{"ld   E, A\n", NONE},
	{"ld   H, B\n", NONE}, //0x60
	{"ld   H, C\n", NONE},
	{"ld   H, D\n", NONE},
	{"ld   H, E\n", NONE},
	{"ld   H, H\n", NONE},
	{"ld   H, L\n", NONE},
	{"ld   H, [HL]\n", NONE},
	{"ld   H, A\n", NONE},
	{"ld   L, B\n", NONE}, //0x68
	{"ld   L, C\n", NONE},
	{"ld   L, D\n", NONE},
	{"ld   L, E\n", NONE},
	{"ld   L, H\n", NONE},
	{"ld   L, L\n", NONE},
	{"ld   L, [HL]\n", NONE},
	{"ld   L, A\n", NONE},
	{"ld   [HL], B\n", NONE}, //0x70
	{"ld   [HL], C\n", NONE},
	{"ld   [HL], D\n", NONE},
	{"ld   [HL], E\n", NONE},
	{"ld   [HL], H\n", NONE},
	{"ld   [HL], L\n", NONE},
	{"halt\n", NONE},
	{"ld   [HL], A\n", NONE},
	{"ld   A, B\n", NONE}, //0x78
	{"ld   A, C\n", NONE},
	{"ld   A, D\n", NONE},
	{"ld   A, E\n", NONE},
	{"ld   A, H\n", NONE},
	{"ld   A, L\n", NONE},
	{"ld   A, [HL]\n", NONE},
	{"ld   A, A\n", NONE},
	{"add  B\n", NONE}, //0x80
	{"add  C\n", NONE},
	{"add  D\n", NONE},
	{"add  E\n", NONE},
	{"add  H\n", NONE},
	{"add  L\n", NONE},
	{"add  [HL]\n", NONE},
	{"add  A\n", NONE},
	{"adc  B\n", NONE}, //0x88
	{"adc  C\n", NONE},
	{"adc  D\n", NONE},
	{"adc  E\n", NONE},
	{"adc  H\n", NONE},
	{"adc  L\n", NONE},
	{"adc  [HL]\n", NONE},
	{"adc  A\n", NONE},
	{"sub  B\n", NONE}, //0x90
	{"sub  C\n", NONE},
	{"sub  D\n", NONE},
	{"sub  E\n", NONE},
	{"sub  H\n", NONE},
	{"sub  L\n", NONE},
	{"sub  [HL]\n", NONE},
	{"sub  A\n", NONE},
	{"sbc  B\n", NONE}, //0x98
	{"sbc  C\n", NONE},
	{"sbc  D\n", NONE},
	{"sbc  E\n", NONE},
	{"sbc  H\n", NONE},
	{"sbc  L\n", NONE},
	{"sbc  [HL]\n", NONE},
	{"sbc  A\n", NONE},
	{"and  B\n", NONE}, //0xa0
	{"and  C\n", NONE},
	{"and  D\n", NONE},
	{"and  E\n", NONE},
	{"and  H\n", NONE},
	{"and  L\n", NONE},
	{"and  [HL]\n", NONE},
	{"and  A\n", NONE},
	{"xor  B\n", NONE}, //0xa8
	{"xor  C\n", NONE},
	{"xor  D\n", NONE},
	{"xor  E\n", NONE},
	{"xor  H\n", NONE},
	{"xor  L\n", NONE},
	{"xor  [HL]\n", NONE},
	{"xor  A\n", NONE},
	{"or   B\n", NONE}, //0xb0
	{"or   C\n", NONE},
	{"or   D\n", NONE},
	{"or   E\n", NONE},
	{"or   H\n", NONE},
	{"or   L\n", NONE},
	{"or   [HL]\n", NONE},
	{"or   A\n", NONE},
	{"cp   B\n", NONE}, //0xb8   alt="cmp   reg\n"
	{"cp   C\n", NONE},
	{"cp   D\n", NONE},
	{"cp   E\n", NONE},
	{"cp   H\n", NONE},
	{"cp   L\n", NONE},
	{"cp   [HL]\n", NONE},
	{"cp   A\n", NONE},
	{"ret  NZ\n", NONE}, //0xc0
	{"pop  BC\n", NONE},
	{"jp   NZ, 0x%0.4hX\n", IMM16}, // alt="jpnz   *\n"
	{"jp   0x%0.4hX\n", IMM16},
	{"call NZ, 0x%0.4hX\n", IMM16}, // alt="callnz   *\n"
	{"push BC\n", NONE},
	{"add  A, 0x%0.2hhX\n", IMM8},
	{"rst  0x00\n", NONE},
	{"ret  Z\n", NONE}, //0xc8   alt="retz\n"
	{"ret\n", NONE},
	{"jp   Z, 0x%0.4hX\n", IMM16}, // alt="jpz   *\n"
	{NULL, 0}, // PREFIX 0xCB
	{"call Z, 0x%0.4hX\n", IMM16}, // alt="callz   *\n"
	{"call 0x%0.4hX\n", IMM16},
	{"adc  A, 0x%0.2hhX\n", IMM8},
	{"rst  0x08\n", NONE},
	{"ret  NC\n", NONE}, //0xd0   alt="retnc\n"
	{"pop  DE\n", NONE},
	{"jp   NC, 0x%0.4hX\n", IMM16}, // alt="jpnc   *\n"
	{NULL, 0}, //unused
	{"call NC, 0x%0.4hX\n", IMM16}, // alt="callnc   *\n"
	{"push DE\n", NONE},
	{"sub  0x%0.2hhX\n", IMM8},
	{"rst  0x10\n", NONE},
	{"ret  C\n", NONE}, //0xd8   alt="retc\n"
	{"reti\n", NONE},
	{"jp   C, 0x%0.4hX\n", IMM16}, // alt="jpc   *\n"
	{NULL, 0}, //unused
	{"call C, 0x%0.4hX\n", IMM16}, // alt="callc   *\n"
	{NULL, 0}, //unused
	{"sbc  A, 0x%0.2hhX\n", IMM8},
	{"rst  0x18\n", NONE},
	{"ld   [0xff00+0x%0.2hX], A\n", IMM8}, //0xe0
	{"pop  HL\n", NONE},
	{"ld   [0xff00+C], A\n", NONE}, /**** 2 bytes ?????  */
	{NULL, 0},
	{NULL, 0},
	{"push HL\n", NONE},
	{"and  0x%0.2hhX\n", IMM8},
	{"rst  0x20\n", NONE},
	{"add  SP, %hhd\n", IMM8}, //0xe8
	{"jp   [HL]\n", NONE},
	{"ld   [0x%0.4hX], A\n", IMM16},
	{NULL, 0},
	{NULL, 0},
	{NULL, 0},
	{"xor  %0.2hhX\n", IMM8},
	{"rst  0x28\n", NONE},
	{"ld   A, [0xff00+0x%0.2hhX]\n", IMM8}, //0xf0
	{"pop  AF\n", NONE},
	{"ld   A, [0xff00+C]\n", NONE}, /**** 2 bytes ?????  */
	{"di\n", NONE},
	{NULL, 0},
	{"push AF\n", NONE},
	{"or   0x%0.2hhX\n", IMM8},
	{"rst  0x30\n", NONE},
	{"ldhl SP, %hhd\n", IMM8}, //0xf8  [== "ld  HL, SP+*"]
	{"ld   SP, HL\n", NONE},
	{"ld   A, [0x%0.4hX]\n", IMM16},
	{"ei\n", NONE},
	{NULL, 0},
	{NULL, 0},
	{"cp   0x%0.2hhX\n", IMM8}, // alt="cmp   *\n"
	{"rst  0x38\n", NONE}
};

t_strconv	cb_opcodes[] = {
	{"rlc  B\n", NONE}, //0x00
	{"rlc  C\n", NONE},
	{"rlc  D\n", NONE},
	{"rlc  E\n", NONE},
	{"rlc  H\n", NONE},
	{"rlc  L\n", NONE},
	{"rlc  (HL)\n", NONE},
	{"rlc  A\n", NONE},
	{"rrc  B\n", NONE}, //0x08
	{"rrc  C\n", NONE},
	{"rrc  D\n", NONE},
	{"rrc  E\n", NONE},
	{"rrc  H\n", NONE},
	{"rrc  L\n", NONE},
	{"rrc  (HL)\n", NONE},
	{"rrc  A\n", NONE},
	{"rl   B\n", NONE}, //0x10
	{"rl   C\n", NONE},
	{"rl   D\n", NONE},
	{"rl   E\n", NONE},
	{"rl   H\n", NONE},
	{"rl   L\n", NONE},
	{"rl   (HL)\n", NONE},
	{"rl   A\n", NONE},
	{"rr   B\n", NONE}, //0x18
	{"rr   C\n", NONE},
	{"rr   D\n", NONE},
	{"rr   E\n", NONE},
	{"rr   H\n", NONE},
	{"rr   L\n", NONE},
	{"rr   (HL)\n", NONE},
	{"rr   A\n", NONE},
	{"sla  B\n", NONE}, //0x20
	{"sla  C\n", NONE},
	{"sla  D\n", NONE},
	{"sla  E\n", NONE},
	{"sla  H\n", NONE},
	{"sla  L\n", NONE},
	{"sla  (HL)\n", NONE},
	{"sla  A\n", NONE},
	{"sra  B\n", NONE}, //0x28
	{"sra  C\n", NONE},
	{"sra  D\n", NONE},
	{"sra  E\n", NONE},
	{"sra  H\n", NONE},
	{"sra  L\n", NONE},
	{"sra  (HL)\n", NONE},
	{"sra  A\n", NONE},
	{"swap B\n", NONE}, //0x30
	{"swap C\n", NONE},
	{"swap D\n", NONE},
	{"swap E\n", NONE},
	{"swap H\n", NONE},
	{"swap L\n", NONE},
	{"swap (HL)\n", NONE},
	{"swap A\n", NONE},
	{"srl  B\n", NONE}, //0x38
	{"srl  C\n", NONE},
	{"srl  D\n", NONE},
	{"srl  E\n", NONE},
	{"srl  H\n", NONE},
	{"srl  L\n", NONE},
	{"srl  (HL)\n", NONE},
	{"srl  A\n", NONE},
	{"bit  0, B\n", NONE}, //0x40
	{"bit  0, C\n", NONE},
	{"bit  0, D\n", NONE},
	{"bit  0, E\n", NONE},
	{"bit  0, H\n", NONE},
	{"bit  0, L\n", NONE},
	{"bit  0, (HL)\n", NONE},
	{"bit  0, A\n", NONE},
	{"bit  1, B\n", NONE}, //0x48
	{"bit  1, C\n", NONE},
	{"bit  1, D\n", NONE},
	{"bit  1, E\n", NONE},
	{"bit  1, H\n", NONE},
	{"bit  1, L\n", NONE},
	{"bit  1, (HL)\n", NONE},
	{"bit  1, A\n", NONE},
	{"bit  2, B\n", NONE}, //0x50
	{"bit  2, C\n", NONE},
	{"bit  2, D\n", NONE},
	{"bit  2, E\n", NONE},
	{"bit  2, H\n", NONE},
	{"bit  2, L\n", NONE},
	{"bit  2, (HL)\n", NONE},
	{"bit  2, A\n", NONE},
	{"bit  3, B\n", NONE}, //0x58
	{"bit  3, C\n", NONE},
	{"bit  3, D\n", NONE},
	{"bit  3, E\n", NONE},
	{"bit  3, H\n", NONE},
	{"bit  3, L\n", NONE},
	{"bit  3, (HL)\n", NONE},
	{"bit  3, A\n", NONE},
	{"bit  4, B\n", NONE}, //0x60
	{"bit  4, C\n", NONE},
	{"bit  4, D\n", NONE},
	{"bit  4, E\n", NONE},
	{"bit  4, H\n", NONE},
	{"bit  4, L\n", NONE},
	{"bit  4, (HL)\n", NONE},
	{"bit  4, A\n", NONE},
	{"bit  5, B\n", NONE}, //0x68
	{"bit  5, C\n", NONE},
	{"bit  5, D\n", NONE},
	{"bit  5, E\n", NONE},
	{"bit  5, H\n", NONE},
	{"bit  5, L\n", NONE},
	{"bit  5, (HL)\n", NONE},
	{"bit  5, A\n", NONE},
	{"bit  6, B\n", NONE}, //0x70
	{"bit  6, C\n", NONE},
	{"bit  6, D\n", NONE},
	{"bit  6, E\n", NONE},
	{"bit  6, H\n", NONE},
	{"bit  6, L\n", NONE},
	{"bit  6, (HL)\n", NONE},
	{"bit  6, A\n", NONE},
	{"bit  7, B\n", NONE}, //0x78
	{"bit  7, C\n", NONE},
	{"bit  7, D\n", NONE},
	{"bit  7, E\n", NONE},
	{"bit  7, H\n", NONE},
	{"bit  7, L\n", NONE},
	{"bit  7, (HL)\n", NONE},
	{"bit  7, A\n", NONE},
	{"res  0, B\n", NONE}, //0x80
	{"res  0, C\n", NONE},
	{"res  0, D\n", NONE},
	{"res  0, E\n", NONE},
	{"res  0, H\n", NONE},
	{"res  0, L\n", NONE},
	{"res  0, (HL)\n", NONE},
	{"res  0, A\n", NONE},
	{"res  1, B\n", NONE}, //0x88
	{"res  1, C\n", NONE},
	{"res  1, D\n", NONE},
	{"res  1, E\n", NONE},
	{"res  1, H\n", NONE},
	{"res  1, L\n", NONE},
	{"res  1, (HL)\n", NONE},
	{"res  1, A\n", NONE},
	{"res  2, B\n", NONE}, //0x90
	{"res  2, C\n", NONE},
	{"res  2, D\n", NONE},
	{"res  2, E\n", NONE},
	{"res  2, H\n", NONE},
	{"res  2, L\n", NONE},
	{"res  2, (HL)\n", NONE},
	{"res  2, A\n", NONE},
	{"res  3, B\n", NONE}, //0x98
	{"res  3, C\n", NONE},
	{"res  3, D\n", NONE},
	{"res  3, E\n", NONE},
	{"res  3, H\n", NONE},
	{"res  3, L\n", NONE},
	{"res  3, (HL)\n", NONE},
	{"res  3, A\n", NONE},
	{"res  4, B\n", NONE}, //0xa0
	{"res  4, C\n", NONE},
	{"res  4, D\n", NONE},
	{"res  4, E\n", NONE},
	{"res  4, H\n", NONE},
	{"res  4, L\n", NONE},
	{"res  4, (HL)\n", NONE},
	{"res  4, A\n", NONE},
	{"res  5, B\n", NONE}, //0xa8
	{"res  5, C\n", NONE},
	{"res  5, D\n", NONE},
	{"res  5, E\n", NONE},
	{"res  5, H\n", NONE},
	{"res  5, L\n", NONE},
	{"res  5, (HL)\n", NONE},
	{"res  5, A\n", NONE},
	{"res  6, B\n", NONE}, //0xb0
	{"res  6, C\n", NONE},
	{"res  6, D\n", NONE},
	{"res  6, E\n", NONE},
	{"res  6, H\n", NONE},
	{"res  6, L\n", NONE},
	{"res  6, (HL)\n", NONE},
	{"res  6, A\n", NONE},
	{"res  7, B\n", NONE}, //0xb8
	{"res  7, C\n", NONE},
	{"res  7, D\n", NONE},
	{"res  7, E\n", NONE},
	{"res  7, H\n", NONE},
	{"res  7, L\n", NONE},
	{"res  7, (HL)\n", NONE},
	{"res  7, A\n", NONE},
	{"set  0, B\n", NONE}, //0xc0
	{"set  0, C\n", NONE},
	{"set  0, D\n", NONE},
	{"set  0, E\n", NONE},
	{"set  0, H\n", NONE},
	{"set  0, L\n", NONE},
	{"set  0, (HL)\n", NONE},
	{"set  0, A\n", NONE},
	{"set  1, B\n", NONE}, //0xc8
	{"set  1, C\n", NONE},
	{"set  1, D\n", NONE},
	{"set  1, E\n", NONE},
	{"set  1, H\n", NONE},
	{"set  1, L\n", NONE},
	{"set  1, (HL)\n", NONE},
	{"set  1, A\n", NONE},
	{"set  2, B\n", NONE}, //0xd0
	{"set  2, C\n", NONE},
	{"set  2, D\n", NONE},
	{"set  2, E\n", NONE},
	{"set  2, H\n", NONE},
	{"set  2, L\n", NONE},
	{"set  2, (HL)\n", NONE},
	{"set  2, A\n", NONE},
	{"set  3, B\n", NONE}, //0xd8
	{"set  3, C\n", NONE},
	{"set  3, D\n", NONE},
	{"set  3, E\n", NONE},
	{"set  3, H\n", NONE},
	{"set  3, L\n", NONE},
	{"set  3, (HL)\n", NONE},
	{"set  3, A\n", NONE},
	{"set  4, B\n", NONE}, //0xe0
	{"set  4, C\n", NONE},
	{"set  4, D\n", NONE},
	{"set  4, E\n", NONE},
	{"set  4, H\n", NONE},
	{"set  4, L\n", NONE},
	{"set  4, (HL)\n", NONE},
	{"set  4, A\n", NONE},
	{"set  5, B\n", NONE}, //0xe8
	{"set  5, C\n", NONE},
	{"set  5, D\n", NONE},
	{"set  5, E\n", NONE},
	{"set  5, H\n", NONE},
	{"set  5, L\n", NONE},
	{"set  5, (HL)\n", NONE},
	{"set  5, A\n", NONE},
	{"set  6, B\n", NONE}, //0xf0
	{"set  6, C\n", NONE},
	{"set  6, D\n", NONE},
	{"set  6, E\n", NONE},
	{"set  6, H\n", NONE},
	{"set  6, L\n", NONE},
	{"set  6, (HL)\n", NONE},
	{"set  6, A\n", NONE},
	{"set  7, B\n", NONE}, //0xf8
	{"set  7, C\n", NONE},
	{"set  7, D\n", NONE},
	{"set  7, E\n", NONE},
	{"set  7, H\n", NONE},
	{"set  7, L\n", NONE},
	{"set  7, (HL)\n", NONE},
	{"set  7, A\n", NONE}
};
