/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   file_read.c                                      .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/05 01:48:23 by mhouppin     #+#   ##    ##    #+#       */
<<<<<<< HEAD
/*   Updated: 2019/07/05 01:48:24 by mhouppin    ###    #+. /#+    ###.fr     */
=======
/*   Updated: 2019/07/08 09:22:29 by mhouppin    ###    #+. /#+    ###.fr     */
>>>>>>> 50f53cc5233d8cc41ae3c497a2d8122c3842a590
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "memory_map.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ROM_MAX_LENGTH	0x800000u

void	*get_file_contents(const char *path, uint32_t *length)
{
	void		*content;
	FILE		*f;

	f = fopen(path, "r");
	if (f == NULL)
<<<<<<< HEAD
	{
		perror("Cartridge opening failed");
		return (NULL);
	}
=======
		return (NULL);
>>>>>>> 50f53cc5233d8cc41ae3c497a2d8122c3842a590
	fseek(f, 0, SEEK_END);
	*length = (uint32_t)ftell(f);
	rewind(f);
	if (*length == 0)
	{
		fprintf(stderr, "Empty cartridge\n");
		return (NULL);
	}
	if (*length > ROM_MAX_LENGTH)
	{
		fprintf(stderr, "Too Heavy cartridge\n");
		return (NULL);
	}
	content = valloc(*length);
	if (content == NULL)
	{
		perror("Cartridge load failed");
		return (NULL);
	}
	if (fread(content, 1, *length, f) != *length)
	{
		perror("Cartridge load failed");
		free(content);
		return (NULL);
	}
	fclose(f);
	return (content);
}

void	load_saved_external_ram(const char *path)
{
	char		*save_name = (char *)malloc(strlen(path) + 2);
	char		*p;
	uint8_t		*save;
	uint32_t	length;

	strcpy(save_name, path);
	if ((p = strrchr(save_name, '.')) == NULL ||
		(strcmp(".gb", p) && strcmp(".gbc", p)))
	{
		fprintf(stderr, "Invalid cartridge extension \"%s\": no save loaded\n", p);
		g_memmap.save_name = NULL;
		free(save_name);
		return ;
	}
	strcpy(p + 1, "sav");
	g_memmap.save_name = save_name;

	if ((g_memmap.save_size = get_external_ram_size()) == 0)
	{
		free(g_memmap.save_name);
		g_memmap.save_name = NULL;
		return ;
	}

	if ((save = get_file_contents(save_name, &length)) == NULL)
	{
		bzero(g_memmap.extern_ram, g_memmap.save_size);
		return ;
	}

	if (length != g_memmap.save_size)
	{
		fprintf(stderr, "Wrong save size (%lu, should be %lu): no save loaded\n",
			(unsigned long)length, (unsigned long)g_memmap.save_size);
		free(save);
		return ;
	}
	memcpy(g_memmap.extern_ram, save, g_memmap.save_size);
	free(save);
}
