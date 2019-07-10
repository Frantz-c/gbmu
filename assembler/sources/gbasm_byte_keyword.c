#include "std_includes.h"
#include "gbasm_struct.h"
#include "gbasm_tools.h"
#include "gbasm_struct_tools.h"
#include "gbasm_macro_func.h"

#define INT_TO_STRPTR(exp)	(char*)((long)(exp))

extern char		*add_bytes(zones_t *curzon, char *s, error_t *err)
{
	uint32_t	byte;
	int32_t		error;


	new_instruction(curzon, strdup("$"));
	do
	{
		while (*s == ' ' || *s == '\t') s++;
		byte = atou_inc_all(&s, &error);
		if (byte > 0xffu)
		{
			// error size;
			fprintf(stderr, "size error: number truncated (%hhu)\n", (uint8_t)(byte));
		}
		if (error == 1)
		{
			fprintf(stderr, "syntax error\n");
			skip_macro(&s);
			return (s);
		}

		push_operand(curzon, INT_TO_STRPTR(byte & 0xffu));

		while (*s == ' ' || *s == '\t') s++;
		if (*s == '\\')
		{
			s++;
			while (*s)
			{
				if (*s == '\n') {
					s++;
					break;
				}
				else if (*s != ' ' && *s != '\t')
					goto __error_token;
				s++;
			}
		}
		else if (*s == ',')
			s++;
		else if (*s != ' ' && *s != '\t')
		{
			if (*s != '\n' && *s != '\0')
			{
				//error
			__error_token:
				err->error++;
				fprintf(stderr, "bad token '%c'\n", *s);
				skip_macro(&s);
				break;
			}
			return (s);
		}
	}
	while (1);

	while (*s != '\n') s++;
	return (s);
}
