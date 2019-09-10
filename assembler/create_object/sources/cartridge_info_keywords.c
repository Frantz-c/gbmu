/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   cartridge_info_keywords.c                        .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/08/23 22:24:16 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/09/10 15:09:37 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */

#include "std_includes.h"

static void __attribute__((always_inline))
to_lower_string(char *s)
{
	while (*s)
	{
		*s = LOWER(*s);
		s++;
	}
}

void	set_program_start(arguments_t args[], data_t *data)
{
	uint32_t	val;

	if (cart_info.member.program_start)
	{
		print_warning(data->filename, data->lineno, data->line,
						"duplicate keyword .program_start");
	}

	if (args[1].value)
		goto __too_many_arguments;
	if (args[0].value == NULL)
		goto __too_few_arguments;
	if (args->type != INTEGER_TYPE)
		goto __wrong_type;

	val = *(uint32_t*)(args->value);
	if (val & 0xffff0000)
		goto __overflow;
	
	cartridge.program_start[1] = (val & 0xff00u) >> 8;
	cartridge.program_start[0] = val & 0xffu;
	cart_info.member.program_start = 1;
	return;

	{
		const char	*error;
	__too_few_arguments:
		error = "argument 1 expected";
		goto __print_error;
	__too_many_arguments:
		error = "only one argument is expected";
		goto __print_error;
	__wrong_type:
		error = "argument 1 should be an integer";
		goto __print_error;
	__overflow:
		sprintf(data->buf, "argument 1 overflow (value is 0x%x, max is 0xffff)", val);
		error = (const char *)data->buf;
	__print_error:
		print_error(data->filename, data->lineno, data->line, error);
	}
	return;
}

void	set_game_title(arguments_t args[], data_t *data)
{
	if (cart_info.member.game_title)
	{
		print_warning(data->filename, data->lineno, data->line,
						"duplicate keyword .game_title");
	}


	if (args[1].value)
		goto __too_many_arguments;
	if (args[0].value == NULL)
		goto __too_few_arguments;
	if ((args->type & STRING_TYPE) == 0)
		goto __wrong_type;
	if ((args->type & GB_STRING_TYPE) == 0)
		goto __not_well_formated_string;

	register char		*src = (char *)args->value;
	register uint32_t	len = strlen(src);
	if (len > 11)
		goto __too_many_characters;

	register char	*dst = (char*)cartridge.game_title;
	memcpy(dst, src, len);
	bzero(dst + len, 11 - len);
	cart_info.member.game_title = 1;
	return;

	{
		const char	*error;
		__not_well_formated_string:
			error = "argument 1 not well formated. expected characters are ascii 32 - 95";
			goto __print_error;
		__too_few_arguments:
			error = "argument 1 expected";
			goto __print_error;
		__too_many_arguments:
			error = "only one argument is expected";
			goto __print_error;
		__wrong_type:
			error = "argument 1 should be a string (ascii 32 - 95)";
			goto __print_error;
		__too_many_characters:
			error = "argument 1 max length is 11 characters";
		__print_error:
			print_error(data->filename, data->lineno, data->line, error);
	}
	return;
}

void	set_game_code(arguments_t args[], data_t *data)
{
	if (cart_info.member.game_code)
	{
		print_warning(data->filename, data->lineno, data->line,
						"duplicate keyword .game_code");
	}

	if (args[1].value)
		goto __too_many_arguments;
	if (args[0].value == NULL)
		goto __too_few_arguments;
	if ((args->type & STRING_TYPE) == 0)
		goto __wrong_type;
	if ((args->type & GB_STRING_TYPE) == 0)
		goto __not_well_formated_string;
	
	register char		*src = (char *)args->value;
	register uint32_t	len = strlen(src);
	if (len > 4)
		goto __too_many_characters;

	register char	*dst = (char*)cartridge.game_code;
	memcpy(dst, src, len);
	bzero(dst + len, 4 - len);
	cart_info.member.game_code = 1;
	return;

	{
		const char	*error;
		__not_well_formated_string:
			error = "argument 1 not well formated. expected characters are ascii 32 - 95";
			goto __print_error;
		__too_few_arguments:
			error = "argument 1 expected";
			goto __print_error;
		__too_many_arguments:
			error = "only one argument is expected";
			goto __print_error;
		__wrong_type:
			error = "argument 1 should be a string (ascii 32 - 95)";
			goto __print_error;
		__too_many_characters:
			error = "argument 1 max length is 4 characters";
		__print_error:
			print_error(data->filename, data->lineno, data->line, error);
	}
}

