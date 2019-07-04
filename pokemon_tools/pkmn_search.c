
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

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

int	main(int argc, char *argv[])
{
	unsigned char	*file;
	unsigned int	l;

	if (argc != 3)
		exit (1);

	unsigned int	i = 0;
	unsigned int	j = (unsigned int)atoi(argv[2]);
	file = get_file_contents(argv[1], &l);
	while (i + (j * 14) < l)
	{
		if (file[i] == 1 && file[i + j] == 2 && file[i + (j * 2)] == 3
			&& file[i + (j * 3)] == 4 && file[i + (j * 4)] == 5 && file[i + (j * 5)] == 6
			&& file[i + (j * 6)] == 7 && file[i + (j * 7)] == 8 && file[i + (j * 8)] == 9
			&& file[i + (j * 9)] == 10 && file[i + (j * 10)] == 11 && file[i + (j * 11)] == 12
			&& file[i + (j * 12)] == 13 && file[i + (j * 13)] == 14 && file[i + (j * 14)] == 15)
			printf("occurence -> 0x%x\n", i);
		i++;
	}
}
