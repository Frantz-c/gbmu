#ifndef RAM_REGISTERS_H
# define RAM_REGISTERS_H

// CONTROLLER
# define P1		0xff00U

// DIVIDER REGISTER
# define DIV	0xff04U

// TIME REGISTERS
# define TIMA	0xff05U
# define TMA	0xff06U
# define TAC	0xff07U

// INTERRUPT FLAGS
# define IF		0xff0fU
# define IE		0xffffU
//IME ???

// SERIAL TRANSFER
# define SB		0xff01U
# define SC		0xff02U

// WORKING RAM BANK SWITCH REGISTER (CGB)
# define SVBK	0xff70U

// CPU OPERATING SPEED SWITCH REGISTER (CGB)
# define KEY1	0xff4dU

// INFRARED PORT REGISTER (CGB)
# define RP		0xff56U

// VRAM BANK (CGB)
# define VBK	0xff4fU //mask 0000 0001

// LCD DISPLAY REGISTERS
# define LCDC	0xff40U
# define STAT	0xff41U
# define SCY	0xff42U //scroll y
# define SCX	0xff43U //scroll x
# define LY		0xff44U //lcdc y coordinate
# define LYC	0xff45U //LY compare
# define BCPS	0xff68U //specifies a BG write
# define BCPD	0xff69U //specifies the BG write data
# define OCPS	0xff6AU //specifies the OBJ write data
# define OCPD	0xff6bU //OBJ write data
# define WY		0xff4aU //window y coordinate
# define WX		0xff4bU //window x coordinate

// OAM REGISTERS

/**** AREA ****/

// BACKGROUND DISPLAY
# define BG_DISPLAY_START	0x9800U // or 0x9c00U
# define BG_DISPLAY_END		0x9bffU // or 0x9fffU

#endif
