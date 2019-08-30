/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   struct_tools.h                                   .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/08/26 16:57:45 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/28 16:25:56 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef STRUCT_TOOLS_H
# define STRUCT_TOOLS_H

# include "struct.h"
# include "vector.h"

macro_t			*get_macro(vector_t *macro, char *name);
uint8_t			get_params(char **s, char *param[10]);
char			*replace_content(macro_t *macro, char *param[10]);

int32_t		variables_match_name(const vector_t *memblock, const char *s, int32_t *block_i);
int32_t		memblock_match_name(const vector_t *memblock, const char *s);

void	push_instruction(code_area_t *area, uint8_t bin[4], param_t p[2], char *symbol,
						vector_t *ext_symbol, loc_sym_t *loc_symbol, data_t *data);
void	new_instruction(code_area_t *area);
int		push_byte(code_area_t *area, uint8_t byte);
void	push_params(code_area_t	*area, uint8_t inst[2], uint8_t p1[2], uint8_t p2[2], param_t t1, param_t t2, char *symbol);

#endif
