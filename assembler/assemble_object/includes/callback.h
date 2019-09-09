/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   callback.h                                       .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <fcordon@le-101.fr>                +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/09/09 18:34:09 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/09 20:23:56 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef CALLBACK_H
# define CALLBACK_H

# include "std_includes.h"

void	loc_destroy(void *a);
void	loc_var_destroy(void *a);
int		loc_ext_compar(const void *a, const void *b);
int		loc_ext_search(const void *b, const void *a);

void	ext_destroy(void *a);

int		code_compar(const void *a, const void *b);
int		code_search(const void *b, const void *a);
void	code_destroy(void *a);

int		memblock_compar(const void *a, const void *b);
int		memblock_search(const void *b, const void *a);


#endif
