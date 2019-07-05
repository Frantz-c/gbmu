#ifndef GRAPHICS_H
# define GRAPHICS_H

#include <stdint.h>
#include "SDL.h"

void	gr_init_window(void);

extern SDL_Window	*g_window;
extern SDL_Texture	*g_texture;
extern SDL_Renderer	*g_render;
extern int32_t		*g_pixels;
extern int			g_pitch;

#endif
