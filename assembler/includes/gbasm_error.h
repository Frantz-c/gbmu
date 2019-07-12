/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_error.h                                    .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/12 13:19:39 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/12 16:22:11 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef GBASM_ERROR_H
# define GBASM_ERROR_H

# include "std_includes.h"

void			print_warning(char *filename, uint32_t lineno, char *line, char *error);
void			print_error(char *filename, uint32_t lineno, char *line, char *error);

#endif
