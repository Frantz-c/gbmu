/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_get_bin_instruction.h                      .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/25 10:02:44 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/25 10:02:55 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef GBASM_GET_BIN_INSTRUCTION_H
# define GBASM_GET_BIN_INSTRUCTION_H

param_error_t	get_bin_instruction(char *mnemonic, param_t param[2], value_t *val, uint8_t bin[4]);

#endif
