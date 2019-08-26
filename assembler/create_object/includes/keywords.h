#ifndef GBASM_KEYWORDS_H
# define GBASM_KEYWORDS_H

void	bank_switch(vector_t *area, arguments_t args[], data_t *data);
void	add_bytes(vector_t *area, arguments_t args[], data_t *data);
void	set_memlock_area(vector_t *memblock, arguments_t args[], data_t *data);
void	assign_var_to_memory(loc_sym_t *loc_symbol, vector_t *ext_symbol, arguments_t args[], data_t *data);
void	set_extern_symbol(vector_t *symbol, arguments_t args[], data_t *data);

// cartridge

void	set_program_start(arguments_t args[], data_t *data);
void	set_game_title(arguments_t args[], data_t *data);
void	set_game_code(arguments_t args[], data_t *data);
void	set_cgb_support(arguments_t args[], data_t *data);
void	set_maker_code(arguments_t args[], data_t *data);
void	set_sgb_support(arguments_t args[], data_t *data);
void	set_cartridge_type(arguments_t args[], data_t *data);
void	set_rom_size(arguments_t args[], data_t *data);
void	set_ram_size(arguments_t args[], data_t *data);
void	set_code_dest(arguments_t args[], data_t *data);
void	version(arguments_t args[], data_t *data);

#endif
