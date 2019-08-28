/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   tools.c                                          .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/12 23:05:07 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/28 21:18:28 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "tools.h"

const uint32_t	ascii[256] = {
	0x40,0,0,0,0,0,0,0,0,2, //0
	0x40,0,0,0,0,0,0,0,0,0, //10
	0,0,0,0,0,0,0,0,0,0, //20
	0,0,2,0,0,0,0,4,4,0, //30
	0x80,0x80,4,4,0,4,0,4,1,1, //40
	1,1,1,1,1,1,1,1,0,0, //50
	4,0,4,0,0,0x18,0x18,0x18,0x18,0x18, //60
	0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18, //70
	0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18, //80
	0x18,0x80,0,0x80,4,0,0,0x28,0x28,0x28, //90
	0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28, //100
	0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28, //110
	0x28,0x28,0x28,0,4,0,0,0,0,0 //120
};

const uint8_t	to_lower_char[128] = {
	0,1,2,3,4,5,6,7,8,9,
	10,11,12,13,14,15,16,17,18,19,
	20,21,22,23,24,25,26,27,28,29,
	30,31,32,33,34,35,36,37,38,39,
	40,41,42,43,44,45,46,47,48,49,
	50,51,52,53,54,55,56,57,58,59,
	60,61,62,63,64,97,98,99,100,101,
	102,103,104,105,106,107,108,109,110,111,
	112,113,114,115,116,117,118,119,120,121,
	122,91,92,93,94,95,96,97,98,99,
	100,101,102,103,104,105,106,107,108,109,
	110,111,112,113,114,115,116,117,118,119,
	120,121,122,123,124,125,126,127
};

extern void __attribute__((always_inline))		str_to_lower(char *s)
{
	while (*s)
	{
		*s = to_lower_char[*s & 0x7f];
		s++;
	}
}

extern uint8_t	is_numeric_len(const char *s, uint32_t *len)
{
	const char	*p = s;

	if (*s == '0' && LOWER(s[1]) == 'x')
	{
		s += 2;
		if (!is_digit(*s) && (LOWER(*s) < 'a' || LOWER(*s) > 'f'))
			goto __not_numeric;
		s++;
		while (is_digit(*s) || (LOWER(*s) >= 'a' && LOWER(*s) <= 'f'))
			s++;
	}
	else if (*s == '0')
	{
		s++;
		while (*s >= '0' && *s <= '7')
			s++;
	}
	else if (is_digit(*s))
	{
		s++;
		while (is_digit(*s))
			s++;
	}
	else
		goto __not_numeric;

	if (len)
		*len = (uint32_t)(s - p);
	return (1);

__not_numeric:
	if (len)
		*len = 0;
	return (0);
}

extern uint8_t	is_numeric_inc(char **s)
{
	char	*p;

	if (**s == '0' && LOWER((*s)[1]) == 'x')
	{
		p = *s + 2;
		if (!is_digit(*p) && (LOWER(*p) < 'a' || LOWER(*p) > 'f'))
			return (0);
		p++;
		while (is_digit(*p) || (LOWER(*p) >= 'a' && LOWER(*p) <= 'f'))
			p++;
	}
	else if (**s == '0')
	{
		p = *s + 1;
		while (*p >= '0' && *p <= '7')
			p++;
	}
	else if (is_digit(**s))
	{
		p = *s + 1;
		while (is_digit(*p))
			p++;
	}
	else
		return (0);

	*s = p;
	return (1);
}

