
/*
 *	replace labels only
 */
void		replace_internal_labels(vector_t *area, loc_sym_t *local_symbol)
{
	register code_area_t	*a;

	a = (code_area_t *)area->data;
	for (uint32_t i = 0; i < area->nitems; i++, a++)
	{
		register code_t	*c = a->data;
		for (; c; c = c->next)
		{
			if (c->symbol && (c->size & 0xffffff00u) == 0)
			{
				// remplacement + calcul
				register ssize_t	index;

				if ((index = vector_search(local_symbol->label, (void*)&c->symbol)) != -1)
				{
					register label_t	*lab = VEC_ELEM(label_t, local_symbol->label, index);


					if (*c->opcode == JR || *c->opcode == JRZ || *c->opcode == JRNZ
						|| *c->opcode == JRC || *c->opcode == JRNC)
					{
						register int32_t	val = c->opcode[1] | (c->opcode[2] << 8);
						register int32_t	lab_addr = lab->base_or_status;

						lab_addr -= (int32_t)c->addr;
						val = (c->opcode[3] == '-') ? lab_addr - val : lab_addr + val;
						val -= 2;
						if (val > 0x7f || val < -128)
						{
							g_error++;
							fprintf(stderr, "too big jump (%d)\n", val);
						}
						c->opcode[1] = (uint8_t)val;
						c->opcode[2] = 0;
					}
					else
					{
						register uint32_t	lab_addr = lab->base_or_status;
						register uint32_t	val = c->opcode[1] | (c->opcode[2] << 8);

						if (lab_addr >= 0x8000)
							lab_addr = (lab_addr % 0x4000) + 0x4000;
						val = (c->opcode[3] == '-') ? lab_addr - val: lab_addr + val;
						c->opcode[1] = (uint8_t)val;
						c->opcode[2] = (val >> 8);
						if (val & 0xffff0000u)
						{
							g_error++;
							fprintf(stderr, "overflow label (0x%x)\n", val);
						}
					}
					free(c->symbol);
					c->symbol = NULL;
				}
			}
		}
	}
}

