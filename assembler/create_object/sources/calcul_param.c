/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   calcul_param.c                                   .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <fcordon@le-101.fr>                +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/08/26 15:36:50 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/27 13:55:34 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "calcul_param.h"

static calc_elem_t	*get_calc(const char *s, uint32_t *n_elem)
{
	calc_elem_t	*calc = NULL;
	uint32_t	i = 0;
	int32_t		n;
	int32_t		lvl = 0;
	uint8_t		minus;
	
	*n_elem = 0;

	if (*s == 0)
		return (NULL);

	while (1)
	{
// is opened parent ?
		while (*s == '(') {
			s++;
			lvl &= ~0x1u;	// cplt = 0;
			lvl += PARENT_VALUE;
		}


// get number
		if (*s == '-')
		{
			minus = 1;
			s++;
		}
		else
			minus = 0;

		//
		{
			uint32_t	len;
			register int32_t	type = is_numeric(s, NULL);
			if (type == 0)
				goto __error_not_digit;
			n = (int32_t)atou_type(s, &len, type);
			if (minus) n = -n;
			s += len;
		}

/*
		if (!is_digit(*s))
			goto __error_not_digit;

		register uint32_t	tmp = 0;
		tmp = *(s++) - 48;
		while (is_digit(*s)) {
			tmp *= 10;
			tmp += *(s++) - 48;
		}
		if (minus)
			n = (int32_t)(~tmp + 1);
		else
			n = (int32_t)tmp;
*/

// add number
		if ((i & 0x7) == 0)
			calc = realloc(calc, sizeof(calc_elem_t) * (i + 8));
		calc[i].val = n;
		calc[i++].lvl = lvl;
		

// is closed parent ?
		int32_t	prev_lvl = lvl;
		while (*s == ')') {
			s++;
			lvl &= ~0x1u; // cplt = 0
			prev_lvl = lvl;
			lvl -= PARENT_VALUE;
			if (lvl < 0)
				goto __parent_error;
		}


// get operator
		switch (*s)
		{
			case '*': case '/': case '%': case '|': case '&': case '^':
			{
				if ((lvl & 0x1u) == 0 && lvl >= prev_lvl)
				{
					calc[i-1].lvl++;
					lvl++;
				}
				break;
			}
			case '>': case '<':
			{
				if (s[1] != s[0])
					exit(1);
				if ((lvl & 0x1u) == 0)
				{
					calc[i-1].lvl++;
					lvl++;
				}
				s++;
				break;
			}
			case '+':
			case '-':
				lvl &= ~0x1u;
			case '\0':
				break;
			default:
				fprintf(stderr, "ope error (%c::%d) \"%s\"\n", *s, *s, s); exit(1);
		}

// add operator
		calc[i].val = *s;
		calc[i++].lvl = lvl;

		if (*s == 0)
			break;
		s++;
	}
	
	if ((lvl &= ~0x1u) != 0)
		goto __parent_error;
	*n_elem = i;
	calc[i-1].lvl = 0;
	return (calc);

	__error_not_digit:
		fprintf(stderr, "%c is not a number\n", *s);
		exit(1);
	__parent_error:
		fprintf(stderr, "parent error\n");
		exit(1);
}


static int32_t	calculate(calc_elem_t calc[])
{
	switch (calc[1].val)
	{
		case '*': return (calc[0].val * calc[2].val);
		case '/': return (calc[0].val / calc[2].val);	// check if r_value == 0 (error)
		case '%': return (calc[0].val % calc[2].val);	// check if r_value == 0 (error)
		case '|': return (calc[0].val | calc[2].val);
		case '&': return (calc[0].val & calc[2].val);
		case '^': return (calc[0].val ^ calc[2].val);
		case '>': return (calc[0].val >> calc[2].val);	// check if r_value < 16 && > -1
		case '<': return (calc[0].val << calc[2].val);	// check if r_value < 16 && > -1
		case '+': return (calc[0].val + calc[2].val);
		case '-': return (calc[0].val - calc[2].val);
		case '\0': return (calc[0].val);
		default : fprintf(stderr, "OPE ERROR"); exit(1);
	}
	return (0);
}

static int32_t	execute_calcul(calc_elem_t *calc, uint32_t n_elem)
{
	uint32_t	start, end;

	for (start = 0; start < n_elem; )
	{
		for (end = start + 1; end < n_elem && calc[start].lvl == calc[end].lvl; end++);

		if (end - start >= 3)
		{
			register uint32_t	len = end - start;

			while (len >= 3)
			{
				printf("(1)calculate(%d %c %d);\n", calc[start].val, calc[start+1].val, calc[start+2].val);
				calc[start].val = calculate(calc + start);
				end -= 2;
				len = end - start;
				memmove(calc + start + 1, calc + start + 3, (n_elem - (start + 2)) * sizeof(calc_elem_t));
				n_elem -= 2;
			}
		}
		else
			start += 2;
	}

	while (n_elem > 2)
	{
		for (start = 1; start < n_elem - 1; )
		{
			if ((start < 2 || calc[start-2].lvl < calc[start].lvl) && calc[start+2].lvl <= calc[start].lvl)
			{
				printf("(2)calculate(%d %c %d);\n", calc[start-1].val, calc[start].val, calc[start+1].val);
				calc[start-1].val = calculate(calc + start - 1);
				memmove(calc + start, calc + start + 2, (n_elem - (start + 1)) * sizeof(calc_elem_t));
				n_elem -= 2;
			}
			else
				start += 2;
		}
	}
	return (calc[0].val);
}

extern uint32_t	calcul_param(char *p, value_t *val)
{
	calc_elem_t	*calc = NULL;
	uint32_t	len;
	uint32_t	n_elem;

	if (is_numeric(p, &len) && !is_alnum(p[len]) && p[len] != '_' && p[len] != 0)
	{
		calc = get_calc(p, &n_elem);
		if (n_elem == 0)
		{
			free(calc);
			return (0xffffffffu);
		}
		if (n_elem == 1)
		{
			val->value = (uint32_t)calc->val;
			goto __ret_0;
		}
		val->value = (uint32_t)execute_calcul(calc, n_elem);
	}
	else
	{
		while (is_alnum(*p) || *p == '_') p++;
		val->sign = *p;
		*(p++) = '\0';
		calc = get_calc(p, &n_elem);
		if (n_elem == 0)
		{
			free(calc);
			return (0xffffffffu);
		}
		else if (n_elem == 1)
		{
			val->value = (uint32_t)calc->val;
			goto __ret_0;
		}
		val->value = execute_calcul(calc, n_elem);
	}

__ret_0:
	free(calc);
	return (0);
}
