/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   launcher.c                                       .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/05 01:48:50 by mhouppin     #+#   ##    ##    #+#       */
<<<<<<< HEAD
/*   Updated: 2019/07/05 01:48:51 by mhouppin    ###    #+. /#+    ###.fr     */
=======
/*   Updated: 2019/07/08 09:18:03 by mhouppin    ###    #+. /#+    ###.fr     */
>>>>>>> 50f53cc5233d8cc41ae3c497a2d8122c3842a590
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "memory_map.h"
#include "cartridge.h"
#include "registers.h"
#include "processor.h"
#include "settings.h"
<<<<<<< HEAD
=======
#include "timing.h"
>>>>>>> 50f53cc5233d8cc41ae3c497a2d8122c3842a590

cycle_count_t	execute_once(registers_t *regs);

void	init_hardware_registers(void)
{
	KEY1_REGISTER = 0;
	LCDC_REGISTER = 0x91u;
	OBP0_REGISTER = 0xFFu;
	OBP1_REGISTER = 0xFFu;
	BGP_REGISTER = 0xFCu;
	P1_REGISTER = 0xCFu;
	TIMA_REGISTER = 0;
	TMA_REGISTER = 0;
	TAC_REGISTER = 0xF8u;
	IF_REGISTER = 0xE1u;
	IE_REGISTER = 0;
	SCY_REGISTER = 0;
	SCX_REGISTER = 0;
	LYC_REGISTER = 0;
	LY_REGISTER = 0;
	WY_REGISTER = 0;
	WX_REGISTER = 0;
	DMA_REGISTER = 0xFFu;
	DIV_REGISTER = 0xD3u;

	for (unsigned int i = 0; i < 8; i++)
		g_memmap.cart_reg[i] = 0;
	g_memmap.ime = false;
}

void	init_cpu_registers(registers_t *regs)
{
	regs->reg_sp = 0xFFFEu;
	regs->reg_pc = 0x100u;
	regs->reg_a = 0x01u;
	regs->reg_f = 0xB0u;
	regs->reg_b = 0x00u;
	regs->reg_c = 0x13u;
	regs->reg_d = 0x00u;
	regs->reg_e = 0xD8u;
	regs->reg_h = 0x01u;
	regs->reg_l = 0x4Du;
	regs->wbytes = 0;
}

void	start_game(void)
{
	registers_t		regs;

	init_hardware_registers();
	init_cpu_registers(&regs);
	g_settings.debug_mode = false;
	g_settings.uspeed_mode = false;
<<<<<<< HEAD
=======
	g_timing.lcd_cycles = 0;
	g_timing.line_render = 0;
	g_timing.render_status = OAM_READ;
	g_timing.timer_cycles = 0;
	g_timing.div_cycles = 0;
>>>>>>> 50f53cc5233d8cc41ae3c497a2d8122c3842a590
	while (1)
		execute_once(&regs);
}
