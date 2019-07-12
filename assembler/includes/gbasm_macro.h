/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_macro.h                                    .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/09 10:59:34 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/09 11:16:42 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef GBASM_MACRO_H
# define GBASM_MACRO_H

#include "gbasm_struct.h"

extern char	*add_macro_with_param(char *name, vector_t *macro, char *s, data_t *data);
extern char	*add_macro_without_param(char *name, vector_t *macro, char *s, data_t *data);
extern char	*undef_macro(vector_t *macro, char *s, data_t *data);
extern char	*define_macro(vector_t *macro, char *s, data_t *data);

#endif