extern uint32_t	atou_len(const char *s, uint32_t *len)
{
	const char	*p = s;
	uint32_t	n;

	if (*s == '0' && LOWER(s[1]) == 'x')
	{
		s += 2;
		if (is_digit(*s))
			n = *s - '0';
		else if (LOWER(*s) >= 'a' && LOWER(*s) <= 'f')
			n = LOWER(*s) - 87;
		else
			goto __ret_0;
		while (1)
		{
			s++;
			if (is_digit(*s))
				n = (n << 4) | (*s - '0');
			else if (LOWER(*s) >= 'a' && LOWER(*s) <= 'f')
				n = (n << 4) | (LOWER(*s) - 87) ;
			else
				break;
		}
	}
	else if (*s == '0')
	{
		if (*s >= '0' && *s <= '7')
			n = *s - '0';
		else
			goto __ret_0;
		while (1)
		{
			s++;
			if (*s >= '0' && *s <= '7')
				n = (n << 3) | (*s - '0');
			else
				break;
		}
	}
	else if (is_digit(*s))
	{
		if (is_digit(*s))
			n = *s - '0';
		else
			goto __ret_0;
		while (1)
		{
			s++;
			if (is_digit(*s))
				n = (n * 10) + (*s - '0');
			else
				break;
		}
	}
	else
		goto __ret_0;

	if (len)
		*len = (uint32_t)(s - p);
	return (n);

__ret_0:
	if (len)
		*len = 0;
	return (0);
}

extern uint32_t	atou_inc(char **s)
{
	char	*p;
	uint32_t	n;

	if (**s == '0' && LOWER((*s)[1]) == 'x')
	{
		p = *s + 2;
		if (is_digit(*p))
			n = *p - '0';
		else if (LOWER(*p) >= 'a' && LOWER(*p) <= 'f')
			n = LOWER(*p) - 87;
		else
			goto __ret_0;
		while (1)
		{
			p++;
			if (is_digit(*p))
				n = (n << 4) | (*p - '0');
			else if (LOWER(*p) >= 'a' && LOWER(*p) <= 'f')
				n = (n << 4) | (LOWER(*p) - 87) ;
			else
				break;
		}
	}
	else if (**s == '0')
	{
		if (**s >= '0' && **s <= '7')
			n = **s - '0';
		else
			goto __ret_0;
		p = *s + 1;
		while (1)
		{
			if (*p >= '0' && *p <= '7')
				n = (n << 3) | (*p - '0');
			else
				break;
			p++;
		}
	}
	else if (is_digit(**s))
	{
		if (is_digit(**s))
			n = **s - '0';
		else
			goto __ret_0;
		p = *s + 1;
		while (1)
		{
			if (is_digit(*p))
				n = (n * 10) + (*p - '0');
			else
				break;
			p++;
		}
	}
	else
		goto __ret_0;

	*s = p;
	return (n);

__ret_0:
	return (0);
}


extern uint8_t		is_numeric(const char *s, uint32_t *len)
{
	uint8_t		type = 0;
//	uint8_t		_signed = 0;
	const char	*p = s;
/*
	if (*s == '-') {
		_signed = 1;
		s++;
	}
*/
	if (*s == '0' && (s[1] == 'x' || s[1] == 'X') &&
		(is_digit(s[2]) || (to_lower_char[(uint8_t)s[2]] >= 'a' && to_lower_char[(uint8_t)s[2]] <= 'f')))
	{
		type = HEXA_NUM;
		s += 2;
		while (is_digit(*s) || (to_lower_char[(uint8_t)*s] >= 'a' && to_lower_char[(uint8_t)*s] <= 'f'))
			s++;
	}
	else if (*s == '0' && s[1] >= '0' && s[1] <= '7')
	{
		type = OCTAL_NUM;
		s++;
		do {
			s++;
		} while (*s >= '0' && *s <= '7');
		if (is_digit(*s))
			goto __decimal;
		if (to_lower_char[(uint8_t)*s] == 'h') {
			type = HEXA_NUM;
			s++;
		}
	}
	else if (is_digit(*s))
	{
__decimal:
		type = DECIMAL_NUM;
		s++;
		while (is_digit(*s)) s++;
		if ((*s >= 'a' && *s <= 'f') || (*s >= 'A' && *s <= 'F') || *s == 'h' || *s == 'H')
		{
		__hexadecimal:
			type = HEXA_NUM;
			while (is_digit(*s) || (LOWER(*s) >= 'a' && LOWER(*s) <= 'f')) s++;
			if (LOWER(*s) == 'h') s++;
			else
			{
				if (len) *len = 0;
				return (0);
			}
		}
	}
	else if ((*s >= 'a' && *s <= 'f') || (*s >= 'A' && *s <= 'F'))
	{
		goto __hexadecimal;
	}

	if (len) *len = (uint32_t)(s - p);
	return (type);
}