#define CGB_EXCLU	0xC0u
#define CGB_COMP	0x80u
#define CGB_INCOMP	0x00u

void	set_cgb_support(arguments_t args[], data_t *data)
{
	if (cart_info.member.cgb_support)
	{
		print_warning(data->filename, data->lineno, data->line,
						"duplicate keyword .cgb_support");
	}

	if (args[1].value)
		goto __too_many_arguments;
	if (args[0].value == NULL)
		goto __too_few_arguments;
	if (args->type & STRING_TYPE)
	{
		to_lower_string((char *)args->value);
		if (strcmp((char *)args->value, "exclusive") == 0)
			cartridge.cgb_support = CGB_EXCLU;
		else if (strcmp((char *)args->value, "compatible") == 0)
			cartridge.cgb_support = CGB_COMP;
		else if (strcmp((char *)args->value, "incompatible") == 0)
			cartridge.cgb_support = CGB_INCOMP;
		else
			goto __unknown_value;
	}
	else
	{
		register uint32_t	value = *(uint32_t*)(args->value);

		if (value != CGB_EXCLU && value != CGB_COMP && value != CGB_INCOMP)
			goto __unknown_value;
		cartridge.cgb_support = value;
	}
	cart_info.member.cgb_support = 1;
	return;

	{
		const char	*error;
		__too_few_arguments:
			error = "argument 1 expected";
			goto __print_error;
		__too_many_arguments:
			error = "only one argument is expected";
			goto __print_error;
		__unknown_value:
			error = "forbidden value for argument 1";
		__print_error:
			print_error(data->filename, data->lineno, data->line, error);
	}
}

void	set_maker_code(arguments_t args[], data_t *data)
{
	if (cart_info.member.maker_code)
	{
		print_warning(data->filename, data->lineno, data->line,
						"duplicate keyword .maker_code");
	}

	if (args[1].value)
		goto __too_many_arguments;
	if (args[0].value == NULL)
		goto __too_few_arguments;
	if ((args->type & STRING_TYPE) == 0)
		goto __wrong_type;
	if ((args->type & GB_STRING_TYPE) == 0)
		goto __not_well_formated_string;

	register char		*src = (char *)args->value;
	register uint32_t	len = strlen(src);
	if (len > 2)
		goto __too_many_characters;

	register char	*dst = (char*)cartridge.maker_code;
	dst[0] = src[0];
	dst[1] = src[1];
	cart_info.member.maker_code = 1;
	return;

	{
		const char	*error;
		__not_well_formated_string:
			error = "argument 1 not well formated. expected characters are ascii 32 - 95";
			goto __print_error;
		__too_few_arguments:
			error = "argument 1 expected";
			goto __print_error;
		__too_many_arguments:
			error = "only one argument is expected";
			goto __print_error;
		__wrong_type:
			error = "argument 1 should be a string (ascii 32 - 95)";
			goto __print_error;
		__too_many_characters:
			error = "argument 1 max length is 2 characters";
		__print_error:
			print_error(data->filename, data->lineno, data->line, error);
	}
}

#define UNUSES_SGB_FUNC	0x00u
#define USES_SGB_FUNC	0x03u

