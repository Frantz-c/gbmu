/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   parse_instruction.h                              .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <fcordon@le-101.fr>                +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/08/27 14:44:38 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/27 14:45:03 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef PARSE_INSTRUCTION_H
# define PARSE_INSTRUCTION_H

char	*parse_instruction(char *s, vector_t *area, vector_t *ext_symbol, loc_sym_t *loc_symbol, vector_t *macro, data_t *data);

#endif
