/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   execute.h                                        .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: mhouppin <mhouppin@le-101.fr>              +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/29 09:58:29 by mhouppin     #+#   ##    ##    #+#       */
/*   Updated: 2019/06/04 21:07:31 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef EXECUTE_H
# define EXECUTE_H

# include "registers.h"
# include "../tests/memory_map.h"

typedef unsigned long	cycle_count_t;

cycle_count_t	execute(registers_t *regs);

#endif
