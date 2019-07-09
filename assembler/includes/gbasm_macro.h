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

extern char	*add_macro_without_param(char *name, defines_t **def, char *s, error_t *err);
extern char	*add_macro_with_param(char *name, defines_t **def, char *s, error_t *err);
extern char	*undef_macro(defines_t *def[], char *s, error_t *err);
extern char	*define_macro(defines_t *def[], char *s, error_t *err);

#endif
