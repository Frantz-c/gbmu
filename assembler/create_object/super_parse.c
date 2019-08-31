/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   super_parse.c                                    .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/08/28 19:11:21 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/28 19:23:39 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

struct super_error_s
{
	uint32_t	champs;
	uint32_t	position;
};

struct super_error	error;

error = super_parse("s:[a-zA-Z_][a-zA-Z0-9_]*\1"
					"[ \t]+\1"
					"s:[-+/%*]\1"
					"[ \t]+\1"
					"i:[0-9]*",
					&identifier, &operator, &number);