void	set_sgb_support(arguments_t args[], data_t *data)
{
	if (cart_info.member.sgb_support)
	{
		print_warning(data->filename, data->lineno, data->line,
						"cart_info.member keyword .sgb_support");
	}

	if (args[1].value)
		goto __too_many_arguments;
	if (args[0].value == NULL)
		goto __too_few_arguments;
	if (args[0].type & STRING_TYPE)
	{
		to_lower_string((char *)args->value);
		printf("ADD_SGB_SUPPORT %s\n", (char*)args->value);
		if (strcmp((char *)args->value, "supported") == 0)
			cartridge.sgb_support = USES_SGB_FUNC;
		else if (strcmp((char *)args->value, "not_supported") == 0)
			cartridge.sgb_support = UNUSES_SGB_FUNC;
		else
			goto __unknown_value;
	}
	else
	{
		register uint32_t value = *(uint32_t*)args->value;

		if (value != 0u && value != 3u)
			goto __unknown_value;
		cartridge.sgb_support = value;
	}
	cart_info.member.sgb_support = 1;
	return;

	{
		const char	*error;
		__too_few_arguments:
			error = "argument 1 expected";
			goto __print_error;
		__too_many_arguments:
			error = "only one argument is expected";
			goto __print_error;
		__unknown_value:
			error = "forbidden value for argument 1";
		__print_error:
			print_error(data->filename, data->lineno, data->line, error);
	}
}

#define	SRAM		1
#define	BATTERY		2
#define RUMBLE		4
#define RTC			8

static uint16_t	get_mbc_options(char *s)
{
	uint8_t	o = 0;
	uint8_t	i;
	uint8_t	test = 0;

	if (!is_space(*s) && *s != '_')
		return (0x8000u);
	for (i = 0; ; i++)
	{
		if (i == 3)
			return (0xC000u);

		s++;
		if (strncmp(s, "sram", 4) == 0 &&
			(is_space(s[4]) || s[4] == '_' || s[4] == 0))
		{
			o |= SRAM;
			if (test & SRAM)
				return (0x4000u);
			test |= SRAM;
			s += 5;
		}
		else if (strncmp(s, "battery", 7) == 0 &&
			(is_space(s[7]) || s[7] == '_' || s[7] == 0))
		{
			o |= BATTERY;
			if (test & BATTERY)
				return (0x4000u);
			test |= BATTERY;
			s += 7;
		}
		else if (strncmp(s, "rtc", 3) == 0 &&
			(is_space(s[3]) || s[3] == '_' || s[3] == 0))
		{
			if (test & RTC || test & RUMBLE)
				return (0x4000u);
			o |= RTC;
			s += 3;
		}
		else if (strncmp(s, "rumble", 6) == 0 &&
			(is_space(s[6]) || s[6] == '_' || s[6] == 0))
		{
			if (test & RUMBLE || test & RTC)
				return (0x4000u);
			o |= RUMBLE;
			s += 6;
		}
		else
			return (0x4000u);
	}
	return (o);
}

