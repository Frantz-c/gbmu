#include "lcd_driver.h"
#include "graphics.h"
#include "settings.h"

void	update_lcd(cycle_count_t cycles)
{
	SDL_UnlockTexture(g_texture);
	SDL_RenderClear(g_render);
	SDL_RenderCopy(g_render, g_texture, NULL, NULL);
	SDL_RenderPresent(g_render);
	SDL_LockTexture(g_texture, NULL, (void **)&g_pixels, &g_pitch);

	if (!g_settings.debug_mode && !g_settings.uspeed_mode)
	{
		uint32_t cur_ticks = SDL_GetTicks();
		if (cur_ticks - ticks < 17)
			SDL_Delay(17 - (cur_ticks - ticks));
		ticks = SDL_GetTicks();
	}
}
