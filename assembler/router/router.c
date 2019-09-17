/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   router.c                                         .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <mhouppin@le-101.fr>               +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/08/12 13:43:03 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/16 22:02:05 by fcordon     ###    #+. /#+    ###.fr     */
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

	ajouter:

		./gbasm file.gbo file.gbs -o exe.gb
*/


typedef struct	src_obj_s
{
	char	*src;
	char	*obj;
}
src_obj_t;

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#define	VERSION		"1.0"
#define	HELP		"usage:\n%s -i [src] (-o [obj])\n%s -o [obj] -i [src]\n%s -o [exe] [obj]...\n%s [src]... -o [exe]\n\n"

#define CREATE_ONE_OBJECT		1	// generate one .o
#define	ASSEMBLE_OBJECTS		2	// generates a lot of .o
#define FULL_COMPILATION		4	// generate executable file directly

int		valid_extension(const char *s, const char *ext)
{
	const char	*start = s;
	s += strlen(s + 1);

	while (*s != '.' && s > start)
		s--;
	return (strcmp(s + 1, ext) == 0);
}

void	push_array(char ***a, char *new, uint32_t *len, uint32_t *max)
{
	if ((*len & 0x7) == 0)
	{
		*max += 8;
		*a = realloc(*a, (*max + 1) * sizeof(char*));
	}
	(*a)[*len] = new;
	(*len)++;
}

void	push_array_struct(src_obj_t **a, char *new_src, char *new_obj, uint32_t *len, uint32_t *max)
{
	if ((*len & 0x7) == 0)
	{
		*max += 8;
		*a = realloc(*a, (*max + 1) * sizeof(src_obj_t));
	}
	((*a)[*len]).src = new_src;
	((*a)[*len]).obj = new_obj;
	(*len)++;
}


