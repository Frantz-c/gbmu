/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   gbasm_tools.c                                    .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/12 23:05:07 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/13 22:27:17 by fcordon     ###    #+. /#+    ###.fr     */
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
	0,0,0,0,0,0,0,0,1,1, //40
	1,1,1,1,1,1,1,1,0,0, //50
	0,0,0,0,0,0x18,0x18,0x18,0x18,0x18, //60
	0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18, //70
	0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18, //80
	0x18,0,0,0,0,0,0,0x28,0x28,0x28, //90
	0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28, //100
	0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28, //110
	0x28,0x28,0x28,0,0,0,0,0,0,0 //120
};

extern uint8_t		is_numeric(const char *s, uint32_t *len)
{
	uint8_t	type = 0;
	char	*p = s;

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
	}
	else if (is_digit(*s))
	{
		type = DECIMAL_NUM;
		s++;
		while (is_digit(*s)) s++;
	}
	*len =  (uint32_t)(p - s);
	return (type)
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

static char
__attribute__((always_inline))
*left_trim(char *s, int32_t *type)
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

 __attribute__((always_inline))
static uint32_t				get_base_value(char c)
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

