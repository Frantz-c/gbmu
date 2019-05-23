/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   ram_registers.h                                  .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/23 18:27:51 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/05/23 18:50:10 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

/*
 * Mieux vaut rajouter des pointeurs dans la structure memory_map_s
 * (memory_map->complete_block + define)
 */

#ifndef RAM_REGISTERS_H
# define RAM_REGISTERS_H

# define P1		0xff00U

# define DIV	0xff04U

// timer
# define TIMA	0xff05U
# define TMA	0xff06U
# define TAC	0xff07U

//interrupts
# define IF		0xff0fU
# define IE		0xffffU
# define IME	??????

//serial transfer
# define SB		0xff01U // data register
# define SC		0xff02U // control register

#endif