void	set_cartridge_type(arguments_t args[], data_t *data)
{
	if (cart_info.member.cart_type)
	{
		print_warning(data->filename, data->lineno, data->line,
						"duplicate keyword .cart_type");
	}

	if (args[1].value)
		goto __too_many_arguments;
	if (args[0].value == NULL)
		goto __too_few_arguments;
	if (args->type & STRING_TYPE)
	{
		register uint8_t opt;

		to_lower_string((char *)args->value);
		char *arg = (char *)args->value;
		cartridge.cart_type = 0;

		if (strncmp(arg, "rom", 3) == 0)
		{
			arg += 3;
			if (*arg == '-')
				arg++;
			if (*arg == 0)
				goto __error_mbc;
			opt = get_mbc_options(arg + 1);

			if (opt & 0x8000)
				goto __separator_error;
			if (opt & 0x4000)
				goto __invalid_option;

			if (opt == 0)
				cartridge.cart_type = 0u;
			else if (opt == SRAM)
				cartridge.cart_type = 8u;
			else if (opt & (SRAM | BATTERY))
				cartridge.cart_type = 9u;
			else
				goto __error_rom;
		}
		else if (strncmp(arg, "mbc", 3) == 0)
		{
			arg += 3;
			if (*arg == '-')
				arg++;
			if (*arg == 0)
				goto __error_mbc;
			opt = get_mbc_options(arg + 1);

			if (opt & 0x8000)
				goto __separator_error;
			if (opt & 0x4000)
				goto __invalid_option;
			if (opt & 0x2000)
				goto __too_many_options;

			switch (*arg)
			{
				case '1':
				{
					if (opt == 0)
						cartridge.cart_type = 1u;
					else if (opt == (BATTERY | SRAM))
						cartridge.cart_type = 3u;
					else if (opt == SRAM)
						cartridge.cart_type = 2u;
					else
						goto __error_mbc1;
					return;
				}
				case '2':
				{
					if (opt == 0)
						cartridge.cart_type = 5u;
					else if (opt == BATTERY)
						cartridge.cart_type = 6u;
					else
						goto __error_mbc2;
				}
				case '3':
				{
					if (opt == 0)
						cartridge.cart_type = 0x11u;
					else if (opt == (RTC | BATTERY))
						cartridge.cart_type = 0xfu;
					else if (opt == (RTC | BATTERY | SRAM))
						cartridge.cart_type = 0x10u;
					else if (opt == (BATTERY | SRAM))
						cartridge.cart_type = 0x13u;
					else if (opt == SRAM)
						cartridge.cart_type = 0x12u;
					else
						goto __error_mbc3;
				}
				case '5':
				{
					if (opt & RTC)
						goto __error_mbc5;
					opt &= 0x3u;
					if (opt == 0)
						cartridge.cart_type = 0x19u;
					else if (opt == (BATTERY | SRAM))
						cartridge.cart_type = 0x1Au;
					else if (opt == SRAM)
						cartridge.cart_type = 0x1Bu;
					else
						goto __error_mbc5;
				}
				default:
					goto __error_mbc;
			}
		}
		else
			goto __unknown_value;
	}
	else
	{
		register uint32_t value = *(uint32_t*)args->value;

		if ((value > 0x9u && value < 0xfu)
				|| (value > 0x13u && value < 0x19u)
				|| value > 0x1Bu)
		{
			goto __unknown_value;
		}
		cartridge.cart_type = value;
	}
	cart_info.member.cart_type = 1;
	return;


	{
		const char	*error;
		__too_few_arguments:
			error = "argument 1 expected";
			goto __print_error;
		__too_many_arguments:
			error = "only one argument is expected";
			goto __print_error;
		__invalid_option:
			error = "invalid option. available options are battery, sram, rumble, rtc";
			goto __print_error;
		__too_many_options:
			error = "too many options";
			goto __print_error;
		__separator_error:
			error = "separator error";
			goto __print_error;
		__error_rom:
			error = "invalid options for rom";
			goto __print_error;
		__error_mbc:
			error = "invalid syntax for mbc";
			goto __print_error;
		__error_mbc1:
			error = "invalid options for mbc-1";
			goto __print_error;
		__error_mbc2:
			error = "invalid options for mbc-2";
			goto __print_error;
		__error_mbc3:
			error = "invalid options for mbc-3";
			goto __print_error;
		__error_mbc5:
			error = "invalid options for mbc-5";
			goto __print_error;
		__unknown_value:
			error = "forbidden value for argument 1";
		__print_error:
			print_error(data->filename, data->lineno, data->line, error);
	}
}

