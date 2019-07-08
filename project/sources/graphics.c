/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   graphics.c                                       .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/05 01:48:43 by mhouppin     #+#   ##    ##    #+#       */
<<<<<<< HEAD
/*   Updated: 2019/07/05 01:48:44 by mhouppin    ###    #+. /#+    ###.fr     */
=======
/*   Updated: 2019/07/08 10:03:09 by mhouppin    ###    #+. /#+    ###.fr     */
>>>>>>> 50f53cc5233d8cc41ae3c497a2d8122c3842a590
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "graphics.h"
#include "cartridge.h"

<<<<<<< HEAD
=======
extern unsigned long	g_ticks;

>>>>>>> 50f53cc5233d8cc41ae3c497a2d8122c3842a590
SDL_Window		*g_window;
SDL_Renderer	*g_render;
SDL_Texture		*g_texture;
int32_t			*g_pixels;
int				g_pitch;

void	gr_init_window(void)
{
<<<<<<< HEAD
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
=======
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0)
>>>>>>> 50f53cc5233d8cc41ae3c497a2d8122c3842a590
	{
		fprintf(stderr, "SDL_Init() failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	g_window = SDL_CreateWindow(g_cart.game_title,
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 160, 144,
		SDL_WINDOW_RESIZABLE);
	if (g_window == NULL)
	{
		fprintf(stderr, "SDL_CreateWindow() failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	g_render = SDL_CreateRenderer(g_window, -1, 0);
	if (g_render == NULL)
	{
		fprintf(stderr, "SDL_CreateRenderer() failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	g_texture = SDL_CreateTexture(g_render, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING, 160, 144);
	if (g_texture == NULL)
	{
		fprintf(stderr, "SDL_CreateTexture() failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	SDL_LockTexture(g_texture, NULL, (void **)&g_pixels, &g_pitch);
<<<<<<< HEAD
=======

	g_ticks = SDL_GetTicks();
>>>>>>> 50f53cc5233d8cc41ae3c497a2d8122c3842a590
}
