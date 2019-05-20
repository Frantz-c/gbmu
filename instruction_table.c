enum	e_param
{
	A, B, C, D, E, F, H, L,
	SP, PC,
	AF, BC, DE, HL,
	IMM8, IMM16,
	ADDR8, ADDR16,
	FLAG,
	HL_ADDR, BC_ADDR, DE_ADDR,
	C_FF00,
	____
};

struct	s_spec
{
	int			opcode;
	t_param		param1;
	t_param		param2;
	int			cycles;
};

struct	s_inst
{
	const char			*lname; // lowercase
	const char			*uname; // uppercase
	struct s_inst_info	*spec;	// t_spec array
};

typedef struct s_inst	t_inst;
typedef struct s_spec	t_spec;
typedef enum e_param	t_param;

// ADDR16 = little endian order 0xff00 = ($00ff)
// X_FF00 = (X)
//

t_inst	g_cpu_info[] = {
	{"ld", "LD", g_ld_spec},
	{"ldd", "LDD", g_ldd_spec},
	{"ldi", "LDI", g_ldi_spec},
	{"ldhl", "LDHL", g_ldhl_spec},
	{"push", "PUSH", g_push_spec},
	{"pop", "POP", g_pop_spec},
	{"add", "ADD", g_add_spec},
	{"adc", "ADC", g_adc_spec},
	{"sub", "SUB", g_sub_spec},
	{"sbc", "SBC", g_sbc_spec},
	{"and", "AND", g_and_spec},
	{},
	{},
	{},
	{},
	{},
	{},
	{},
	{},
	{},
};

t_spec	g_sbc_spec[] = {
	{0x9f, A, A, 4},
	{0x98, B, 0, 4},
	{0x99, C, 0, 4},
	{0x9a, D, 0, 4},
	{0x9b, E, 0, 4},
	{0x9c, H, 0, 4},
	{0x9d, L, 0, 4},
	{0x9e, HL_ADDR, 0, 8},
	{????, ____, 0, ?},
};


t_spec	g_sub_spec[] = {
	{0x97, A, 0, 4},
	{0x90, B, 0, 4},
	{0x91, C, 0, 4},
	{0x92, D, 0, 4},
	{0x93, E, 0, 4},
	{0x94, H, 0, 4},
	{0x95, L, 0, 4},
	{0x06, HL_ADDR, 0, 8},
	{0xd6, ____, 0, 8},
};

// flags: {Z = set if result is zero; N = reset;
//			H = set if carry from bit 3;
//			C = set if carry of bit 7}
// carry flag to A
t_spec	g_adc_spec[] = {
	{0x8f, A, A, 4},
	{0x88, A, B, 4},
	{0x89, A, C, 4},
	{0x8a, A, D, 4},
	{0x8b, A, E, 4},
	{0x8c, A, H, 4},
	{0x8d, A, L, 4},
	{0x8e, A, HL_ADDR, 8},
	{0xce, A, ____, 8},
};

// flags: {Z = set if result is zero; N = reset;
//			H = set if carry from bit 3;
//			C = set if carry of bit 7}
t_spec	g_add_spec[] = {
	{0x87, A, A, 4},
	{0x80, A, B, 4},
	{0x81, A, C, 4},
	{0x82, A, D, 4},
	{0x83, A, E, 4},
	{0x84, A, H, 4},
	{0x85, A, L, 4},
	{0x86, A, HL_ADDR, 8},
	{0xc6, A, ____, 8},
};

t_spec	g_pop_spec[] = {
	{0xf1, AF, 0, 12},
	{0xc1, BC, 0, 12},
	{0xd1, DE, 0, 12},
	{0xe1, HL, 0, 12},
};

t_spec	g_push_spec[] = {
	{0xf5, AF, 0, 16},
	{0xc5, BC, 0, 16},
	{0xd5, DE, 0, 16},
	{0xe5, HL, 0, 16},
};

t_spec	g_ldhl_spec[] = {
	// lea	(SP+IMM8), HL
	// flags :: {Z, N = reset; H, C = set or reset}
	{0xf8, SP, IMM8, 12},
};

t_spec	g_ldd_spec[] = {
	// load		reg, (addr)
	// dec		addr
	{0x3a, A, HL_ADDR, 8},
	// load		(addr), reg
	//dec		addr
	{0x32, HL_ADDR, A, 8},
};