/*
00h -  32KBytes (no ROM banking)
01h -  64KBytes (4 banks)
02h - 128KBytes (8 banks)
03h - 256KBytes (16 banks)
04h - 512KBytes (32 banks)
05h -   1MByte (64 banks)  - only 63 banks used by MBC1
06h -   2MBytes (128 banks) - only 125 banks used by MBC1
07h -   4MBytes (256 banks)
08h -   8MBytes (512 banks)
52h - 1.1MBytes (72 banks)
53h - 1.2MBytes (80 banks)
54h - 1.5MBytes (96 banks)
*/
void	set_rom_size(arguments_t args[], data_t *data)
{
	char		*unit = (char *)args->value;
	uint32_t	value;

	if (cart_info.member.rom_size)
	{
		print_warning(data->filename, data->lineno, data->line,
						"duplicate keyword .rom_size");
	}

	if (args[1].value)
		goto __too_many_arguments;
	if (args[0].value == NULL)
		goto __too_few_arguments;
	if (args->type & STRING_TYPE)
	{
		if (!(is_numeric_len(unit, NULL)))
			goto __unexpected_char;
		value = atou_inc(&unit);
		while (is_space(*unit)) unit++;
		if (*unit == 0)
			goto __missing_unit;

		to_lower_string(unit);
		if (unit[0] == 'k'
			&&	(
					(unit[1] == 'b' && unit[2] == 0)
					|| !strcmp(unit + 1, "byte")
					|| !strcmp(unit + 1, "bytes")
				)
			)
		{
			switch (value)
			{
				case 32: cartridge.rom_size = 0; break;
				case 64: cartridge.rom_size = 1; break;
				case 128: cartridge.rom_size = 2; break;
				case 256: cartridge.rom_size = 3; break;
				case 512: cartridge.rom_size = 4; break;
				default: goto __invalid_size;
			}
		}
		else if (unit[0] == 'm'
			&&	(
					(unit[1] == 'b' && unit[2] == 0)
					|| !strcmp(unit + 1, "byte")
					|| !strcmp(unit + 1, "bytes")
				)
			)
		{
			switch (value)
			{
				case 1: cartridge.rom_size = 5; break;
				case 2: cartridge.rom_size = 6; break;
				case 4: cartridge.rom_size = 7; break;
				case 8: cartridge.rom_size = 8; break;
				default: goto __invalid_size;
			}
		}
		else if (strcmp(unit, "banks") == 0 || strcmp(unit, "bank") == 0)
		{
			switch (value)
			{
				case 0: cartridge.rom_size = 0; break;
				case 4: cartridge.rom_size = 1; break;
				case 8: cartridge.rom_size = 2; break;
				case 16: cartridge.rom_size = 3; break;
				case 32: cartridge.rom_size = 4; break;
				case 64: cartridge.rom_size = 5; break;
				case 128: cartridge.rom_size = 6; break;
				case 256: cartridge.rom_size = 7; break;
				case 512: cartridge.rom_size = 8; break;
				default: goto __invalid_size;
			}
		}
		else
			goto __unknown_unit;
	}
	else
	{
		register uint32_t	value = *(uint32_t *)args->value;
		if (value > 8)
			goto __invalid_size;
		cartridge.rom_size = value;
	}
	cart_info.member.rom_size = 1;
	return;


	{
		const char	*error;
		__too_few_arguments:
			error = "argument 1 expected";
			goto __print_error;
		__too_many_arguments:
			error = "only one argument is expected";
			goto __print_error;
		__missing_unit:
			error = "missing unit";
			goto __print_error;
		__unknown_unit:
			sprintf(data->buf, "unknown unit `%s`", unit);
			goto __print_error_fmt;
		__invalid_size:
			error = "forbidden value for argument 1";
			goto __print_error;
		__unexpected_char:
			sprintf(data->buf, "unexpected character `%c`", *unit);
		__print_error_fmt:
			error = data->buf;
		__print_error:
			print_error(data->filename, data->lineno, data->line, error);
	}
}

