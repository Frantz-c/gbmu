#include "std_includes.h"
#include "gbasm_struct.h"
#include "time.h"

uint8_t char_value[58];
/*
static uint8_t	char_value[58] = {
	  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,
	 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
	255,255,255,255,255,255,
	  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,
     13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25
};
*/
/*
static uint8_t	lower[58] = {
	 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
	32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
	64,'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
	91,92,93,94,95,96,
	'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
	123,124,125,126,127
};
*/
static int8_t	lower[58] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
	0,0,0,0,0,0,
	'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
	0,0,0,0,0
};


#define	legal_char(a)	lower[a]

extern char		*add_instruction_or_label(zones_t **zon, zones_t **curzon,
								defines_t *def[], char *s, error_t *err)
{
	char	*name;
	while (*s == ' ' || *s == '\t') s++;
	
	name = s;
	while (legal_char(*s)) s++;

	if (*s == ':')
	{
		s++;
		if (*s != ' ' && *s != '\t' && *s != '\n')
		{
			//error
			goto __error;
		}
		name = strndup(name, s - name);
		add_label(name, zon, curzon, err);
	}
	else if (*s != ' ' && *s != '\t' && *s != '\n')
	{
		//error
		goto __error;

		name = strndup(name, s - name);
		for (char *p = name; *p; p++)
			*p = lower[*p];
		add_instruction(name, zon, curzon, err);
	}
	return (s);

__error:
		fprintf(stderr, "bad token (%c)\n", *s);
		while (*s != '\n') s++;
		return (s);
}
#undef legal_char
