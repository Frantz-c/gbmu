#include "graphics.h"
#include "cartridge.h"

SDL_Window		*g_window;
SDL_Renderer	*g_render;
SDL_Texture		*g_texture;
int32_t			*g_pixels;
int				g_pitch;

void	gr_init_window(void)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
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
}
