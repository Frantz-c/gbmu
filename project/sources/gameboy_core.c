#include "registers.h"
#include "memory_map.h"
#include "cartridge.h"

cycle_count_t	execute_once(registers_t *regs)
{
	cycle_count_t	cycles;
	cycle_count_t	rcycles;

	regs->wbytes = 0;
	if (GAMEBOY == NORMAL_MODE)
		cycles = execute(regs);
	else
		cycles = 4;

	rcycles = cycles;
	if (KEY1_REGISTER & BIT_7)
		rcycles /= 2;

	if (regs->wbytes)
		write_bytes(regs);

	update_lcd(rcycles);
	if (GAMEBOY != STOP_MODE)
		update_timer_values(cycles);
	check_interrupts(&regs);
	return (rcycles);
}
