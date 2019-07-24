/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_callback.h                                 .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/12 16:16:17 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/24 11:11:31 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef GBASM_CALLBACK_H
# define GBASM_CALLBACK_H

void	memblock_destroy(void *a);
void	macro_destroy(void *a);
int		macro_destroy_allocated(void *a);
void	area_destroy(void *a);
void	label_destroy(void *a);
void	ext_symbol_destroy(void *a);
int		area_match(const void *a, const void *b);
int		macro_match(const void *a, const void *b);
int		label_match(const void *a, const void *b);
int		memblock_match(const void *a, const void *b);
int		ext_symbol_match(const void *b, const void *a);
int		label_cmp(const void *a, const void *b);
int		area_cmp(const void *a, const void *b);
int		macro_cmp(const void *a, const void *b);
int		memblock_cmp(const void *a, const void *b);
int		ext_symbol_cmp(const void *a, const void *b);
int		macro_filter(void *a);

#endif
