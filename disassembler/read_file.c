/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   read_file.c                                      .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/05/24 14:40:04 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/06/04 12:32:24 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>

static inline const char		*left_trim(const char *s)
{
	while (*s == ' ' || *s == '\t')
		s++;
	if (*s == '0' && s[1] == 'x')
	{
		s += 2;
		while (*s == '0')
			s++;
		return (s);
	}
	return (NULL);
}

static unsigned int				get_base_value(char c)
{
	if (c >= 'a' && c <= 'f')
		return (c - ('a' - 10));
	if (c >= 'A' && c <= 'F')
		return (c - ('a' - 10));
	return (c - '0');
}

static inline unsigned int		ft_strtoi(const char *s)
{
	unsigned int	n;

	n = 0;
	while (1)
	{
		if (*s > 'f' || (*s > 'F' && *s < 'a')
				|| (*s > '9' && *s < 'A') || *s < '0')
			break ;
		n *= 16;
		n += get_base_value(*(s++));
	}
	return (n);
}

extern inline unsigned int		atoi_hexa(const char *s)
{
	if ((s = left_trim(s)) == NULL)
		return (0);
	return (ft_strtoi(s));
}

unsigned char	*get_file_contents(const char *file, unsigned int *length)
{
	unsigned char		*content;
	const int	fd = open(file, O_RDONLY);
	struct stat	filestat;

	if (fd == -1)
	{
		fprintf(stderr, "can't open file %s\n", file);
		return (NULL);
	}
	lstat(file, &filestat);
	if (filestat.st_size == 0)
	{
		fprintf(stderr, "empty file\n");
		return (NULL);
	}
	if ((content = malloc(filestat.st_size + 1)) != NULL)
	{
		*length = read(fd, content, filestat.st_size);
		close(fd);
	}
	return (content);
}

void	print_help(const char *prog)
{
		fprintf(stderr, "%s [option]? \"file\" ([start offset] [end offset]?)?\n\n"
						"options:\n"
						"  without verbose\n"
						"    --bin: binary\n"
						"    --hex: hexa\n"
						"    --dec: decimal\n"
						"    --chr: ascii\n"
						"    --mixed: hexa + ascii\n\n"
						"  with verbose\n"
						"    --vbin: binary\n"
						"    --vhex: hexa\n"
						"    --vdec: decimal\n"
						"    --vchr: ascii\n"
						"    --vmixed: hexa + ascii\n\n", prog);
}

char	*get_bin(unsigned char n)
{
	static char					buf[128] = {0};
	unsigned char				curs;
	unsigned int				i = 0;

	curs = 1UL << 7;
	while (curs)
	{
		buf[i++] = (curs & n) ? '1' : '0';
		curs >>= 1;
	}
	buf[i] = 0;
	return (buf);
}

char	*get_hex(unsigned char n)
{
	static char	buf[3];

	buf[2] = 0;
	buf[1] = n % 16 + '0';
	if (buf[1] > '9')
		buf[1] += 39;
	n /= 16;
	buf[0] = n + '0';
	if (buf[0] > '9')
		buf[0] += 39;

	return (buf);
}

void	print_addr(unsigned int addr, int base)
{
	static char	*fmt = NULL;

	if (!fmt)
	{
		switch (base)
		{
			case 2:
			case 'h':
			case 16: fmt = "0x%-8x: "; break;
			case 'c':
			case 10: fmt = "%-10u: "; break;
		}
	}
	dprintf(2, "\e[0;33m");
	dprintf(1, fmt, addr);
	dprintf(2, "\e[0m");
}

void	print_line(char *buf, int base, unsigned int addr, int verbose)
{
	if (*buf)
		dprintf(1, "%s\n", buf);
	if (verbose)
		print_addr(addr, base);
}

