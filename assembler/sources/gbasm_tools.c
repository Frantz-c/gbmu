/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_tools.c                                    .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/12 23:05:07 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/18 02:05:37 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"
#include "gbasm_tools.h"

const uint32_t	ascii[256] = {
	0x40,0,0,0,0,0,0,0,0,2, //0
	0x40,0,0,0,0,0,0,0,0,0, //10
	0,0,0,0,0,0,0,0,0,0, //20
	0,0,2,0,0,0,0,0,0,0, //30
	0x80,0x80,4,4,0,0,0,0,1,1, //40
	1,1,1,1,1,1,1,1,0,0, //50
	0,0,0,0,0,0x18,0x18,0x18,0x18,0x18, //60
	0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18, //70
	0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18, //80
	0x18,0x80,0,0x80,0,0,0,0x28,0x28,0x28, //90
	0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28, //100
	0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28, //110
	0x28,0x28,0x28,0,0,0,0,0,0,0 //120
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

extern uint8_t		is_numeric(const char *s, uint32_t *len)
{
	uint8_t		type = 0;
	const char	*p = s;

	if (*s == '0' && (s[1] == 'x' || s[1] == 'X'))
	{
		type = HEXA_NUM;
		s += 2;
		while (is_digit(*s) || (*s >= 'a' && *s <= 'f') || (*s >= 'A' && *s <= 'F')) s++;
	}
	else if (*s == '0')
	{
		type = OCTAL_NUM;
		s++;
		while (*s >= '0' && *s <= '7') s++;
		if (is_digit(*s))
			goto __decimal;
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
			while (is_digit(*s) || (*s >= 'a' && *s <= 'f') || (*s >= 'A' && *s <= 'F')) s++;
			if (*s == 'h' || *s == 'H') s++;
		}
	}
	else if ((*s >= 'a' && *s <= 'f') || (*s >= 'A' && *s <= 'F'))
	{
		puts("HHHHEEEEXXXXAAA");
		goto __hexadecimal;
	}

	*len =  (uint32_t)(s - p);
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

static uint32_t __attribute__((always_inline))	get_base_value(char c)
{
	if (c >= 'a' && c <= 'f')
		return (c - ('a' - 10));
	if (c >= 'A' && c <= 'F')
		return (c - ('a' - 10));
	return (c - '0');
}

static char		*ft_strtoi(char *s, uint32_t *value, int32_t type)
{
	uint32_t	n;

	n = 0;
	if (type == 0)
	{
		while (1)
		{
			if (*s < '0' || *s > '9')
				break ;
			n *= 10;
			n += *s - '0';
			s++;
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
			if (*s > 'f' || (*s > 'F' && *s < 'a')
					|| (*s > '9' && *s < 'A') || *s < '0')
				break ;
			n *= 16;
			n += get_base_value(*s);
			s++;
		}
	}
	*value = n;
	return (s);
}

static uint32_t		ft_strtoi_decimal(char **s)
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

static uint32_t		ft_strtoi_octal(char **s)
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

static uint32_t	__attribute__((always_inline))	get_hexa_value(char c)
{
	if (c >= 'a')
		return (c - ('a' - 10));
	return (c - '0');
}

static uint32_t		ft_strtoi_hexa(char **s)
{
	uint32_t	n = 0;

	while (1)
	{
		register char	lower;
		if (!is_digit(**s) &&
			((lower = to_lower_char[(uint8_t)(**s)]) < 'a' || lower > 'f'))
		{
			break ;
		}
		n *= 16;
		n += get_hexa_value(lower);
		(*s)++;
	}
	if (to_lower_char[(uint8_t)(**s)] == 'h')
		(*s)++;

	return (n);
}

static uint32_t		ft_strtoi_binary(char **s)
{
	uint32_t	n = 0;

	while (1)
	{
		register char	lower;
		if (**s != '0' && **s != '1')
		{
			break ;
		}
		n *= 2;
		n += **s - '0';
		(*s)++;
	}
	if (to_lower_char[(uint8_t)(**s)] == 'b')
		(*s)++;

	return (n);
}



extern uint32_t		atou_type(char *s, uint32_t *len, uint8_t type)
{
	uint32_t	result;
	char		*p = s;
	
	if (type == HEXA_NUM)
	{
		if (*s == '0' && to_lower_char[(uint8_t)(s[1])] == 'x') s += 2;
		result = ft_strtoi_hexa(&s);
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
		if (*s == '0' && to_lower_char[(uint8_t)(s[1])] == 'b') s += 2;
		result = ft_strtoi_binary(&s);
	}
	if (len)
		*len = (uint32_t)(s - p);
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
	*s = ft_strtoi(*s, &result, type);
	return (result);
}

extern void		*get_file_contents(const char *path, uint32_t *length)
{
	void		*content;
	FILE		*f;

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
	content = valloc(*length + 1);
	if (content == NULL)
	{
		perror("allocation failed");
		return (NULL);
	}
	if (fread(content, 1, *length, f) != *length)
	{
		perror("read failed");
		free(content);
		return (NULL);
	}
	fclose(f);
	return (content);
}

