/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_add_label.c                                .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/22 22:53:58 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/22 22:56:23 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "gbasm_struct.h"
#include "gbasm_tools.h"
#include "gbasm_error.h"

void	add_label(char *name, vector_t *area, vector_t *ext_symbol, loc_sym_t *loc_symbol, data_t *data)
{
	// verifier s'il y a un double du symbole dans les symboles locaux et externes
	label_t	new = {name, VEC_ELEM(code_area_t, area, 0)->addr};
	vector_push(loc_symbol->label, (void*)&new);
}