extern uint32_t		var_len(const char *s)
{
	const char	*p = s;
	if (!is_alnum(*s) && *s != '_')
		return (0);
	s++;
	while (is_alnum(*s) || *s == '_') s++;
	return (p - s);
}
 
extern uint32_t		alnum_len(const char *s)
{
	const char *p = s;
	while (is_alnum(*p)) p++;
	return ((uint32_t)(p - s));
}

extern uint32_t		alpha_len(const char *s)
{
	const char *p = s;
	while (is_alpha(*p)) p++;
	return ((uint32_t)(p - s));
}

static char __attribute__((always_inline))		*left_trim(char *s, int32_t *type)
{
	while (*s == ' ' || *s == '\t')
		s++;
	if (*s == '0')
	{
		if (s[1] == 'x')
		{
			*type = 2;
			s += 2;
			while (*s == '0')
				s++;
			return (s);
		}
		*type = 1;
		s++;
		while (*s == '0')
			s++;
		return (s);
	}
	else
	{
		*type = 0;
		if (*s < '0' || *s > '9')
			return (NULL);
	}
	return (s);
}
/*
static uint32_t __attribute__((always_inline))	get_base_value(char c)
{
	if (LOWER(c) >= 'a' && LOWER(c) <= 'f')
		return (c - 87);
	return (c - '0');
}
*/
static const char		*ft_strtoi(const char *s, uint32_t *value, int32_t type)
{
	uint32_t	n = 0;

	if (type == 0)
	{
		while (1)
		{
			if (*s < '0' || *s > '9')
				break ;
			n *= 10;
			n += *(s++) - '0';
		}
	}
	else if (type == 1)
	{
		while (1)
		{
			if (*s < '0' || *s > '6')
				break ;
			n *= 8;
			n += *s - '0';
			s++;
		}
	}
	else
	{
		while (1)
		{
			register char	lower = LOWER(*s);
			if (!is_alnum(lower) && (lower < 'a' || lower > 'f'))
				break ;
			n *= 16;
			n += (lower >= 'a') ? lower - 87 : lower - '0';
			s++;
		}
	}
	*value = n;
	return (s);
}

static uint32_t		ft_strtoi_decimal(const char **s)
{
	uint32_t	n = 0;

	while (1)
	{
		if (**s < '0' || **s > '9')
			break ;
		n *= 10;
		n += **s - '0';
		(*s)++;
	}
	return (n);
}

static uint32_t		ft_strtoi_octal(const char **s)
{
	uint32_t	n = 0;

	while (1)
	{
		if (**s < '0' || **s > '6')
			break ;
		n *= 8;
		n += **s - '0';
		(*s)++;
	}
	return (n);
}

static uint32_t		ft_strtoi_hexa(const char **s, uint8_t prefix)
{
	uint32_t	n = 0;

	while (1)
	{
		register char	lower = to_lower_char[(uint8_t)(**s)];

		if (!is_digit(lower) && (lower < 'a' || lower > 'f'))
			break ;
		n *= 16;
		n += (lower >= 'a') ? lower - 87 : lower - '0';
		(*s)++;
	}
	if (!prefix) {
		if (to_lower_char[(uint8_t)(**s)] == 'h')
			(*s)++;
	}
	return (n);
}

