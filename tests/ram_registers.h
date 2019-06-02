/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   ram_registers.h                                  .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/24 17:07:32 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/05/30 11:05:36 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef RAM_REGISTERS_H
# define RAM_REGISTERS_H

// http://www.devrs.com/gb/files/faqs.html#GBBugs






/*******************
**** INTERRUPTS ****
*******************/
# define VBLANK_INT		0x0040U
# define LCDC_INT		0x0048U
# define TIMAOVF_INT	0x0050U
# define SERIAL_INT		0x0058U
# define JOYPAD_INT		0x0060U

/* INTERRUPT AUTOMATIC PROCESS

	1) initialisation du registre IF (liste des interruptions)
	2) Si le flag IME est a 1 et le flag du registre IE correspondant
		a l'interruption est a 1 aussi, on passe a l'etape 3.
	3) Mise a zero du flag IME.
	4) PUSH PC.
	5) Jump a l'adresse de l'interruption

*/










/*****************
*** CONTROLLER ***
*****************/
# define P1		0xff00U // [..BD3210]
/*
	B = bouton (A, B, select, start)
	D = direction (droite, gauche, haut, bas)
	3210 = bit correspondant au bouton.
				0			1			2			3
			B =	A,			B,			select,		start
			D =	droite,		gauche,		haut,		bas

utilisation:
	mettre a 0 le bit D pour obtenir les touches directionnelles
	mettre a 0 le bit B pour obtenir les touches A, B, ...
	recuperer la valeur du registre P1 pour lire les touches enfoncees (utiliser BIT)
	/!\ touche enfoncee = bit a zero

exemple:
`````````````````````````````````````````````````````````
	LD	A, $20		; requete touches directionnelles
	LD	($FF00), A
	LD	A, ($FF00)
	.
	.
	LD	A, ($10)	; requete touches A, B, ...
	LD	($FF00), A
	LD	A, ($FF00)
	.
	.
	LD	A, $30		; 0x30 = (B | D) = requete reinitialisation du port ??
	LD	($FF00), A

`````````````````````````````````````````````````````````
*/










/******************
****** TIMER ******
******************/
# define DIV	0xff04U //????
/*
	ecrire dans ce registre le remet a zero
	il est automatiquement incremente toutes les ?µs
*/
# define TIMA	0xff05U // TIMER [VVVVVVVV]
/*
	Genere une interruption si overflow
*/
# define TMA	0xff06U // TIMA base value [VVVVVVVV]
/*
	La valeur de TMA est chargee dans TIMA lorsque TIMA overflow
*/
# define TAC	0xff07U // TIMER CONTROLLER [.....TSS]
/*
	T	=	Timer stop/start (0 = stop, 1 = start)
	SS	=	Input clock select
			00 = 4.194304 MHz / 2^10 = 4.096 KHz
			01 = 4.194304 MHz / 2^4 = 262.144 KHz
			10 = 4.194304 MHz / 2^6 = 65.536 KHz
			11 = 4.194304 MHz / 2^8 = 16.384 KHz

	Le timer doit etre demarre apres avoir ete initialise

exemple:
`````````````````````````````````````````````````````````
	LD		A, 2			; f/2^6  (f = 4.194304)
	LD		($FF00+07), A	; set clock speed
	LD		A, 6			; 0b00000 1 10
	LD		($FF00+07), A	; start timer

`````````````````````````````````````````````````````````
*/










/************************
**** INTERRUPT FLAGS ****
************************/
# define IF		0xff0fU // [...JSTLV] Interrupt request
/*
	list of requested interrupts

	J = Joypad interrupt
	S = Serial interrupt
	T = Timer interrupt
	L = LCDC interrupt
	V = VBlank interrupt

	0 = disable, 1 = enable
*/
# define IE		0xffffU // [...JSTLV] Interrupt enable
/*
	enabled interrupts list

	J = Joypad interrupt
	S = Serial interrupt
	T = Timer interrupt
	L = LCDC interrupt
	V = VBank interrupt

	0 = disable, 1 = enable
*/
//IME register (interrupt master enable) : 0 = DI, 1 = EI












/************************
**** SERIAL TRANSFER ****
************************/
# define SB		0xff01U
# define SC		0xff02U
// On s'en balance !











