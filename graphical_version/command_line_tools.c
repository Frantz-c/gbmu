
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <signal.h>
#include <termios.h>
#include <stdarg.h>
#include <time.h>
#include "memory_map.h"
#include "cheat.h"

extern int		term_noecho_mode(int stat)
{
	static int				mode = -1;
	static struct termios	default_mode;
	static struct termios	noecho_mode;

	if (mode == -1)
	{
		if (tcgetattr(0, &default_mode) == -1)
			return (-1);
		noecho_mode = default_mode;
		noecho_mode.c_lflag &= ~(ECHO | ICANON);
		noecho_mode.c_cc[VMIN] = 1;
		noecho_mode.c_cc[VTIME] = 5;
		mode = 0;
	}
	if (mode == stat)
		return (0);
	if (stat == 1)
		return (tcsetattr(0, TCSANOW, &noecho_mode));
	if (stat == 0)
		return (tcsetattr(0, TCSANOW, &default_mode));
	return (-1);
}

static inline char		*left_trim(char *s, int *type)
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

static unsigned int				get_base_value(char c)
{
	if (c >= 'a' && c <= 'f')
		return (c - ('a' - 10));
	if (c >= 'A' && c <= 'F')
		return (c - ('a' - 10));
	return (c - '0');
}

static inline unsigned int		ft_strtoi(char **s, int type)
{
	unsigned int	n;

	n = 0;
	if (type == 0)
	{
		while (1)
		{
			if (**s < '0' || **s > '9')
				break ;
			n *= 10;
			n += **s - '0';
			(*s)++;
		}
	}
	else if (type == 1)
	{
		while (1)
		{
			if (**s < '0' || **s > '6')
				break ;
			n *= 8;
			n += **s - '0';
			(*s)++;
		}
	}
	else
	{
		while (1)
		{
			if (**s > 'f' || (**s > 'F' && **s < 'a')
					|| (**s > '9' && **s < 'A') || **s < '0')
				break ;
			n *= 16;
			n += get_base_value(**s);
			(*s)++;
		}
	}
	return (n);
}

extern unsigned int		atoi_hexa(char **s, int *err)
{
	int	type; // 0 = base 10, 1 = octal, 2 = hexa

	if ((*s = left_trim(*s, &type)) == NULL)
	{
		if (err)
			*err = 1;
		return (0);
	}
	if (err)
		*err = 0;
	return (ft_strtoi(s, type));
}

extern int		non_alnum(char c)
{
	return (c < '0' || (c > '9' && c < 'A') || (c > 'Z' && c < 'a') || c > 'z');
}

extern void		write_dump_switchable_mem_fd(int fd, void *start, unsigned int length, char *buf, const char *title)
{
	unsigned int	j;
	uint8_t			*ptr;

	ptr = GET_REAL_ADDR((uint16_t)start);
	memcpy(buf, title, (j = strlen(title)));
	for (uint8_t *ptr_end = ptr + length; ptr != ptr_end; start++, ptr++)
	{
		if (((uint16_t)start & 0xf) == 0)
			j += sprintf(buf + j, "\n%p:  ", start);
		j += sprintf(buf + j, "%4hhx ", *ptr);
		if (j >= 0xfff0)
		{
			write(fd, buf, j);
			j = 0;
		}
	}
	if (j)
		write(fd, buf, j);
}

extern void		write_dump_fd(int fd, void *start, unsigned int length, char *buf, const char *title)
{
	unsigned int	j;

	memcpy(buf, title, (j = strlen(title)));
	for (void *end = start + length; start != end; start++)
	{
		if (((uint16_t)start & 0xf) == 0)
			j += sprintf(buf + j, "\n%p:  ", start);
		j += sprintf(buf + j, "%4hhx ", *GET_REAL_ADDR((uint16_t)start));
		if (j >= 0xfff0)
		{
			write(fd, buf, j);
			j = 0;
		}
	}
	if (j)
		write(fd, buf, j);
}

// 0 < min_arg <= max_arg
extern char		*va_parse_u32(char *p, int min_arg, int max_arg, ...)
{
	uint32_t	*tmp;
	int			cur = 0;
	int			err;
	va_list	ap;

	va_start(ap, max_arg);

	while (*p == ' ') p++;
	if (*p == '\0' && min_arg == 0)
	{
		tmp = va_arg(ap, uint32_t*);
		*tmp = 0xffffffffU;
		return (p);
	}
	if (*p != '(' && p[-1] != ' ')
		goto __error;
	if (*p == '(') {
		p++;
		while (*p == ' ') p++;
	}
	cur++;

	tmp = va_arg(ap, uint32_t*);
	*tmp = atoi_hexa(&p, &err);
	if (err) goto __error;

	while (cur < max_arg)
	{
		while (*p == ' ') p++;
		if (*p != ',' && p[-1] != ' ')
		{
			if (min_arg <= cur && non_alnum(*p))
			{
				while (cur < max_arg)
				{
					tmp = va_arg(ap, uint32_t*);
					*tmp = 0xffffffffU;
					cur++;
				}
				while (*p == ' ') p++;
				return (p);
			}
			goto __error;
		}
		if (*p == ',') {
			p++;
			while (*p == ' ') p++;
		}
		cur++;

		tmp = va_arg(ap, uint32_t*);
		*tmp = atoi_hexa(&p, &err);
		if (err) goto __error;
	}
	va_end(ap);
	while (*p == ' ') p++;
	return (p);

__error:
	va_end(ap);
	write(1, "syntax error\n", 13);
	return (NULL);
}

static void	free_hist(t_hist *hist)
{
	free(hist->s);
	free(hist);
}

extern void	load_hist(char *s, t_hist *h)
{
	memcpy(s, h->s, h->l + 1);
}

static t_hist	*new_hist(char *s, unsigned int l)
{
	t_hist *new;

	new = malloc(sizeof(t_hist));
	new->s = malloc(l + 1);
	memcpy(new->s, s, l + 1);
	new->l = l;
	new->prev = NULL;
	new->next = NULL;
	return (new);
}

extern void	add_hist(t_hist **hist, char *str, unsigned int len, unsigned int *hsize)
{
	t_hist	*tmp;
	t_hist	*new;

	new = new_hist(str, len);

	if (*hist == NULL)
	{
		*hist = new;
		*hsize += 1;
		return;
	}
	if (*hsize > MAX_HISTORY)
	{
		for (tmp = *hist; tmp->next->next; tmp = tmp->next);
		free_hist(tmp->next);
		tmp->next = NULL;
	}
	else
		*hsize += 1;
	tmp = *hist;
	*hist = new;
	new->next = tmp;
	tmp->prev = new;
}
