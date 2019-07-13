#include "../includes/std_includes.h"
#include "../includes/gbasm_struct.h"

extern char		*copy_macro_content(char *dest, char *s, uint32_t *lineno)
{
	char	*bs_pos;

	for (; ; s++)
	{
		if (*s == '\n') {
			break;
		}
		else if (*s == '\\') {
			bs_pos = s;
			s++;
			if (*s == ' ' || *s == '\t') s++;
			if (*s == '\n')
			{
				*(dest++) = *(s++);
				if (*s == ' ' || *s == '\t')
				{
					while (s[1] == ' ' || s[1] == '\t') s++;
				}
				(*lineno)++;
				continue;
			}
			*(dest++) = '\\';
			s = bs_pos;
			continue;
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
		if (*s == '\n') {
			break;
		}
		else if (*s == '\\')
		{
			bs_pos = s;
			s++;
			if (*s == ' ' || *s == '\t') (s)++;
			if (*s == '\n')
			{
				s++;
				if (*s == ' ' || *s == '\t')
				{
					while (s[1] == ' ' || s[1] == '\t') s++;
				}
				count++;
				continue;
			}
			s = bs_pos;
			count++;
			continue;
		}
	}

	return (count);
}

extern void	skip_macro(char **s, uint32_t *lineno)
{
	char	*bs_pos;

	for (; ; (*s)++)
	{
		if (**s == '\n' || **s == '\0')
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
