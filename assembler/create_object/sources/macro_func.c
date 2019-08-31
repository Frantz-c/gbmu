#include "std_includes.h"
#include "struct.h"
#include "tools.h"

extern char		*copy_macro_content(char *dest, char *s, uint32_t *lineno)
{
	char	*bs_pos;

	for (; ; s++)
	{
		if (is_endl(*s))
			break;
		if (*s == '\\')
		{
			bs_pos = s;
			s++;
			if (is_space(*s)) s++;
			if (*s == '\n')
			{
				*(dest++) = *(s++);
				if (is_space(*s))
					while (is_space(s[1])) s++;
				(*lineno)++;
				continue;
			}
			*(dest++) = '\\';
			s = bs_pos;
			continue;
		}
		if (is_space(*s))
		{
			while (is_space(s[1])) s++;
			*(dest++) = ' ';
		}
		else
			*(dest++) = *s;
	}
	*dest = '\0';
	return (s);
}

extern uint32_t	get_macro_content_length(char *s)
{
	uint32_t	count = 1;
	char		*bs_pos;

	for (; ; s++, count++)
	{
		if (is_endl(*s))
			break;
		if (*s == '\\')
		{
			bs_pos = s;
			s++;
			if (is_space(*s)) (s)++;
			if (*s == '\n')
			{
				s++;
				if (is_space(*s))
					while (is_space(s[1])) s++;
				count++; //?
				continue;
			}
			s = bs_pos;
			count++;
			continue;
		}
		else if (!is_alnum(*s) && !is_space(*s) && *s != '_' && *s != ',' && !is_parent(*s) && !is_operator(*s))
		{
			return ((count - 1) | 0x80000000u);
		}
	}

	return (count);
}

extern void	skip_macro(char **s, uint32_t *lineno)
{
	char	*bs_pos;

	for (; ; (*s)++)
	{
		if (is_endl(**s))
		{
			break;
		}
		else if (**s == '\\')
		{
			bs_pos = *s;
			(*s)++;
			if (**s == ' ' || **s == '\t') (s)++;
			if (**s == '\n')
			{
				s++;
				(*lineno)++;
				continue;
			}
			*s = bs_pos;
			continue;
		}
	}
}
