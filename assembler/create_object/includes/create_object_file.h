/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   create_object_file.h                             .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/08/04 17:04:48 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/26 18:46:43 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef CREATE_OBJECT_FILE_H
# define CREATE_OBJECT_FILE_H

# include "struct.h"
# include "vector.h"

int		create_object_file(vector_t *code_area, loc_sym_t *local_symbol, vector_t *extern_symbol, char *filename);

#endif