/***********************************************
**** WORKING RAM BANK SWITCH REGISTER (CGB) ****
***********************************************/
# define SVBK	0xff70U // [.....BBB]
/*
	Select VRAM bank

	BBB:
		0, 1 = bank 1
		2-7  = banks 2-7

	bank 1		= [0xc000-0xcfff]
	bank 2-7	= [0xd000-0xdfff]

*/









/************************************************
*** CPU OPERATING SPEED SWITCH REGISTER (CGB) ***
************************************************/
# define KEY1	0xff4dU // [S......E]
/*
	S = Speed Flag (Read Only)
	E = Enable speed flag

	le bit 0 est automatiquement mis a zero apres un changement de mode de vitesse du cpu.

	passage du mode normal a double-speed : 16ms
	passage du mode double-speed a normal : 32ms
	pour utiliser le double speed, il faut une cartouche qui le supporte.
	les registres DIV et TIMA operent eux aussi a double vitesse.


procedure de passage en mode double-speed:
``````````````````````````````````````````
	%define IF		0xff0f
	%define IE		0xffff
	%define KEY1	0xff4d
	%define P1		0xff00

	LD		HL, KEY1
	BIT		7, (HL)			; verification du mode actuel
	JR		NZ, _NO_SWITCH	; si le mode double-speed est deja active, on arrete

	SET		0, (HL)			; set double-speed flag

	XOR		A
	LD		(IF), A			; reset IF
	LD		(IE), A			; reset IE
	LD		A, 0x30
	LD		(P1), A			; reset P1
	STOP

_NO_SWITCH:
``````````````````````````````````````````
*/








/*************************************
**** INFRARED PORT REGISTER (CGB) ****
*************************************/
# define RP		0xff56U
// On s'en fout !!!!!!!








/**************************
***** VRAM BANK (CGB) *****
**************************/
# define VBK	0xff4fU // [.......B]
/*
	VRAM BANK SELECT
	[0x8000-0x9fff]

	B = Bank (0 ou 1)
*/










/******************************
**** LCD DISPLAY REGISTERS ****
******************************/
# define LCDC	0xff40U // [76543210]
/*
	0 = BG display.
		1 = on
		0 = off (if CGB it's always on)
	1 = OBJ.
		1 = on
		0 = off
	2 = OBJ block size.
		1 = 8*16
		0 = 8*8
	3 = BG code area.
		1 = 0x9c00 - 0x9fff,
		0 = 0x9800 - 0x9bff
	4 = BG chr data area
		1 = 0x8000 - 0x8fff
		0 = 0x8800 - 0x97ff
	5 = Windowing enable flag
		1 = on
		0 = off
	6 = Window code area
		1 = 0x9c00 - 0x9fff
		0 = 0x9800 - 0x9bff
	7 = LCD enable flag
		1 = on
		0 = off


	details BG, window, obj, ...


*/

# define STAT	0xff41U // [.LOVHFMM]
/*
	MM = Mode flag (read-only)
		00 = in horizontal blanking	(cpu can access to VRAM + OAM)
		01 = in vertical blanking	(cpu can access to VRAM + OAM)
		10 = reading OAM			(cpu cannot access to OAM)
		11 = reading both OAM VRAM	(cpu cannot acces to OAM + VRAM)
	
	F  = Coincidence flag (read-only)
		1 = LYC == LY
		0 = LYC != LY
	
	H = H-blank interrupt enable flag
		1 = on
		0 = off
	V = V-blank interrupt flag
		1 = on
		0 = off
	O = OAM interrupt flag
		1 = on
		0 = off
	L = LYC=LY coincidence interrupt
		1 = on
		0 = off

	detail routine affichage LCD :
````````````````````````````````
while (1)
{
	times(144)
	{
		10: lecture de l'OAM				(19µs)
		11: lecture de l'OAM + lecture VRAM	(40-60µs)
		00: horizontal blank: fin de ligne	(20-49µs)
	}
	01: vertical blank						(1087µs)
}
*/

