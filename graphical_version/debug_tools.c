/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   debug_tools.c                                    .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/06/20 17:39:24 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/06/21 18:30:03 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "debug_tools.h"
#include <stdint.h>

void	print_memory(const void *addr, size_t size)
{
	for (size_t i = 0; i < size; i += 32)
	{
		for (size_t b = 0; i + b < size && b < 32; b++)
		{
			printf("%.2hhx ", ((const uint8_t *)addr)[i + b]);
		}
		printf("\n");
	}
	fflush(stdout);
}