unsigned int	add_octet(char *buf, unsigned int i, const unsigned char *byte, int base)
{
/*
	static char	*fmt = NULL;

	if (!fmt)
	{
		switch (base)
		{
			case 2: fmt = (void*)1; break;
			case 10: fmt = "%3u "; break;
		//	case 16: fmt = "%x "; break;
			case 'c': fmt = "%c"; break;
		}
	}
	*/
	if (base == 2)
		return (i + sprintf(buf + i, "%s ", get_bin(*byte)));
	else if (base == 'c')
	{
	   	if (!isprint(*byte))
			return (i + sprintf(buf + i, " \e[1;35m%3u\e[0m ", *byte));
		return (i + sprintf(buf + i, "%c", *byte));
	}
	else if (base == 16)
		return(i + sprintf(buf + i, "%s ", get_hex(*byte))); 
	else if (base == 'h')
	{
	   	if (!isprint(*byte))
			return (i + sprintf(buf + i, "%s ", get_hex(*byte)));
		return (i + sprintf(buf + i, "\e[0;33m%c\e[0m  ", *byte));
	}
	return (i + sprintf(buf + i, "%-3u ", *byte));
}

void	print_file(unsigned char *file, unsigned int start, unsigned int end, int base, int verbose)
{
	char			buf[512] = {0};
	unsigned int	i;

	i = 0;
	if (verbose && (start & 0xf))
		print_addr(start, base);
	while (start < end)
	{
		if ((start & 0xf) == 0)
		{
			print_line(buf, base, start, verbose);
			i = 0;
		}
		i = add_octet(buf, i, file + start, base);
		start++;
	}
	if (i)
		print_line(buf, base, start, verbose);
}

int		main(int ac, char *av[])
{
	unsigned char	*file;
	unsigned int	len;
	unsigned int	start, end;
	int				base = 0;
	int				verbose = 1;
	unsigned int	i = 1;

	if (ac < 2 || ac > 5)
	{
		print_help(av[0]);
		return (1);
	}
	base = 0;
	if (strcmp(av[1], "--bin") == 0) {
		base = 2;
		verbose = 0;
	}
	else if (strcmp(av[1], "--hex") == 0) {
		base = 16;
		verbose = 0;
	}
	else if (strcmp(av[1], "--dec") == 0) {
		base = 10;
		verbose = 0;
	}
	else if (strcmp(av[1], "--chr") == 0) {
		base = 'c';
		verbose = 0;
	}
	else if (strcmp(av[1], "--mixed") == 0) {
		base = 'h';
		verbose = 0;
	}
	else if (strcmp(av[1], "--vbin") == 0)
		base = 2;
	else if (strcmp(av[1], "--vhex") == 0)
		base = 16;
	else if (strcmp(av[1], "--vdec") == 0)
		base = 10;
	else if (strcmp(av[1], "--vchr") == 0)
		base = 'c';
	else if (strcmp(av[1], "--vmixed") == 0)
		base = 'h';

	if (base)
		i++;
	else
	{
		if (ac == 5)
		{
			print_help(av[0]);
			return (1);
		}
	}

	if ((file = get_file_contents(av[i], &len)) == NULL)
		return (1);
	start = 0U;
	end = len;
	if (ac > i + 1)
	{
		if (av[i + 1][0] == '0' && av[i + 1][1] == 'x')
			start = (unsigned int)atoi_hexa(av[i + 1]);
		else
			start = (unsigned int)atoi(av[i + 1]);
	}
	if (ac > i + 2)
	{
		if (av[i + 2][0] == '0' && av[i + 2][1] == 'x')
			end = (unsigned int)atoi_hexa(av[i + 2]);
		else
			end = (unsigned int)atoi(av[i + 2]);
	}
	if (len < end)
	{
		fprintf(stderr, "end offset (%d) > file end (%d)\n", end, len);
		return (1);
	}
	if (start > end)
	{
		fprintf(stderr, "start offset (%d) > end offset (%d)\n", start, end);
		return (1);
	}
	if (!base)
		base = 16;

	print_file(file, start, end, base, verbose);
	puts("");

	return (0);
}
