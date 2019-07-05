/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   lcd_driver.h                                     .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/05 05:24:42 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/07/05 07:51:39 by mhouppin    ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef LCD_DRIVER_H
# define LCD_DRIVER_H

# include "processor.h"
# include <stdbool.h>

typedef struct	object_s
{
	uint8_t		lcd_y;
	uint8_t		lcd_x;
	uint8_t		code;
	uint8_t		attrib;
	uint8_t		type;
	uint8_t		next_prior;
	uint16_t	prior;
}				object_t;

typedef struct	oam_s
{
	object_t	obj[104];
	bool		active;
}				oam_t;

void	update_lcd(cycle_count_t cycles);
void	lcd_function(int line, int type);
void	update_display(void);
void	draw_line(oam_t *oam, int line);

#endif
