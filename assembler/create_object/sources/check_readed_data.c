
void	check_undefined_symbols(vector_t *label)
{
	register label_t	*l = VEC_ELEM_FIRST(label_t, label);

	for (uint32_t i = 0; i < label->nitems; i++, l++)
	{
		if (l->base_or_status == 0xffffffffu)
		{
			g_error++;
			fprintf(stderr, "\e[1;31mundefined symbol \e[0m\"%s\"\n", l->name);
		}
	}
}

void	check_code_area_overflow(vector_t *area)
{
	register code_area_t	*a;
	register uint32_t	end;
	
	if (area->nitems == 0)
		return;
	a = (code_area_t *)area->data;
	end = a->addr + a->size;
	a++;

	for (uint32_t i = 1; i < area->nitems; i++, a++)
	{
		if (end >= a->addr)
		{
			g_error++;
			fprintf(stderr, "overlap (end = 0x%x, start = 0x%x)\n", end, a->addr);
		}
	}
}

