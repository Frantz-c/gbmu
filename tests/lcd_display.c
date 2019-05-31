/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   lcd_display.c                                    .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/31 13:46:11 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/05/31 14:18:53 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "memory_map.h"
#include "ram_registers.h"

void	render_screen(char *screen)
{
	size_t	char_count = (size_t)(g_memmap.complete_block[DMA] & 0x7Fu);

	return;
}

void	*screen_control_thread(void *param __attribute__((unused)))
{
	char	screen[160 * SCREEN_W];
	clock_t	delay;
	clcok_t new_delay;

	bzero(screen, sizeof(screen));
	while (true)
	{
		delay = clock();
		if (g_memmap.stop_mode == false)
			render_screen((char *)screen);
		delay = clock() - delay;
		delay = delay * 1000000u / CLOCKS_PER_SEC;
		delay %= 16667u;
		usleep(16667 - delay);
	}
}