void	set_ram_size(arguments_t args[], data_t *data)
{
	char		*unit = (char *)args->value;
	uint32_t	value;

	if (cart_info.member.ram_size)
	{
		print_warning(data->filename, data->lineno, data->line,
						"duolicate keyword .ram_size");
	}

	if (args[1].value)
		goto __too_many_arguments;
	if (args[0].value == NULL)
		goto __too_few_arguments;
	if (args->type & STRING_TYPE)
	{
		if (strcmp(unit, "none") == 0)
		{
			cartridge.ram_size = 0;
			return ;
		}


		if (!is_numeric_len(unit, NULL))
			goto __unexpected_char;
		value = atou_inc(&unit);
		while (is_space(*unit)) unit++;
		if (*unit == 0)
			goto __missing_unit;

		to_lower_string(unit);
		if (unit[0] == 'k'
			&&	(
					(unit[1] == 'b' && unit[2] == 0)
					|| !strcmp(unit + 1, "byte")
					|| !strcmp(unit + 1, "bytes")
				)
			)
		{
			switch (value)
			{
				case 0: cartridge.ram_size = 0; break;
				case 8: cartridge.ram_size = 2; break;
				case 32: cartridge.ram_size = 3; break;
				case 128: cartridge.ram_size = 4; break;
				default: goto __invalid_size;
			}
		}
	}
	else
	{
		register uint32_t	value = *(uint32_t *)args->value;

		if (value == 1 || value > 4)
			goto __invalid_size;
		cartridge.ram_size = value;
	}
	cart_info.member.ram_size = 1;
	return;


	{
		const char	*error;
		__too_few_arguments:
			error = "argument 1 expected";
			goto __print_error;
		__too_many_arguments:
			error = "only one argument is expected";
			goto __print_error;
		__missing_unit:
			error = "missing unit";
			goto __print_error;
		__invalid_size:
			error = "forbidden value for argument 1";
			goto __print_error;
		__unexpected_char:
			sprintf(data->buf, "unexpected char `%c`", *unit);
			error = data->buf;
		__print_error:
			print_error(data->filename, data->lineno, data->line, error);
	}
}

#define	JAPAN	0x0u
#define	OTHER	0x1u

void	set_code_dest(arguments_t args[], data_t *data)
{
	if (cart_info.member.destination)
	{
		print_warning(data->filename, data->lineno, data->line,
						"duplicate keyword .destination");
	}

	if (args[1].value)
		goto __too_many_arguments;
	if (args[0].value == NULL)
		goto __too_few_arguments;
	if (args->type & STRING_TYPE)
	{
		to_lower_string((char *)args->value);
		if (strcmp((char *)args->value, "japan") == 0)
			cartridge.destination = JAPAN;
		else if (strcmp((char *)args->value, "other") == 0)
			cartridge.destination = OTHER;
		else
			goto __unknown_value;
	}
	else
	{
		register uint32_t value = *(uint32_t*)args->value;

		if (value != JAPAN && value != OTHER)
			goto __unknown_value;
		cartridge.destination = value;
	}
	cart_info.member.destination = 1;
	return;


	{
		const char	*error;
		__too_few_arguments:
			error = "argument 1 expected";
			goto __print_error;
		__too_many_arguments:
			error = "only one argument is expected";
			goto __print_error;
		__unknown_value:
			error = "forbidden value for argument 1";
		__print_error:
			print_error(data->filename, data->lineno, data->line, error);
	}
}

void	set_version(arguments_t args[], data_t *data)
{
	uint32_t	value;

	if (cart_info.member.version)
	{
		print_warning(data->filename, data->lineno, data->line,
						"duplicate keyword .version");
	}

	if (args[1].value)
		goto __too_many_arguments;
	if (args[0].value == NULL)
		goto __too_few_arguments;
	if (args[0].type & STRING_TYPE)
		goto __error_type;

	value = *(uint32_t *)(args->value);
	if (value & 0xffffff00u)
		goto __overflow;
	cartridge.version = value;
	cart_info.member.version = 1;
	return;

	{
		const char	*error;
		__too_few_arguments:
			error = "argument 1 expected";
			goto __print_error;
		__too_many_arguments:
			error = "only one argument is expected";
			goto __print_error;
		__error_type:
			error = "argument 1 should be an integer";
			goto __print_error;
		__overflow:
			sprintf(data->buf, "argument 1 overflow. (value is 0x%x, max is 0xff)", value);
			error = (const char *)data->buf;
		__print_error:
			print_error(data->filename, data->lineno, data->line, error);
	}
}
