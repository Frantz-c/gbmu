/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_global.h                                   .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/10 18:47:38 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/31 21:19:26 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef GBASM_GLOBAL_H
# define GBASM_GLOBAL_H

# include "vector.h"
# include "struct.h"

// mnemonics table (name + label_addr)
extern const char *const	*inst;

// errors & warnings
extern uint32_t				g_error;
extern uint32_t				g_warning;

// cartridge data
extern cart_data_t			cartridge;

// verify if multiple access to a catrtidge_info member
extern cart_info_t			cart_info;

// automatique double include protection
extern char					*included_list[128];
extern uint32_t				included_index;

// path + path_len, example: "sources/main.s" -> "sources/", 8
extern char					*base;
extern uint32_t				base_length;

extern uint8_t				inst_length[256];

#endif
