#ifndef GBASM_KEYWORDS_H
# define GBASM_KEYWORDS_H

extern char	*bank_switch(zones_t **zon, zones_t **curzon, char *s, error_t *err);
extern char		*add_bytes(zones_t *curzon, char *s, error_t *err);

#endif
