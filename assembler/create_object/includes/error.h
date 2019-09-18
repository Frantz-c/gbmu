/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_error.h                                    .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/12 13:19:39 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/18 11:30:33 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef GBASM_ERROR_H
# define GBASM_ERROR_H

# include "std_includes.h"

void			print_warning(char *filename, uint32_t lineno, char *line, const char *error);
void			print_warning_dont_show(char *filename, uint32_t lineno, const char *error);
void			print_error(char *filename, uint32_t lineno, char *line, const char *error);
void			print_error_dont_show(char *filename, uint32_t lineno, const char *error);

#endif
