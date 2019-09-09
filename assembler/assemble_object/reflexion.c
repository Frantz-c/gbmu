/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   reflexion.c                                      .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/09/09 16:45:42 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/09 17:06:12 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

"file1"

.extern	var
.extern	func

ld		A,		12
ld		[var],	A
call	func

next:
	stop

------------------------

"file2"

%define	DEFAULT		1
%define	LOAD_ADDR(addr, value)	ld	A, value\
								ld	[addr], value

.memlock	var_area, 0xC000, 0xC010
.var1		var, var_area

LOAD_ADDR(var, DEFAULT)


------------------------

"file3"

.extern	var
.extern	next

func:
	halt
	nop
	ret

ld	A, [var]
cp	10
jp	next





===========================


file 1 = {var, q, p,   func, q, p},	{next, 0x2343}
file 2 = {NULL},					{var_area, 0xc000, 0xc010,    var, var_area, 1}
file 3 = {var, q, p,   next, q, p},	{func, 0x2444}