uint8_t	get_action(char *argv[], char **exe, void **obj, void **src)
{
	uint32_t	i = 0;
	char		**o = NULL;//malloc(sizeof(char*) * 9);
	src_obj_t	*s = NULL;//malloc(sizeof(char*) * 9);
	uint32_t	olen = 0, omax = 0;
	uint32_t	slen = 0, smax = 0;

	// -i *.gbs -o *.gbo
	if (strcmp(argv[i], "-i") == 0)
	{
		i++;
		*src = argv[i];
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
	// -o exe *.gbo
	// *.gbs ... -o exe
	else
	{
		uint8_t		flag_o = 0;
		*exe = NULL;

		if (strcmp(argv[i], "-o") == 0)
		{
			i++;
			if (argv[i] && argv[i+1] && strcmp(argv[i+1], "-i") == 0)
				goto __flag_o__flag_i;
			flag_o = 1;
			if (argv[i] == NULL)
			{
				// ERROR "./gbasm -o"
				fprintf(stderr, "./gbasm -o\n");
				goto __free_and_exit;
			}
			*exe = argv[i++];
		}

		while (argv[i] && (flag_o || (argv[i][0] != '-' || argv[i][1] != 'o' || argv[i][2] != 0)))
		{
			if (valid_extension(argv[i], "gbs"))
			{
				char	*new_obj = strdup(argv[i]);


				new_obj[strlen(new_obj + 1)] = 'o';
				push_array_struct(&s, strdup(argv[i]), new_obj, &slen, &smax);
				push_array(&o, new_obj, &olen, &omax);
			}
			else if (valid_extension(argv[i], "gbo"))
			{
				char	*new_obj = strdup(argv[i]);
				push_array(&o, new_obj, &olen, &omax);
			}
			else
			{
				fprintf(stderr, "invalid extension file \"%s\"\n", argv[i]);
				goto __free_and_exit;
			}
			i++;
		}

		if (!flag_o)
		{
			if (argv[i] == NULL)
			{
				// ERROR "./gbasm file.gbs"
				fprintf(stderr, "./gbasm file.gbs\n");
				goto __free_and_exit;
			}
			i++;
			*exe = argv[i++];
			if (argv[i])
			{
				// ERROR "./gbasm file.gbs -o"
				fprintf(stderr, "./gbasm file.gbs -o\n");
				goto __free_and_exit;
			}
		}

		o[olen] = NULL;
		if (s)
			s[slen].src = NULL;
		*obj = o;
		*src = s;
		return (FULL_COMPILATION);
	}

	// -o *.gbo -i *.gbs
__flag_o__flag_i:
		*obj = strdup(argv[i]);
		i++;
		i++;
		if (argv[i] == NULL)
		{
			// ERROR "./gbasm -o file.gbo -i"
			goto __free_and_exit;
		}
		if (!valid_extension(argv[i], "gbs"))
		{
			// ERROR "./gbasm -o file.gbo -i error.toto"
			goto __free_and_exit;
		}
		*src = argv[i];
		return (CREATE_ONE_OBJECT);

__free_and_exit:
		if (olen)
		{
			for (uint32_t i = 0; i < olen; i++)
				free(o[i]);
		}
		free(o);
		if (slen)
		{
			for (uint32_t i = 0; i < slen; i++)
				free(o[i]);
		}
		free(s);
		exit(1);




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

uint32_t	count_tab(char *tab[])
{
	uint32_t	len;

	for (len = 0; tab[len]; len++);
	return (len);
}

// argv = obj, src
unsigned int	call_create_object(char *obj, char *src)
{
	int		process;
	int		status;

	process = fork();
	if (process == 0)
	{
		execl("gbasm_create_object", obj, src, NULL);
		fprintf(stderr, "can't find ./gbasm_create_object");
		exit(1);
	}
	wait(&status);
	if (WIFSIGNALED(status))
	{
		printf("CHILD TERMINATED WITH SIGNAL %u\n", WTERMSIG(status));
		return (1);
	}
	if (WIFEXITED(status))
	{
		if (WEXITSTATUS(status) != 0)
			return (1);
	}
	return (0);
}

// argv = exe, obj...
unsigned int	call_assemble_objects(char **obj, char *exe)
{
	int			process;
	char		**argv = malloc((count_tab(obj) + 2) * sizeof(char *));
	uint32_t	i;
	int			status;

	argv[0] = exe;
	for (i = 0; obj[i]; i++)
		argv[i + 1] = obj[i];
	argv[i + 1] = NULL;

	if ((process = fork()) == 0)
	{
		execv("gbasm_assemble_objects", argv);
		fprintf(stderr, "can't find ./gbasm_assemble_objects");
		exit(1);
	}
	wait(&status);
	free(argv);

	if (WIFSIGNALED(status))
	{
		printf("CHILD TERMINATED WITH SIGNAL %u\n", WTERMSIG(status));
		return (1);
	}
	if (WIFEXITED(status))
	{
		if (WEXITSTATUS(status) != 0)
			return (1);
	}
	return (0);
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
		printf(HELP, argv[0], argv[0], argv[0], argv[0]);
		return (0);
	}

	action = get_action(argv + 1, &exe, &obj, &src);

	switch (action)
	{
		case CREATE_ONE_OBJECT:
		{
			printf("object = \"%s\", source = \"%s\"\n",
					(char*)obj, (char*)src);
			call_create_object(obj, src);
			free(obj);
			break;
		}
		case FULL_COMPILATION:
		{
			register char		**objects = (char **)obj;
			register src_obj_t	*sources = (src_obj_t *)src;

			printf("exe = \"%s\", ", (char*)exe);
			if (sources)
			{
				printf("sources = ");
				for (uint32_t i = 0; sources[i].src; i++)
				{
					printf("\"%s\" ", sources[i].src);
				}
			}
			if (objects)
			{
				printf(", objects = ");
				for (uint32_t i = 0; objects[i]; i++)
				{
					printf("\"%s\" ", objects[i]);
				}
			}
			printf("\n");

			if (sources)
			{
				for (uint32_t i = 0; sources[i].src; i++)
				{
					if (call_create_object(sources[i].obj, sources[i].src) != 0)
						exit(1);
					free(sources[i].src);
				}
			}
			call_assemble_objects(objects, exe);
			
			if (sources)
			{
				for (uint32_t i = 0; sources[i].src; i++)
					remove(sources[i].obj);
				free(sources);
			}
			for (uint32_t i = 0; objects[i]; i++)
				free(objects[i]);
			free(objects);
			break;
		}
	}
}
