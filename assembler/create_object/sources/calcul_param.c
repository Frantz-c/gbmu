/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   calcul_param.c                                   .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <fcordon@le-101.fr>                +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/08/26 15:36:50 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/28 23:02:18 by fcordon     ###    #+. /#+    ###.fr     */
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
	uint8_t		not;
	
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
		if (*s == '~')
		{
			s++;
			not = 1;
		}
		else
			not = 0;

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
			if (not) n = (uint16_t)~n | (n & 0xffff0000u);
			s += len;
			printf("\e[1;32mpush_value(%i);\n\e[0m", n);
		}

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
			case '*': case '/': case '%': case '|': case '&': case '^': case '>': case '<':
			{
				if ((lvl & 0x1u) == 0 && lvl >= prev_lvl)
				{
					calc[i-1].lvl++;
					lvl++;
				}
				break;
			}
			case '+': case '-':
			{
				lvl &= ~0x1u;
			}
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

extern uint32_t	calcul_param(char *p, value_t *val, data_t *data, uint8_t param_number)
{
	calc_elem_t	*calc = NULL;
//	uint32_t	len;
	uint32_t	n_elem;

	if (*p == '[') p++;
	if (*p == '~' || *p == '(' || is_numeric_len(p, NULL))
	{
		printf("get_calc(\"%s\");\n", p);
		calc = get_calc(p, &n_elem);
		val->sign = '+';

		if (n_elem == 0)
			goto __impossible_error;
		if (n_elem == 1)
			val->value = (uint32_t)calc->val;
		else
			val->value = (uint32_t)execute_calcul(calc, n_elem);

		if (calc->val > 0xffff)
			goto __overflow;
		if (calc->val < -0x8000)
			goto __underflow;
	}
	else
	{
		while (is_alnum(*p) || *p == '_') p++;
		val->sign = *p;

		if (is_endl(p[1]))
		{
			val->sign = '+';
			val->value = 0;
			return (0);
		}

		*(p++) = '\0';
		printf("(2) get_calc(\"%s\");\n", p);
		calc = get_calc(p, &n_elem);

		if (n_elem == 0)
			goto __impossible_error;
		else if (n_elem == 1)
			val->value = (uint32_t)calc->val;
		else
			val->value = execute_calcul(calc, n_elem);

		if (calc->val > 0xffff)
			goto __overflow;
		if (calc->val < -0x8000)
			goto __underflow;
	}

__ret_0:
	free(calc);
	return (0);


	const char	*error_msg;
__impossible_error:
	sprintf(data->buf, "impossible error argument %u", param_number);
	goto __print_error_fmt;
__overflow:
	sprintf(data->buf, "overflow argument %u (value is %u (0x%x))", param_number, val->value, val->value);
	goto __print_error_fmt;
__underflow:
	sprintf(data->buf, "underflow argument %u (value is %i (-0x%x))", param_number, (int32_t)val->value, -((int32_t)val->value));
__print_error_fmt:
	error_msg = (const char *)data->buf;
__print_error:
	print_error(data->filename, data->lineno, data->line, error_msg);
	free(calc);
	return (0xffffffffu);
}
