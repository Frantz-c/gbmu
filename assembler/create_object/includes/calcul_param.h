/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   calcul_param.h                                   .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <fcordon@le-101.fr>                +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/08/26 17:49:44 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/27 19:18:21 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#ifndef CALCUL_PARAM_H
# define CALCUL_PARAM_H

typedef struct	calc_elem_s
{
	int32_t		val;		// value or operator
	int32_t		lvl;
}
calc_elem_t;

#define PARENT_VALUE	2

uint32_t	calcul_param(char *p, value_t *val, data_t *data, uint8_t param_number);

#endif
