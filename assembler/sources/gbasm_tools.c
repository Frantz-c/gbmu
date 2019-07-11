#include "std_includes.h"

 
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
	}
	if (*s < '0' || *s > '9')
		return (NULL);
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

	if ((*s = left_trim(*s, &type)) == NULL)
	{
		if (err)
			*err = 1;
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
	content = valloc(*length);
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
