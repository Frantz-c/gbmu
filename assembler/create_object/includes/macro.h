/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   macro.h                                          .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/09 10:59:34 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/26 18:45:04 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef MACRO_H
# define MACRO_H

# include "std_includes.h"

extern char	*add_macro_with_param(char *name, vector_t *macro, char *s, data_t *data, char *parent);
extern char	*add_macro_without_param(char *name, vector_t *macro, char *s, data_t *data);
extern char	*undef_macro(vector_t *macro, char *s, data_t *data);
extern char	*define_macro(vector_t *macro, char *s, data_t *data);

#endif
