/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   ram_registers.h                                  .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/24 17:07:32 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/05/24 20:42:37 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

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
//IME register (no addr)

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
# define WY		0xff4aU //window y coordinate
# define WX		0xff4bU //window x coordinate

// OAM REGISTERS
# define OBJ_Y(n)		0xfe00U + ((unsigned int)n << 2) //Y coordinate
# define OBJ_X(n)		0xfe01U + ((unsigned int)n << 2) //X coordinate
# define OBJ_CHR(n)		0xfe02U + ((unsigned int)n << 2) //CHR code
# define OBJ_ATTR(n)	0xfe03U + ((unsigned int)n << 2) //Attributes flags

// DMA REGISTERS
# define DMA	0xff46U //DMA transfer register 
	// (value = starting addr : (uint8_t)(uint16_t >> 8))    for 0xc0000, DMA = 0xc0
	// [ RAM area (0x8000-0xdfff) to OAM (0xfe00-0xfe9f) ]
# define HDMA5	0xff55U //new DMA transfer register (CGB)
	// transfer start + number of bytes to transfer

// LCD COLOR DISPLAY
# define BCPS	0xff68U //specifies a BG write			(CGB)
# define BCPD	0xff69U //specifies the BG write data	(CGB)
# define OCPS	0xff6AU //specifies the OBJ write data	(CGB)
# define OCPD	0xff6bU //OBJ write data				(CGB)

// SOUND 1 REGISTER
# define NR10	0xff10U //frequency + sound color?
# define NR11	0xff11U //sound length?
# define NR12	0xff12U //cresc. decresc. effects? + volume?
# define NR13	0xff13U //???
# define NR14	0xff14U //???

// SOUND 2 REGISTER
# define NR21	0xff16U //frequency + sound color?
# define NR22	0xff17U //Envelope
# define NR23	0xff18U //??
# define NR24	0xff19U //??

// SOUND 3 REGISTER
# define NR30	0xff1aU // stop/enable sound 3
# define NR31	0xff1bU // sound length
# define NR32	0xff1cU // output level
# define NR33	0xff1dU // low-order frequency data
# define NR34	0xff1eU // ???

// SOUND 4 REGISTER
# define NR41	0xff20U // sound length
# define NR42	0xff21U // envelope
# define NR43	0xff22U // ???
# define NR44	0xff23U // ???

// SOUND CONTROL REGISTERS
# define NR50	0xff24U // SO1 output level [3 bits], Vin -> SO1 on/off [1 bit]
						// SO2 output level [3 bits], Vin -> SO2 on/off [1 bit]
# define NR51	0xff25U // Select sound output terminal:
						// sound 4 to SO2 [1 bit], sound 3 to SO2 [1 bit], sound 2 to SO2 [1 bit], sound 1 to SO2 [1 bit], 
						// sound 4 to SO1 [1 bit], sound 3 to SO1 [1 bit], sound 2 to SO1 [1 bit], sound 1 to SO1 [1 bit], 
# define NR52	0xff26U // Disable/enable All sound circuits [1 bit], NULL [3 bits]
						// sound 4 On flag [1 bit], sound 3 On flag [1 bit], sound 2 On flag [1 bit], sound 1 On flag [1 bit], 

/**** BG & OBJ AREA ****/

// BACKGROUND DISPLAY
# define BG_DISPLAY_DATA_1_START	0x9800U
# define BG_DISPLAY_DATA_1_END		0x9bffU
# define BG_DISPLAY_DATA_2_START	0x9c00U
# define BG_DISPLAY_DATA_2_END		0x9fffU
	//(use of data 1 or 2 is determined by bit 3 of the LCDC register)

// OBJECTS DISPLAY (max 40 OBJ : max 10 OBJ per line)
# define OAM_DISPLAY_START			0xfe00U
# define OAM_DISPLAY_END			0xfe9fU
	//OBJ	 =	y [8 bits], x [8 bits], CHR code [8 bits], display priority [1 bit], 
	//			vertical flip [1 bit], horizontal flip [1 bit], DMG-mode palette [1 bit],
	//			character bank [1 bit] (CGB), color palette [3 bits] (CGB).
	//	total = 32 bits

#endif
