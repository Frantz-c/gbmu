/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   instruction_or_label.h                           .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/08/27 13:46:07 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/27 13:46:11 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef GBASM_INSTRUCTION_OR_LABEL_H
# define GBASM_INSTRUCTION_OR_LABEL_H

char	*add_instruction(char *inst, vector_t *area, vector_t *ext_symbol,
						loc_sym_t *loc_symbol, vector_t *macro, char *s, data_t *data);
void	add_label(char *name, vector_t *area, vector_t *ext_symbol, loc_sym_t *loc_symbol, data_t *data);

#endif
