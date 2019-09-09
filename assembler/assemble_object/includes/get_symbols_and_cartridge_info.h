/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   get_symbols_and_cartridge_info.h                 .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <fcordon@le-101.fr>                +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/09/09 18:21:33 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/09 19:08:37 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef GET_SYMBOLS_AND_CARTRIDGE_INFO_H
# define GET_SYMBOLS_AND_CARTRIDGE_INFO_H

# include "std_includes.h"

void	get_symbols_and_cartridge_info(const char *filename, loc_symbols_t *loc, vector_t *ext, uint32_t file_number);

#endif