static uint32_t		ft_strtoi_binary(const char **s, uint8_t prefix)
{
	uint32_t	n = 0;

	while (1)
	{
		if (**s != '0' && **s != '1')
		{
			break ;
		}
		n *= 2;
		n += **s - '0';
		(*s)++;
	}
	if (!prefix) {
		if (to_lower_char[(uint8_t)(**s)] == 'b')
			(*s)++;
	}
	return (n);
}



extern uint32_t		atou_type(const char *s, uint32_t *len, uint8_t type)
{
	uint32_t	result;
	uint8_t		prefix = 0;
//	uint8_t		_signed = 0;
	const char	*p = s;
/*	
	if (*s == '-') {
		_signed++;
		s++;
	}
*/
	if (type == HEXA_NUM)
	{
		if (*s == '0' && to_lower_char[(uint8_t)(s[1])] == 'x') {
			s += 2;
			prefix = 1;
		}
		result = ft_strtoi_hexa(&s, prefix);
	}
	else if (type == DECIMAL_NUM)
	{
		result = ft_strtoi_decimal(&s);
	}
	else if (type == OCTAL_NUM)
	{
		if (*s == '0') s++;
		result = ft_strtoi_octal(&s);
	}
	else if (type == BINARY_NUM)
	{
		if (*s == '0' && to_lower_char[(uint8_t)(s[1])] == 'b') {
			s += 2;
			prefix = 1;
		}
		result = ft_strtoi_binary(&s, prefix);
	}
	else
	{
		if (len) *len = 0;
		return (0);
	}
	if (len)
		*len = (uint32_t)(s - p);
//	return ((_signed) ? 0 - result : result);
	return (result);
}

extern uint32_t		atou_all(char *s, int32_t *err)
{
	int32_t		type; // 0 = base 10, 1 = octal, 2 = hexa
	uint32_t	result;

	if ((s = left_trim(s, &type)) == NULL)
	{
		if (err)
			*err = 1;
		return (0);
	}
	if (err)
		*err = 0;
	ft_strtoi(s, &result, type);
	return (result);
}

extern uint32_t		atou_inc_all(char **s, int32_t *err)
{
	int32_t		type; // 0 = base 10, 1 = octal, 2 = hexa
	uint32_t	result;
	char		*backup = *s;

	if ((*s = left_trim(*s, &type)) == NULL)
	{
		if (err)
			*err = 1;
		*s = backup;
		return (0);
	}
	if (err)
		*err = 0;
	*s = (char *)ft_strtoi(*s, &result, type);
	return (result);
}

static void		copy_without_comment(uint8_t **dst, uint32_t *len, uint8_t *buf)
{
	uint8_t		new[4096];
	uint32_t	newl = 0;

	while (*buf)
	{
		if (is_comment(*buf))
			while (*buf != '\n') buf++;
		else
			new[newl++] = *buf;
		buf++;
	}
	new[newl] = '\0';

	*dst = realloc(*dst, *len + newl + 1);
	memcpy(*dst + *len, new, newl + 1);
	*len += newl;
}

extern void		*get_file_contents(const char *path, uint32_t *length)
{
	uint8_t		*content = NULL;
	uint32_t	readl;
	FILE		*f;
	uint8_t		buf[4096];

	f = fopen(path, "r");
	if (f == NULL)
		return (NULL);
	fseek(f, 0, SEEK_END);
	*length = (uint32_t)ftell(f);
	rewind(f);
	if (*length == 0)
	{
		fprintf(stderr, "Empty file\n");
		return (NULL);
	}
	if (*length > FILE_MAX_LENGTH)
	{
		fprintf(stderr, "Too Heavy file\n");
		return (NULL);
	}
	*length = 0;

	while ((readl = fread(buf, 1, 4095, f)))
	{
		buf[readl] = '\0';
		copy_without_comment(&content, length, buf);
	}
	fclose(f);
	return (content);
}
