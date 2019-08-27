/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   get_bin_instruction.h                            .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <fcordon@le-101.fr>                +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/25 10:02:44 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/27 13:51:05 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef GET_BIN_INSTRUCTION_H
# define GET_BIN_INSTRUCTION_H

param_error_t	get_bin_instruction(char *mnemonic, param_t param[2], value_t *val, uint8_t bin[4]);

#endif
