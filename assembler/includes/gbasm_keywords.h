#ifndef GBASM_KEYWORDS_H
# define GBASM_KEYWORDS_H

char	*bank_switch(vector_t *area, char *s, data_t *data);
char	*add_bytes(vector_t *area, char *s, data_t *data);
char	*set_memlock_area(vector_t *memblock, char *s, data_t *data);
char	*assign_var_to_memory(vector_t *memblock, char *s, data_t *data);
char	*set_extern_symbol(vector_t *symbol, char *s, data_t *data);

#endif
