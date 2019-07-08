/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gameboy_core.c                                   .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/05 01:48:34 by mhouppin     #+#   ##    ##    #+#       */
<<<<<<< HEAD
/*   Updated: 2019/07/05 05:29:02 by mhouppin    ###    #+. /#+    ###.fr     */
=======
/*   Updated: 2019/07/08 09:58:22 by mhouppin    ###    #+. /#+    ###.fr     */
>>>>>>> 50f53cc5233d8cc41ae3c497a2d8122c3842a590
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

<<<<<<< HEAD
=======
#include "check_events.h"
>>>>>>> 50f53cc5233d8cc41ae3c497a2d8122c3842a590
#include "cpu_specs.h"
#include "lcd_driver.h"
#include "write.h"
#include "bitmask.h"
#include "registers.h"
#include "processor.h"
#include "memory_map.h"
#include "cartridge.h"

cycle_count_t	execute_once(registers_t *regs)
{
	cycle_count_t	cycles;
	cycle_count_t	rcycles;

	regs->wbytes = 0;
	if (GAMEBOY_STATUS == NORMAL_MODE)
		cycles = execute(regs);
	else
		cycles = 4;

	rcycles = cycles;
	if (KEY1_REGISTER & BIT_7)
		rcycles /= 2;

	if (regs->wbytes)
		write_bytes(regs);

	update_lcd(rcycles);
	if (GAMEBOY_STATUS != STOP_MODE)
		update_timer_values(cycles);
<<<<<<< HEAD
=======
	check_cntrl_events(cycles);
>>>>>>> 50f53cc5233d8cc41ae3c497a2d8122c3842a590
	check_interrupts(regs);
	return (rcycles);
}
