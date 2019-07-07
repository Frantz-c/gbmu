/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   pkmn_green_string.h                              .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/03 09:20:24 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/03 09:20:45 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef PKMN_GREEN_STRING_H
# define PKMN_GREEN_STRING_H

# include <stdint.h>

extern int		get_hiragana(char **s, uint8_t **mem, uint8_t *count);
extern int		get_katakana(char **s, uint8_t **mem, uint8_t *count);

#endif