# define SCY	0xff42U // [vvvvvvvv] scroll y (R/W)
/*
	position y dans le background (32*32 tiles)
	le background est circulaire.
*/
# define SCX	0xff43U // [vvvvvvvv] scroll x (R/W)
/*
	position x dans le background (32*32 tiles)
	le background est circulaire.
*/
# define LY		0xff44U // [vvvvvvvv] lcdc y coordinate (read-only)
/*
	0 - 153 (144-153 = v-blank)
*/
# define LYC	0xff45U // [vvvvvvvv] LY compare (R/W)
/*
	LY to compare register (LYC == LY interrupt, ...)
*/
# define BGP	0xff47U // [33221100] BG palette (R/W), non CGB-only
/*
	00 = shade for color number 0
	11 = shade for color number 1
	22 = shade for color number 2
	33 = shade for color number 3

	00 -> white
	01 -> light gray
	10 -> dark gray
	11 -> black
*/
# define OBP0	0xff48U // [332211..] OBJ palette 0 (R/W), non CGB-only
/*
	same as BGP (but color number 0 is transparent)
*/
# define OBP1	0xff49U // [332211..] OBJ palette 1 (R/W), non CGB-only
/*
	same as BGP (but color number 0 is transparent)
*/

# define WY		0xff4aU // [vvvvvvvv] window y coordinate (R/W)
/*
	0 - 143 = visible
	(modulo de 8 applique ?)
*/
# define WX		0xff4bU // [vvvvvvvv] window x coordinate (R/W)
/*
	0 - 166 = visible (bug hardware, real value = WX - 7)
	(modulo de 8 applique ?)
*/

/*********************************
**** CGB ONLY COLOR REGISTERS ****
*********************************/
#define BGPI	0xff68 // same as BCPS
#define BCPS	0xff68 // [w.xxxyyz]
/*
	
*/







/*******************
***** OAM AREA *****
*******************/
# define OBJ_Y(n)		0xfe00U + ((unsigned int)n << 2) //Y coordinate
# define OBJ_X(n)		0xfe01U + ((unsigned int)n << 2) //X coordinate
# define OBJ_CHR(n)		0xfe02U + ((unsigned int)n << 2) //CHR code
# define OBJ_ATTR(n)	0xfe03U + ((unsigned int)n << 2) //Attributes flags
// n = 0-40







/********************
*** DMA REGISTERS ***
********************/
# define DMA	0xff46U // [ssssssss] DMA transfer register 
/*
	ssssssss = source (0xss00 - 0xss9f)
				min = 0x80 (CGB = 0x00)
				max = 0xf1
	
	destination = 0xfe00 - 0xfe9f (OAM)
	
	transfer time = 160µs (60µs in double speed)

	write to this register starts the transfer

	exemple:
````````````````````````````````````````````````````
%define DMA		0xff46

3e c0			ld		A, 0xc0
e0 46			ld		(DMA), A	; start address to DMA
3e 28			ld		A, 40		; wait 4 x 40 cycles (~160µs)
3d		wait:	dec		A
20 fd			jrnz	wait
c9				ret
Example of program that writes the above instructions to
internal RAM starting from 0xff80:

		ld		C, 0x80
		ld		B, 10
		ld		HL, DMA_DATA	; code address
copy:	ld		A, (HL++)
		ld		(ff00+C), A		; copy to HRAM
		inc		C
		dec		B
		jrnz	copy
		call	0xff80			; ??????? maybe

.DMA_DATA	db 0x3e, 0xc0, 0xe0, 0x46, 0x3e
			db 0x28, 0x3d, 0x20, 0xfd, 0xc9

*/

# define HDMA1	0xff51U // [hhhhhhhh] source high
# define HDMA2	0xff52U // [llllllll] source low
/*
	source = 0xhhll & 0xfff0 (last four bits ignored)
			source = 0x0000-0x7ff0, 0xa000-0xdff0
	
	source == VRAM cause garbage to be copied
*/

# define HDMA3	0xff53U // [hhhhhhhh] destination high
# define HDMA4	0xff54U // [llllllll] destination low
/*
	destination = 0xhhll & 0xfff0 (last four bits ignored)
			destination = 0x8000-0x9ff0
*/

# define HDMA5	0xff55U //new DMA transfer register (CGB)
/*
	cpu is halted during new DMA transfer.
	writing to this register starts the transfer.
	16 bytes transfer takes 8µs
*/













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
