#ifndef GBASM_MACRO_FUNC_H
# define GBASM_MACRO_FUNC_H

//extern void		push_macro(defines_t **def, char *name, char *content, uint32_t count);
//extern int		macro_exists(defines_t *p, char *name);
extern void		skip_macro(char **s, uint32_t *lineno);
extern char		*copy_macro_content(char *dest, char *s, uint32_t *lineno);
extern uint32_t	get_macro_content_length(char *s);

#endif
