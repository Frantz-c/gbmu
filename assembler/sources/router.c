/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   router.c                                         .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/08/12 13:43:03 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/08/12 17:00:03 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

/*
	generation de .o:

		./gbasm -i file.gbs (-o file.gbo)?
		./gbasm -o file.gbo -i file.gbs ...

	generation de binaire:

		./gbasm -o file.gb file.gbo ...
		./gbasm file.gbs -o file.gb
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define	VERSION		"1.0"
#define	HELP		"(help)"

#define CREATE_ONE_OBJECT				1	// generate one .o
#define	MULTIPLE_OBJECTS_COMPILATION	2	// generates a lot of .o
#define FULL_COMPILATION				3	// generate executable file directly

int	valid_extension(const char *s, const char *ext)
{
	const char	*start = s;
	s += strlen(s + 1);

	while (*s != '.' && s > start)
		s--;
	return (strcmp(s + 1, ext) == 0);
}

uint8_t	get_action(char *argv[], char **exe, void **obj, void **src)
{
	uint32_t	i = 0;

	// -i *.gbs -o *.gbo
	if (strcmp(argv[i], "-i") == 0)
	{
		i++;
		*src = argv + i;
		if (argv[i] == NULL || !valid_extension(argv[i], "gbs"))
			goto __error_extension;
		i++;
		
		if (argv[i] == NULL)
		{
			*obj = (void*)strdup((char*)*src);
			((char*)(*obj))[strlen((char*)*obj) - 1] = 'o';
		}
		else if (strcmp(argv[i], "-o") == 0)
		{
			i++;
			if (argv[i] == NULL || !valid_extension(argv[i], "gbo"))
				goto __error_extension;
			*obj = strdup(argv[i]);
			if (argv[i + 1] != NULL)
			{
				free(*obj);
				goto __invalid_param;
			}
		}
		else
			goto __invalid_param;
		return (CREATE_ONE_OBJECT);
	}
	// -o *.gbo -i *.gbs
	// -o exe *.gbo
	else if (strcmp(argv[i], "-o") == 0)
	{
		i++;
		if (argv[i] == NULL || !valid_extension(argv[i], "gbo"))
		{
		__multiple_objects_compilation:
			// -o exe *.gbo
			*exe = argv[i];
			i++;
			*obj = argv + i;
			while (argv[i] != NULL)
			{
				if (!valid_extension(argv[i], "gbo"))
					goto __invalid_extension;
				i++;
			}
			return (MULTIPLE_OBJECTS_COMPILATION);
		}

		// -o *.gbo -i *.gbs
		*obj = (void*)strdup(argv[i]);
		i++;
		if (argv[i] == NULL)
		{
			free(*obj);
			goto __i_expected;
		}
		else if (strcmp(argv[i], "-i") == 0)
		{
			i++;
			if (argv[i] == NULL || !valid_extension(argv[i], "gbs"))
				goto __error_extension;
			*src = argv[i];
			i++;
			if (argv[i] != NULL)
			{
				free(*obj);
				goto __invalid_param;
			}
		}
		else
		{
			free(*obj);
			i = 1;
			goto __multiple_objects_compilation;
		}
		return (CREATE_ONE_OBJECT);
	}
	// *.gbs ... -o exe
	else
	{
		*src = argv + i;
		while (argv[i] != NULL && valid_extension(argv[i], "gbs")) i++;
		if (i == 0)
			goto __invalid_param;
		if (argv[i] == NULL)
			goto __invalid_param;
		if (strcmp(argv[i], "-o") == 0)
		{
			argv[i] = NULL;
			i++;
			if (argv[i] == NULL)
				goto __invalid_param;
			i++;
			*exe = argv[i];
			i++;
			if (argv[i] != NULL)
				goto __invalid_param;
		}
		else
			goto __invalid_param;

		register char		**p = *src;
		register char		**o = malloc(i - 1 * sizeof(char*));
		for (uint32_t j = 0; ; j++)
		{
#if	defined(__LINUX__) || defined(_SYSTYPE_BSD)
			o[j] = strdup(p[j]);
			o[j][strlen(p[j] + 1)] = 'o';
#elif defined(_WIN32)
			printf("WINDOWS\n");
			exit(1);
#else
			printf("ERROR\n");
			exit(1);
#endif
			if (p[j] == NULL)
			{
				o[j] = NULL;
				break;
			}
		}
		*obj = o;
	}
	return (FULL_COMPILATION);

__i_expected:
	fprintf(stderr, "-i expected");
__invalid_param:
	fprintf(stderr, "invalid argument %s\n", argv[i]);
	goto __exit;
__invalid_extension:
__error_extension:
	fprintf(stderr, "file %s: invalid extension\n", argv[i]);
__exit:
	exit(1);
}

int		main(int argc, char *argv[])
{
	uint8_t		action = 0;
	char		*exe = NULL;
	void		*obj = NULL;
	void		*src = NULL;

	if (argc == 1)
	{
		printf("GBASM VERSION %s\n", VERSION);
		puts(HELP);
		return (0);
	}

	action = get_action(argv + 1, &exe, &obj, &src);
	printf("action = %u\n", action);
}