t_spec	g_ldi_spec[] = {
	// load		reg, addr
	// inc		addr
	{0x2a, A, HL_ADDR, 8},
	// load		addr, reg
	// inc		addr
	{0x22, HL_ADDR, A, 8},
}

t_spec	g_ld_spec[] = {
	// ld	(ADDR16), SP
	{0x08, ADDR16, SP, 20},

	{0xe0, ADDR8, A, 12}, // LD	($FF00+n), A
	{0xf0}, A, ADDR8, 12, // LD A, ($FF00+n)
	// LD	reg16, imm16
	{0x01, BC, IMM16, 12},
	{0x11, DE, IMM16, 12},
	{0x21, HL, IMM16, 12},
	{0x31, SP, IMM16, 12},
	{0xf9, SP, HL, 8},
	// LD	reg, imm8
	{0x06, B, IMM8, 8},
	{0x0e, C, IMM8, 8},
	{0x16, D, IMM8, 8},
	{0x1e, E, IMM8, 8},
	{0x26, H, IMM8, 8},
	{0x2e, L, IMM8, 8},
	// LD	A, x
	{0x7f, A, A, 4},
	{0x78, A, B, 4},
	{0x79, A, C, 4},
	{0x7a, A, D, 4},
	{0x7b, A, E, 4},
	{0x7c, A, H, 4},
	{0x7d, A, L, 4},
	{0x0a, A, BC_ADDR, 8},
	{0x1a, A, DE_ADDR, 8},
	{0x7e, A, HL_ADDR, 8},
	{0xfa, A, ADDR16, 16},
	{0x3e, A, ____, 8},
	{0xf2, A, C_FF00, 8},
	// LD	x, A
	{0x7f, A, A, 4},
	{0x47, B, A, 4},
	{0x4f, C, A, 4},
	{0x57, D, A, 4},
	{0x5f, E, A, 4},
	{0x67, H, A, 4},
	{0x6f, L, A, 4},
	{0x02, BC_ADDR, A, 8},
	{0x12, DE_ADDR, A, 8},
	{0x77, HL_ADDR, A, 8},
	{0xea, ADDR16, A, 16},
	{0xe2, C_FF00, A, 8},
	{0x40, B, B, 4},
	{0x41, B, C, 4},
	{0x42, B, D, 4},
	{0x43, B, E, 4},
	{0x44, B, H, 4},
	{0x45, B, L, 4},
	{0x46, B, HL_ADDR, 8},
	{0x48, C, B, 4},
	{0x49, C, C, 4},
	{0x4a, C, D, 4},
	{0x4b, C, E, 4},
	{0x4c, C, H, 4},
	{0x4d, C, L, 4},
	{0x4e, C, HL_ADDR, 8},
	{0x50, D, B, 4},
	{0x51, D, C, 4},
	{0x52, D, D, 4},
	{0x53, D, E, 4},
	{0x54, D, H, 4},
	{0x55, D, L, 4},
	{0x56, D, HL_ADDR, 8},
	{0x58, E, B, 4},
	{0x59, E, C, 4},
	{0x5a, E, D, 4},
	{0x5b, E, E, 4},
	{0x5c, E, H, 4},
	{0x5d, E, L, 4},
	{0x5e, E, HL_ADDR, 8},
	{0x60, H, B, 4},
	{0x61, H, C, 4},
	{0x62, H, D, 4},
	{0x63, H, E, 4},
	{0x64, H, H, 4},
	{0x65, H, L, 4},
	{0x66, H, HL_ADDR, 8},
	{0x68, L, B, 4},
	{0x69, L, C, 4},
	{0x6a, L, D, 4},
	{0x6b, L, E, 4},
	{0x6c, L, H, 4},
	{0x6d, L, L, 4},
	{0x6e, L, HL_ADDR, 8},
	{0x70, HL_ADDR, B, 8},
	{0x71, HL_ADDR, C, 8},
	{0x72, HL_ADDR, D, 8},
	{0x73, HL_ADDR, E, 8},
	{0x74, HL_ADDR, H, 8},
	{0x75, HL_ADDR, L, 8},
	{0x36, HL_ADDR, IMM8, 12},
	{0x, , , 4},
	{0x, , , 4},
	{0x, , , 4},
	{0x, , , 4},
	{0x, , , 4},
};
