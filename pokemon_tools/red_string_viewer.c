
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

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

static inline unsigned int		ft_strtoi(char *s, int type)
{
	unsigned int	n;

	n = 0;
	if (type == 0)
	{
		while (1)
		{
			if (*s < '0' || *s > '9')
				break ;
			n *= 10;
			n += *s - '0';
			(s)++;
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
			(s)++;
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
			(s)++;
		}
	}
	return (n);
}

static unsigned int		atoi_hexa(char *s, int *err)
{
	int	type; // 0 = base 10, 1 = octal, 2 = hexa

	if ((s = left_trim(s, &type)) == NULL)
	{
		if (err)
			*err = 1;
		return (0);
	}
	if (err)
		*err = 0;
	return (ft_strtoi(s, type));
}

unsigned char	*get_file_contents(const char *file, unsigned int *length)
{
	unsigned char	*content;
	const int		fd = open(file, O_RDONLY);
	struct stat		filestat;

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


static const char *const	kana[256] = {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	"A","B","C","D","E","F","G","H","I","J","K","L","M",
	"N","O","P","Q","R","S","T","U","V","W","X","Y","Z",
	"(", ")", ":", ";", "[", "]",
	"a","b","c","d","e","f","g","h","i","j","k","l","m",
	"n","o","p","q","r","s","t","u","v","w","x","y","z",
	"à","è","é","ù","ß","ç","Ä","Ö","Ü","ä","ö","ü","ë",
	"ï","â","ô","û","ê","î"," ",
	NULL, NULL, NULL, NULL, NULL, NULL,
	"c'","d'","j'","l'","m'","n'","p'","s'","'s","t'","u'",
	"y'","'", NULL, NULL, "-", "+", " ", "?", "!", ".",
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL,
	"0","1","2","3","4","5","6","7","8","9"
};

int		space_only(char **s)
{
	char	*p;

	while (**s)
	{
		if (**s != ' ')
			break;
		(*s)++;
	}
	if (**s == '\0')
		return (1);
	p = *s;
	while (*p) p++;
	p--;
	while (*p == ' ')
	{
		*p = '\0';
		p--;
	}
	return (0);
}

#define BUFSIZE	64

int		main(int argc, char *argv[])
{
	unsigned char	*file;
	char			*str;
	unsigned int	length;
	unsigned int	i;
	char			buf[BUFSIZE];
	unsigned int	count;
	unsigned int	end;
	int				err[2] = {0};
	int				link = 0;
	unsigned int	j;

	if (argc < 2 || argc > 4)
	{
		fprintf(stderr, "ERROR\n");
		exit(1);
	}
	file = get_file_contents(argv[1], &length);
	if (file == NULL)
		return (1);

	if (argc > 2)
		i = atoi_hexa(argv[2], err + 0);
	else
		i = 0;

	if (argc == 4)
		end = atoi_hexa(argv[3], err + 1);
	else
		end = length;

	if (*(unsigned long*)err)
		return (1);

	for (; i != end; i++)
	{
		if (kana[file[i]])
		{
			for (j = 0, count = 0; ; count++, i++)
			{
				if (i == end)
					return (0);
				if (kana[file[i]] == NULL || j >= (BUFSIZE - 4)) {
					if (j && (link || count > 2))
					{
						buf[j] = '\0';
						str = buf;
						if (!space_only(&str)) {
							if (link)
								printf("%s", str);
							else
								printf("\n0x%x: %s", i - count, str);
						}
					}
					break;
				}
				memcpy(buf + j, kana[file[i]], strlen(kana[file[i]]));
				j += strlen(kana[file[i]]);
			}
			if (kana[file[i]])
				i--;
			if (file[i - 1] > 4)
				link = 1;
		}
		else
		{
			link = 0;
		}
	}
}
