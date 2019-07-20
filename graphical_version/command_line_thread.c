/* ************************************************************************** */
/*                                                          LE - /            */
/*                                                              /             */
/*   command_line_thread.c                            .::    .:/ .      .::   */
/*                                                 +:+:+   +:    +:  +:+:+    */
/*   By: fcordon <marvin@le-101.fr>                 +:+   +:    +:    +:+     */
/*                                                 #+#   #+    #+    #+#      */
/*   Created: 2019/07/03 09:33:12 by fcordon      #+#   ##    ##    #+#       */
/*   Updated: 2019/07/20 22:36:39 by fcordon     ###    #+. /#+    ###.fr     */
/*                                                         /                  */
/*                                                        /                   */
/* ************************************************************************** */


#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <signal.h>
#include <termios.h>
#include <stdarg.h>
#include <time.h>
#include "memory_map.h"
#include "pkmn_green_string.h"
#include "cheat.h"
/*
#include "SDL.h"
#include "test.h"
#include "registers.h"
#include "execute.h"
*/

/*
	prochainement :

	addr pokemon 1 proprietaire = 0xd233

*/

extern void		*command_line_thread(void *unused)
{
	char			buf[512] = {0};
	t_hist			*hist = NULL;
	t_hist			*curh = NULL;
	unsigned int	hsize = 0;
	char			buf2[32] = {0};
	char			*p;
	unsigned int	i = 0;
	unsigned int	j = 0;
	unsigned long	chr;
	unsigned int	tmp;
	unsigned char	*ptr;
	unsigned int	pkmn;
	int				err;
	uint32_t		pkmn_addr[43] = {0};

	srand(time(NULL));
	write(2, "\e[?25l", 6);

	if (strncmp(g_cart.game_title, "POKEMON RED", 11) == 0
			|| strncmp(g_cart.game_title, "POKEMON BLU", 11) == 0)
	{
		pkmn = PKMN_RB;
		pkmn_addr[HEAD] = 0xd168U;
		pkmn_addr[NO] = 0xd170U;
		pkmn_addr[CHP] = 0xd171U;
		pkmn_addr[STATUS] = 0xd174U;
		pkmn_addr[ATT1] = 0xd178U;
		pkmn_addr[ATT2] = 0xd179U;
		pkmn_addr[ATT3] = 0xd17aU;
		pkmn_addr[ATT4] = 0xd17bU;
		pkmn_addr[ID] = 0xd17cU;
		pkmn_addr[XP] = 0xd17eU;
		pkmn_addr[PP1] = 0xd18dU;
		pkmn_addr[PP2] = 0xd18eU;
		pkmn_addr[PP3] = 0xd18fU;
		pkmn_addr[PP4] = 0xd190U;
		pkmn_addr[LVL] = 0xd191U;
		pkmn_addr[HP] = 0xd192U;
		pkmn_addr[ATT] = 0xd194U;
		pkmn_addr[DEF] = 0xd196U;
		pkmn_addr[VIT] = 0xd198U;
		pkmn_addr[SPE] = 0xd19aU;
		pkmn_addr[OWNER] = 0xd26dU;
		pkmn_addr[NAME] = 0xd2baU;
		pkmn_addr[OBJ] = 0xd322U;
		pkmn_addr[OBJ_PC] = 0xd53fU;
		pkmn_addr[CASH] = 0xd34dU;
		pkmn_addr[CBT_CHP] = 0xd01aU;
		pkmn_addr[CBT_HP] = 0xd028U;
		pkmn_addr[ADV_HP] = 0xcfebU;
		pkmn_addr[CBT_ATT] = 0xd02aU;
		pkmn_addr[CBT_DEF] = 0xd02cU;
		pkmn_addr[CBT_VIT] = 0xd02eU;
		pkmn_addr[CBT_SPE] = 0xd030U;
		pkmn_addr[CBT_ATT_2] = 0xd00cU;
		pkmn_addr[CBT_DEF_2] = 0xd00eU;
		pkmn_addr[CBT_VIT_2] = 0xd010U;
		pkmn_addr[CBT_SPE_2] = 0xd012U;
		pkmn_addr[ROM_ATT] = 0x38000U;
		pkmn_addr[ROM_ATT_NAME] = 0xb0000U;
		pkmn_addr[ROM_POK_NAME] = 0x1c21eU;
		pkmn_addr[BADGES] = 0xd35bU;
		pkmn_addr[USER_NAME] = 0xd15dU;
		pkmn_addr[RIVAL_NAME] = 0xd34fU;
		pkmn_addr[CSTAT] = 0xd01dU;
	}
	else if (strncmp(g_cart.game_title, "POKEMON GRE", 11) == 0)
	{
		pkmn = PKMN_GRE;
		pkmn_addr[HEAD] = 0xd123U;
		pkmn_addr[NO] = 0xd12bU;
		pkmn_addr[CHP] = 0xd12cU;
		pkmn_addr[STATUS] = 0xd12fU;
		pkmn_addr[ATT1] = 0xd133U;
		pkmn_addr[ATT2] = 0xd134U;
		pkmn_addr[ATT3] = 0xd135U;
		pkmn_addr[ATT4] = 0xd136U;
		pkmn_addr[ID] = 0xd137U;
		pkmn_addr[XP] = 0xd139U;
		pkmn_addr[PP1] = 0xd148U;
		pkmn_addr[PP2] = 0xd149U;
		pkmn_addr[PP3] = 0xd14aU;
		pkmn_addr[PP4] = 0xd14bU;
		pkmn_addr[LVL] = 0xd14cU;
		pkmn_addr[HP] = 0xd14dU;
		pkmn_addr[ATT] = 0xd14fU;
		pkmn_addr[DEF] = 0xd151U;
		pkmn_addr[VIT] = 0xd153U;
		pkmn_addr[SPE] = 0xd155U;
		pkmn_addr[OWNER] = 0xd233U;
		pkmn_addr[NAME] = 0xd257U;
		pkmn_addr[OBJ] = 0xd2a1U;
		pkmn_addr[OBJ_PC] = 0xd4b9U;
		pkmn_addr[CASH] = 0xd2cbU;
		pkmn_addr[CBT_CHP] = 0xcffcU;
		pkmn_addr[ADV_HP] = 0xcfcdU;
		pkmn_addr[CBT_HP] = 0xcfa1U;
		pkmn_addr[CBT_ATT] = 0xcfa3U;
		pkmn_addr[CBT_DEF] = 0xcfa5U;
		pkmn_addr[CBT_VIT] = 0xcfa7U;
		pkmn_addr[CBT_SPE] = 0xcfa9U;
		pkmn_addr[CBT_ATT_2] = 0xd00cU;
		pkmn_addr[CBT_DEF_2] = 0xd00eU;
		pkmn_addr[CBT_VIT_2] = 0xd010U;
		pkmn_addr[CBT_SPE_2] = 0xd012U;
		pkmn_addr[ROM_ATT] = 0x39658U;
		pkmn_addr[ROM_ATT_NAME] = 0x10000U;
		pkmn_addr[ROM_POK_NAME] = 0x39068U;
		pkmn_addr[BADGES] = 0xd2d5U;
	}
	else
		pkmn = 0;
	(void)unused;
	term_noecho_mode(1);
	for (;;)
	{
__read:
		if (*buf == 0)
			write(1, "\e[0;41m \e[0m", 12);
		chr = 0x0UL;
		read(0, &chr, sizeof(unsigned long));

		write(2, "\e[512D\e[2K", 10);
/*
*	backspace, arrows
*/
		if (chr == 127)
		{
			if (i > 0)
			{
				if (i == j) {
					i--;
					buf[i] = 0;
				}
				else
				{
					i--;
					memmove(buf + i, buf + i + 1, (j - i) + 0);
				}
				j--;
			}
			else
				goto __read;
		}
		else if (chr == K_LEFT)
		{
			if (i > 0) {
				i--;
			}
		}
		else if (chr == K_RIGHT)
		{
			if (i < j) {
				i++;
			}
		}
		else if (chr == K_UP || chr == K_DOWN)
		{
			if (chr == K_UP)
			{
				if (curh == NULL)
					curh = hist;
				else if (curh->next)
					curh = curh->next;
			}
			else
			{
				if (curh)
					curh = curh->prev;
			}

			if (curh)
			{
				load_hist(buf, curh);
				i = curh->l;
				j = i;
			}
			else
			{
				buf[i = 0] = '\0';
				j = 0;
			}
			write(1, "\e[512D\e[2K", 10);
			chr = 127UL;
			goto __print;
		}
		else
		{
			if (chr != '\n')
			{
				if (i == j)
				{
					buf[i++] = (uint8_t)chr;
					buf[i] = 0;
					j++;
				}
				else
				{
					memmove(buf + i + 1, buf + i, (j - i) + 1);
					j++;
					buf[i++] = (uint8_t)chr;
				}
			}
		}
__print:
		if (*buf)
		{
			write(1, "\e[0m", 4);
			write(1, buf, i);
			write(1, "\e[0;41m", 7);

			if (buf[i] == 0)
				write(1, " ", 1);
			else
				write(1, buf + i, 1);

			write(1, "\e[0m", 4);
			if (j > i)
				write(1, buf + i + 1, j - i);
		}


		if (chr == 127 || chr == K_LEFT || chr == K_RIGHT || chr == K_UP || chr == K_DOWN)
			goto __read;

		else if (chr == '\n')
		{
			buf[j] = '\0';
			write(1, "\e[512D\e[2K", 10);
			write(1, buf, j);
			write(1, "\n", 1);
			//write(1, "\e[512C\n", 7);

			if (strcmp(buf, "exit") == 0)
			{
				term_noecho_mode(0);
				close_log_file_and_exit(0);
			}
			if (strncmp(buf, "rset", 4) == 0 && non_alnum(buf[4]))
			{
				uint32_t	addr;
				uint32_t	val;

				p = buf + 4;
				
				while (*p == ' ') p++;
				if (*p != '(' && p[-1] != ' ')
				{
					puts("\n\e[0;31msyntax error\n\e[0m");
					goto __forest_end;
				}
				if (*p == '(') {
					p++;
					while (*p == ' ') p++;
				}

				addr = (unsigned short)atoi_hexa(&p, &err);
				if (err) {
					puts("\n\e[0;31msyntax error\n\e[0m");
					goto __forest_end;
				}

				while (*p == ' ') p++;
				if (*p != ',' && p[-1] != ' ') {
					puts("\n\e[0;31msyntax error\n\e[0m");
					goto __forest_end;
				}

			__next_set:
				if (*p == ',') {
					p++;
					while (*p == ' ') p++;
				}
				if (*p == '\0')
					goto __forest_end;

				val = atoi_hexa(&p, &err);
				if (err) {
					puts("\n\e[0;31msyntax error\e[0m");
					goto __forest_end;
				}

				*(GET_REAL_ADDR(addr)) = (uint8_t)val;
				addr++;
				
				while (*p == ' ') p++;
				if (*p == ',' || p[-1] == ' ')
					goto __next_set;
			}
			/*
			else if (strncmp(buf, "set", 3) == 0 && non_alnum(buf[3]))
			{
				uint32_t	addr, val;
				
				if (va_parse_u32(buf + 3, 2, 2, &addr, &val) == NULL)
					goto __forest_end;

				*(GET_REAL_ADDR((uint16_t)addr)) = val;
			}
			*/
			else if (strncmp(buf, "set", 3) == 0 && non_alnum(buf[3]))
			{
				uint32_t	addr, val1, val2;
				
				if (va_parse_u32(buf + 3, 2, 3, &addr, &val2, &val1) == NULL)
					goto __forest_end;
				if (val1 == 0xffffffffU) {
					val1 = val2;
					val2 = 0;
				}
				*(GET_REAL_ADDR((uint16_t)addr) + val2) = val1;
			}
			else if (strncmp(buf, "get", 3) == 0 && non_alnum(buf[3]))
			{
				uint32_t	addr;
				p = buf + 3;

				if (va_parse_u32(buf + 3, 1, 1, &addr) == NULL)
					goto __forest_end;

				printf("--> %hhu (%hhi:0x%hhx)\n",
						*(GET_REAL_ADDR(addr)), *(GET_REAL_ADDR(addr)), *(GET_REAL_ADDR(addr)));
			}
			else if (strncmp(buf, "dump", 4) == 0 && non_alnum(buf[4]))
			{
				uint32_t	addr, addr2;

				if (va_parse_u32(buf + 4, 2, 2, &addr, &addr2) == NULL)
					goto __forest_end;

				for (; addr < addr2; addr++)
				{
					if ((addr & 0xf) == 0) {
						tmp = sprintf(buf2, "\n%p:  ", (void*)((uint64_t)addr));
						write(1, buf2, tmp);
					}
					sprintf(buf2, "%#5hhx ", *(GET_REAL_ADDR((uint16_t)addr)));
					write(1, buf2, strlen(buf2));
				}
				write(1, "\n", 1);
			}
			else if (strncmp(buf, "fdump", 5) == 0 && non_alnum(buf[5]))
			{
				int				fd;
				char			*mem;

				p = buf + 5;
				while (*p == ' ') p++;

				if (*p != '\0')
				{
					fd = open(p, O_WRONLY | O_CREAT | O_TRUNC, 0666);
					if (fd > 0)
					{
						mem = malloc(0x10000);
						write_dump_switchable_mem_fd(fd, (void*)0xa000, g_memmap.save_size, mem, "==> EXTERNAL RAM (cartridge: 0xa000)\n\n");
						write_dump_fd(fd, (void*)0xc000, 0x2000, mem, "\n\n\n==> UNIT WORKING RAM (gameboy: 0xc000)\n\n");
						write_dump_fd(fd, (void*)0xff80, 0x7f, mem, "\n\n\n==> WORKING & STACK RAM (gameboy: 0xff80)\n\n");
						write_dump_fd(fd, (void*)0xfe00, 0xa0, mem, "\n\n\n==> OAM (gameboy: 0xfe00)\n\n");
						write_dump_fd(fd, (void*)0x8000, 0x2000, mem, "\n\n\n==> VRAM (cartridge: 0x8000)\n\n");
						free(mem);
						close(fd);
					}
				}
				else
				{
					puts("\nsyntax error");
				}
			}
			else if (strncmp(buf, "fbdump", 6) == 0 && non_alnum(buf[6]))
			{
				int			fd;
				char		*name;
				char		*zone[5] = {NULL};

				p = buf + 6;
				while (*p == ' ') p++;
				if (*p == '(')
				{
					p++;
					while (*p == ' ') p++;
				}
				if (non_alnum(*p))
				{
					puts("(non1) syntax error");
					goto __forest_end;
				}

				name = p;
				while (*p && *p != ' ' && *p != ',') p++;
				if (*p == '\0' || p[1] == '\0') {
					puts("(nulbyte) syntax error");
					goto __forest_end;
				}
				*(p++) = '\0';

				for (unsigned int i = 0; i < 5; i++)
				{
					zone[i] = p;
					while (*p && *p != ' ' && *p != ',') p++;
					if (*p == '\0')
					{
						break;
					}
					*(p++) = '\0';
					while (*p == ' ') p++;
					if (*p == ',') {
						p++;
						while (*p == ' ') p++;
					}
				}

				if ((fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1)
					goto __forest_end;

				for (unsigned int i = 0; i < 5 && zone[i]; i++)
				{
					if (!strcmp("ERAM", zone[i]) || !strcmp("eram", zone[i]) || !strcmp("e", zone[i]))
						write(fd, GET_REAL_ADDR(0xa000), g_memmap.save_size);
					else if (!strcmp("URAM", zone[i]) || !strcmp("uram", zone[i]) || !strcmp("u", zone[i]))
						write(fd, GET_REAL_ADDR(0xc000), 0x2000);
					else if (!strcmp("WRAM", zone[i]) || !strcmp("wram", zone[i]) || !strcmp("w", zone[i]))
						write(fd, GET_REAL_ADDR(0xff80), 0x7f);
					else if (!strcmp("OAM", zone[i]) || !strcmp("oam", zone[i]) || !strcmp("o", zone[i]))
						write(fd, GET_REAL_ADDR(0xfe00), 0xa0);
					else if (!strcmp("VRAM", zone[i]) || !strcmp("vram", zone[i]) || !strcmp("v", zone[i]))
						write(fd, GET_REAL_ADDR(0x8000), 0x2000);
					else {
						puts("unknown argument");
						close(fd);
						goto __forest_end;
					}
				}
				close(fd);
			}
			else if (strncmp(buf, "bzero", 5) == 0 && non_alnum(buf[5]))
			{
				uint32_t	start, length, offset;
				uint8_t		*_addr;

				if (va_parse_u32(buf + 5, 2, 3, &start, &length, &offset) == NULL)
					goto __forest_end;
				_addr = GET_REAL_ADDR(start);
				if (offset != 0xffffffffU)
					_addr += offset;
				g_stop_execution = 1;
				while (g_stop_execution != 2) usleep(1000);
				bzero(_addr, length);
				g_stop_execution = 0;
			}
			else if (strncmp(buf, "logx", 4) == 0 && non_alnum(buf[4]))
			{
				uint32_t	n_inst;

				if (va_parse_u32(buf + 4, 1, 1, &n_inst) == NULL)
					goto __forest_end;
				if (_LOG_ENABLE == false) {
					_N_INST_LOG = (n_inst > 0) ? n_inst : 1;
					printf("start log %lu cycles\n", _N_INST_LOG);
					_LOG_ENABLE = true;
					_CPU_LOG = true;
				}
				else
					puts("log currently writed...");
			}
			else if (strncmp(buf, "help", 4) == 0 && non_alnum(buf[4]))
			{
				p = buf + 4;
				if (*p == 0)
				{
					#define HELP	"memory functions:\n"\
								"    \e[0;33mset   \e[0m(\e[1;34m2b\e[0m addr, \e[1;34m1b\e[0m value)\n"\
								"    \e[0;33mget   \e[0m(\e[1;34m2b\e[0m addr)\n"\
								"    \e[0;33mrset  \e[0m(\e[1;34m2b\e[0m addr, \e[1;34m1b\e[0m value, ...)\n"\
								"    \e[0;33mdump  \e[0m(\e[1;34m2b\e[0m start, \e[1;34m2b\e[0m end)\n"\
								"    \e[0;33mfdump \e[0m(\e[1;34mstring\e[0m filename)\n\n"\
								"pokemon green/red/blue functions:\n"\
								"    \e[0;33msoin     \e[0m(n\e[1;31m?\e[0m);\n"\
								"    \e[0;33madv_pv   \e[0m(\e[1;34m2b\e[0m pv);\n"\
								"    \e[0;33mfor++    \e[0m(\e[1;34m2b\e[0m add_value\e[1;31m?\e[0m);\n"\
								"    \e[0;33mdef++    \e[0m(\e[1;34m2b\e[0m add_value\e[1;31m?\e[0m);\n"\
								"    \e[0;33mvit++    \e[0m(\e[1;34m2b\e[0m add_value\e[1;31m?\e[0m);\n"\
								"    \e[0;33mspe++    \e[0m(\e[1;34m2b\e[0m add_value\e[1;31m?\e[0m);\n"\
								"    \e[0;33mpkmn_num \e[0m(n, \e[1;34m1b\e[0m numero);\n"\
								"    \e[0;33mpkmn_push\e[0m(n, \e[1;34m1b\e[0m numero);\n"\
								"    \e[0;33mpv       \e[0m(n, \e[1;34m2b\e[0m max, \e[1;34m2b\e[0m cur\e[1;31m?\e[0m);\n"\
								"    \e[0;33matt1     \e[0m(n, \e[1;34m1b\e[0m numero, \e[1;34m1b\e[0m pp\e[1;31m?\e[0m);\n"\
								"    \e[0;33matt2     \e[0m(n, \e[1;34m1b\e[0m numero, \e[1;34m1b\e[0m pp\e[1;31m?\e[0m);\n"\
								"    \e[0;33matt3     \e[0m(n, \e[1;34m1b\e[0m numero, \e[1;34m1b\e[0m pp\e[1;31m?\e[0m);\n"\
								"    \e[0;33matt4     \e[0m(n, \e[1;34m1b\e[0m numero, \e[1;34m1b\e[0m pp\e[1;31m?\e[0m);\n"\
								"    \e[0;33mid       \e[0m(n, \e[1;34m2b\e[0m id);\n"\
								"    \e[0;33mxp       \e[0m(n, \e[1;34m3b\e[0m xp);\n"\
								"    \e[0;33mniv      \e[0m(n, \e[1;34m1b\e[0m niveau);\n"\
								"    \e[0;33mfor      \e[0m(n, \e[1;34m2b\e[0m for);\n"\
								"    \e[0;33mdef      \e[0m(n, \e[1;34m2b\e[0m def);\n"\
								"    \e[0;33mvit      \e[0m(n, \e[1;34m2b\e[0m vit);\n"\
								"    \e[0;33mspe      \e[0m(n, \e[1;34m2b\e[0m spe);\n"\
								"    \e[0;33mnom      \e[0m(n, \e[1;34mstring\e[0m name);\n"\
								"    \e[0;33maj       \e[0m(\e[0;34mvoid\e[0m);\n"\
								"    \e[0;33msup      \e[0m(n);\n"\
								"    \e[0;33mobj_push \e[0m(\e[1;34m1b\e[0m object, \e[1;34m1b\e[0m quantity);\n"\
								"    \e[0;33mobj_pop  \e[0m(\e[0;34mvoid\e[0m);\n"\
								"    \e[0;33mstat     \e[0m(\e[1;34m2b\e[0m for, \e[1;34m2b\e[0m def, "\
									"\e[1;34m2b       \e[0m vit, \e[1;34m2b\e[0m spe);\n"\
								"    \e[0;33margent   \e[0m(\e[1;34m2b\e[0m cash); [red/blue]\n"\
								"    \e[0;33margent   \e[0m(\e[1;34m3b\e[0m cash); [green]\n"\
								"  n = [1-6]\n"\
								"  1b = 1 byte, 2b = 2 bytes, 3b = 3 bytes\n\n"\
								"syntax:\n"\
								"    mnemonic(a,b,...)\n"\
								"    mnemonic a b ...\n"\
								"    mnemonic a, b, ...\n"\
								"    mnemonic(a b ...)\n\n"\

					write(1, HELP, strlen(HELP));
					goto __forest_end;
				}
				while (*p == ' ') p++;
				if (pkmn && strcmp(p, "soin") == 0)
					puts("soin(void) : soigne le pokemon courant (en combat).\n"
						"soin(0)    : soigne tous les pokemons et restaure leurs pp (hors combat)\n"
						"soin(n)    : soigne le pokemon n et restaure ses pp (hors combat)\n");
				else if (pkmn && strcmp(p, "adv_pv") == 0)
					puts("adv_pv : choisir le nombre de pv du pokemon ennemi (en combat)\n");
				else if (pkmn && strcmp(p, "for++") == 0)
					puts("for++(void) : augmente la force du pokemon courant de 8 (en combat)\n"
						"for++(x)    : augmente la force du pokemon courant de x (en combat)\n");
				else if (pkmn && strcmp(p, "def++") == 0)
					puts("def++(void) : augmente la defense du pokemon courant de 8 (en combat)\n"
						"def++(x)    : augmente la defense du pokemon courant de x (en combat)\n");
				else if (pkmn && strcmp(p, "vit++") == 0)
					puts("vit++(void) : augmente la vitesse du pokemon courant de 8 (en combat)\n"
						"vit++(x)    : augmente la vitesse du pokemon courant de x (en combat)\n");
				else if (pkmn && strcmp(p, "spe++") == 0)
					puts("spe++(void) : augmente le special du pokemon courant de 8 (en combat)\n"
						"spe++(x)    : augmente le special du pokemon courant de x (en combat)\n");
				else if (pkmn && strcmp(p, "pkmn_num") == 0)
					puts("pkmn_num(n, x) : modifier le numero du pokemon x (hors combat)\n");
				else if (pkmn && strcmp(p, "pkmn_push") == 0)
					puts("pkmn_push(n)    : ajouter un pokemon choisi aleatoirement a la suite\n"
						 "pkmn_push(n, x) : ajouter le pokemon numero x a la suite\n");
				else if (pkmn && strcmp(p, "pv") == 0)
					puts("pv(n, x)    : change le nombre de points de vie max et courant (x) du pokemon n\n"
						 "pv(n, x, y) : change le nombre de points de vie max (x) et courant (y) du pokemon n\n");
				else if (pkmn && strcmp(p, "att1") == 0)
					puts("att1(n)    : ajoute une attaque aleatoire a l'emplacement 1\n"
						 "att1(n, x) : ajoute l'attaque numero x a l'emplacement 1\n");
				else if (pkmn && strcmp(p, "att2") == 0)
					puts("att2(n)    : ajoute une attaque aleatoire a l'emplacement 2\n"
						 "att2(n, x) : ajoute l'attaque numero x a l'emplacement 2\n");
				else if (pkmn && strcmp(p, "att3") == 0)
					puts("att3(n)    : ajoute une attaque aleatoire a l'emplacement 3\n"
						 "att3(n, x) : ajoute l'attaque numero x a l'emplacement 3\n");
				else if (pkmn && strcmp(p, "att4") == 0)
					puts("att4(n)    : ajoute une attaque aleatoire a l'emplacement 4\n"
						 "att4(n, x) : ajoute l'attaque numero x a l'emplacement 4\n");
				else if (pkmn && strcmp(p, "id") == 0)
					puts("id(n, x) : modifie l'id (x) du pokemon n\n");
				else if (pkmn && strcmp(p, "xp") == 0)
					puts("xp(n, x) : modifie l'xp (x) du pokemon n\n");
				else if (pkmn && strcmp(p, "niv") == 0)
					puts("niv(n, x) : reinitialise le niveau du pokemon n avec la valeur x\n");
				else if (pkmn && strcmp(p, "for") == 0)
					puts("for(n, x) : reinitialise la force du pokemon n avec la valeur x\n");
				else if (pkmn && strcmp(p, "def") == 0)
					puts("def(n, x) : reinitialise la defense du pokemon n avec la valeur x\n");
				else if (pkmn && strcmp(p, "vit") == 0)
					puts("vit(n, x) : reinitialise la vitesse du pokemon n avec la valeur x\n");
				else if (pkmn && strcmp(p, "spe") == 0)
					puts("spe(n, x) : reinitialise le special du pokemon n avec la valeur x\n");
				else if (pkmn && strcmp(p, "nom") == 0)
					puts("nom(n, x) : change le nom du pokemon n avec x\n");
				else if (pkmn && strcmp(p, "maj") == 0)
					puts("maj(void) : met a jour le header des pokemons\n");
				else if (pkmn && strcmp(p, "pkmn_sup") == 0)
					puts("pkmn_sup(n) : supprime le pokemon n (decale les suivants si necessaire)\n");
				else if (pkmn && strcmp(p, "obj_push") == 0)
					puts("obj_push(n, x)    : ajoute l'objet x a l'inventaire (quantite par defaut : 99)\n"
						 "obj_push(n, x, y) : ajoute l'objet x a l'inventaire en y exemplaires\n");
				else if (pkmn && strcmp(p, "obj_pop") == 0)
					puts("obj_pop(void) : supprime le dernier objet de l'inventaire\n");
				else if (pkmn && strcmp(p, "stat") == 0)
					puts("stat(n, for, def?, vit?, spe?) : modifie les caracteristiques du pokemon n\n");
				else if (pkmn && strcmp(p, "argent") == 0)
					puts("argent(x) : modifie l'argent que possede le joueur\n");
				else
					puts("commande inconnue\n");
				//...
			}
/*
 *	POKEMON ROUGE, BLEU, VERT
 */
			else if (pkmn)
			{
				p = buf;

				// auto update header ?
				if (strncmp(p, "pkmn_num", 8) == 0 && non_alnum(p[8]))
				{
					uint32_t	val;
					uint32_t	offset;
					uint32_t	count;
					uint8_t		*addr;

					if (va_parse_u32(p + 8, 2, 2, &offset, &val) == NULL)
						goto __forest_end;

					offset -= 1;
					*GET_REAL_ADDR(pkmn_addr[NO] + (offset * 44)) = (uint8_t)val;
					addr = GET_REAL_ADDR(pkmn_addr[HEAD]) + 1;
					addr[offset] = val;
					for (count = 0; (addr[count] != 0xffU && addr[count] != 0U) || count == 6; count++);
					addr[count] = 0xffU;
					addr[-1] = (uint8_t)count;
				}
				else if (strncmp(p, "pkmn_push", 9) == 0 && non_alnum(p[9]))
				{
					uint32_t		val;
					uint32_t		a, b, c;
					uint32_t		count;
					uint8_t			*addr, *dest;
					uint8_t			*pname;
					const uint32_t	max = (pkmn == PKMN_GRE) ? 5 : 10;
					const uint8_t	*default_stats = (uint8_t*)"\x0\x17\x0\x0\x14\x14\x2a\x21\x2b\x0\x0\x12\x34"
													"\x0\x0\xef\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x23"
													"\x28\x0\x0\x5\x0\x17\x0\xd\x0\xd\x0\xd\x0\xd";

					if (va_parse_u32(p + 9, 0, 1, &val) == NULL)
						goto __forest_end;

					if (val == 0xffffffffU)
					{
						do
						{
							val = (rand() % 0xbe) + 1;
						}
						while (is_missingno(val));
					}

					// pkmn Nº & header
					addr = GET_REAL_ADDR(pkmn_addr[HEAD]);
					count = addr[0];
					if (count == 6) {
						puts("equipe complete, bientot : ajout dans l'ordinateur.");
						goto __forest_end;
					}
					addr++;
					addr[count] = val;
					addr[count + 1] = 0xffU;
					addr[-1] = count + 1;
					addr = GET_REAL_ADDR(pkmn_addr[NO] + (count * 44));
					*addr = val;

					// default stats
					memcpy(addr + 1, default_stats, 43);

					// set name
					addr = GET_REAL_ADDR(pkmn_addr[NAME] + (count * (max + 1)));
					pname = g_memmap.fixed_rom + pkmn_addr[ROM_POK_NAME];
					for (a = b = 0; a < (val - 1); a++)
					{
						for (c = 0; c < max; c++, b++)
						{
							if (pname[b] == 0x50)
							{
								b++;
								while (pname[b] == 0x50) b++;
								break;
							}
						}
					}

					for (c = 0; pname[b] != 0x50 && c < max; c++, b++)
						addr[c] = pname[b];

					if (pkmn == PKMN_GRE)
						addr[c] = 0x50;
					else
					{
						for (; c < max; c++)
							addr[c] = 0x50;
					}

					// set owner
					addr = GET_REAL_ADDR(pkmn_addr[USER_NAME]);
					dest = GET_REAL_ADDR(pkmn_addr[OWNER] + (count * 11));
					for (; (*dest = *addr) != 0x50; addr++, dest++);
				}
				else if (strncmp(p, "soin", 4) == 0 && non_alnum(p[4]))
				{
					uint8_t			*pv_addr;
					uint32_t		numero;

					if (va_parse_u32(p + 4, 0, 1, &numero) == NULL)
						goto __forest_end;

					if (numero == 0xffffffffU) // soin du pokemon en combat
					{
						pv_addr = GET_REAL_ADDR(pkmn_addr[CBT_HP]);
						*GET_REAL_ADDR(pkmn_addr[CBT_CHP]) = *pv_addr;
						*GET_REAL_ADDR(pkmn_addr[CBT_CHP] + 1) = pv_addr[1];
					}
					else if (numero < 7)
					{
						uint8_t		pp_plus;
						uint8_t		max_pp;
						uint32_t	offset;
						uint32_t	att_data_offset;
						uint32_t	i;
						uint32_t	exit;

						if (numero == 0) {
							exit = 0;
							offset = 0;
						}
						else {
							exit = 1;
							offset = (numero - 1) * 44;
						}

						for (; offset < (uint32_t)(6 * 44); offset += 44U)
						{
							pv_addr = GET_REAL_ADDR(pkmn_addr[HP] + offset);
							*GET_REAL_ADDR(pkmn_addr[CHP] + offset) = pv_addr[0];
							*GET_REAL_ADDR(pkmn_addr[CHP] + 1 + offset) = pv_addr[1];
							printf("pv = %u\n", (pv_addr[0] << 8) | pv_addr[1]);
							for (i = 0; i < 4; i++)
							{
								if ((att_data_offset = *GET_REAL_ADDR(pkmn_addr[ATT1] + i + offset)) == 0)
									continue;
								att_data_offset = ((att_data_offset - 1) * 6);
								max_pp = *((uint8_t*)(g_memmap.fixed_rom
														+ pkmn_addr[ROM_ATT]
														+ att_data_offset + 5
													));
								pp_plus = *GET_REAL_ADDR(pkmn_addr[PP1] + i + offset) & 0xc0;
								*GET_REAL_ADDR(pkmn_addr[PP1] + i + offset) = (max_pp | pp_plus);
							}
							if (exit) break;
						}
					}
					else
						puts("syntax error");
				}
				else if (strncmp(p, "etat", 4) == 0 && non_alnum(p[4]))
				{
					unsigned int	offset;

					if ((p = va_parse_u32(p + 4, 1, 1, &offset)) == NULL)
						goto __forest_end;

					offset = (offset - 1) * 44;
					if (strncmp("psn", p, 3) == 0 || strncmp("PSN", p, 3) == 0)
						*GET_REAL_ADDR(pkmn_addr[STATUS] + offset) = 0x8;
					else if (strncmp("par", p, 3) == 0 || strncmp("PAR", p, 3) == 0)
						*GET_REAL_ADDR(pkmn_addr[STATUS] + offset) = 0x40;
					else if (strncmp("gel", p, 3) == 0 || strncmp("GEL", p, 3) == 0)
						*GET_REAL_ADDR(pkmn_addr[STATUS] + offset) = 0x20;
					else if (strncmp("bru", p, 3) == 0 || strncmp("BRU", p, 3) == 0)
						*GET_REAL_ADDR(pkmn_addr[STATUS] + offset) = 0x10;
					else if (strncmp("som", p, 3) == 0 || strncmp("SOM", p, 3) == 0)
						*GET_REAL_ADDR(pkmn_addr[STATUS] + offset) = 0x4;
					else if (strncmp("ok", p, 2) == 0 || strncmp("OK", p, 2) == 0)
						*GET_REAL_ADDR(pkmn_addr[STATUS] + offset) = 0x0;
					else
						puts("unknown argument");
				}
				else if (strncmp(p, "badge", 5) == 0 && non_alnum(p[5]))
				{
					uint8_t	badges = 0;

					p += 5;
					while (*p == ' ') p++;
					if (*p == '(') {
						p++;
						while (*p == ' ') p++;
					}

					while (*p)
					{
						if (*p > '8' || *p < '0')
						{
							puts("invalid argument");
							goto __forest_end;
						}

						if (*p != '0')
							badges |= (1 << (*p - '1'));
						p++;
						while(*p == ' ') p++;
					}
					*GET_REAL_ADDR(pkmn_addr[BADGES]) = badges;
				}
				else if (strncmp(p, "adv_pv", 6) == 0 && non_alnum(p[6]))
				{
					unsigned int	pv;

					if (va_parse_u32(p + 6, 1, 1, &pv) == NULL)
						goto __forest_end;

					*GET_REAL_ADDR(pkmn_addr[ADV_HP]) = (uint8_t)((pv & 0xff00) >> 8);
					*GET_REAL_ADDR(pkmn_addr[ADV_HP] + 1) = (uint8_t)(pv & 0xff);
				}
				else if (strncmp(p, "for++", 5) == 0 && non_alnum(p[5]))
				{
					uint8_t		*for_addr;
					uint32_t	for_value;
					uint32_t	add_value;

					if (va_parse_u32(p + 5, 0, 1, &add_value) == NULL)
						goto __forest_end;
					if (add_value == 0xffffffffU)
						add_value = 8;
					
					for_addr = GET_REAL_ADDR(pkmn_addr[CBT_ATT]);
					for_value = (*for_addr << 8) | for_addr[1];
					if (for_value + add_value > 999U)
						for_value = 999;
					else
						for_value += add_value;
					*for_addr = (uint8_t)((for_value & 0xff00) >> 8);
					for_addr[1] = (uint8_t)(for_value & 0xff);
					/*
					for_addr = GET_REAL_ADDR(pkmn_addr[CBT_ATT]);
					*for_addr = (uint8_t)((for_value & 0xff00) >> 8);
					for_addr[1] = (uint8_t)(for_value & 0xff);
					*/
				}
				else if (strncmp(p, "def++", 5) == 0 && non_alnum(p[5]))
				{
					uint8_t		*def_addr;
					uint32_t	def_value;
					uint32_t	add_value;

					if (va_parse_u32(p + 5, 0, 1, &add_value) == NULL)
						goto __forest_end;
					if (add_value == 0xffffffffU)
						add_value = 8;
					
					def_addr = GET_REAL_ADDR(pkmn_addr[CBT_DEF]);
					def_value = (*def_addr << 8) | def_addr[1];
					if (def_value + add_value > 999U)
						def_value = 999;
					else
						def_value += add_value;
					*def_addr = (uint8_t)((def_value & 0xff00) >> 8);
					def_addr[1] = (uint8_t)(def_value & 0xff);
					/*
					def_addr = GET_REAL_ADDR(pkmn_addr[CBT_DEF]);
					*def_addr = (uint8_t)((def_value & 0xff00) >> 8);
					def_addr[1] = (uint8_t)(def_value & 0xff);
					*/
				}
				else if (strncmp(p, "vit++", 5) == 0 && non_alnum(p[5]))
				{
					uint8_t		*vit_addr;
					uint32_t	vit_value;
					uint32_t	add_value;

					if (va_parse_u32(p + 5, 0, 1, &add_value) == NULL)
						goto __forest_end;
					if (add_value == 0xffffffffU)
						add_value = 8;
					
					vit_addr = GET_REAL_ADDR(pkmn_addr[CBT_VIT]);
					vit_value = (*vit_addr << 8) | vit_addr[1];
					if (vit_value + add_value > 999U)
						vit_value = 999;
					else
						vit_value += add_value;
					*vit_addr = (uint8_t)((vit_value & 0xff00) >> 8);
					vit_addr[1] = (uint8_t)(vit_value & 0xff);
					/*
					vit_addr = GET_REAL_ADDR(pkmn_addr[CBT_VIT]);
					*vit_addr = (uint8_t)((vit_value & 0xff00) >> 8);
					vit_addr[1] = (uint8_t)(vit_value & 0xff);
					*/
				}
				else if (strncmp(p, "spe++", 5) == 0 && non_alnum(p[5]))
				{
					uint8_t		*spe_addr;
					uint32_t	spe_value;
					uint32_t	add_value;

					if (va_parse_u32(p + 5, 0, 1, &add_value) == NULL)
						goto __forest_end;
					if (add_value == 0xffffffffU)
						add_value = 8;
					
					spe_addr = GET_REAL_ADDR(pkmn_addr[CBT_SPE]);
					spe_value = (*spe_addr << 8) | spe_addr[1];
					if (spe_value + add_value > 999U)
						spe_value = 999;
					else
						spe_value += add_value;
					*spe_addr = (uint8_t)((spe_value & 0xff00) >> 8);
					spe_addr[1] = (uint8_t)(spe_value & 0xff);
					/*
					spe_addr = GET_REAL_ADDR(pkmn_addr[CBT_SPE]);
					*spe_addr = (uint8_t)((spe_value & 0xff00) >> 8);
					spe_addr[1] = (uint8_t)(spe_value & 0xff);
					*/
				}
				else if (strncmp(p, "pv", 2) == 0 && non_alnum(p[2]))
				{
					uint32_t	val1;
					uint32_t	val2;
					uint32_t	offset;

					if (va_parse_u32(p + 2, 2, 3, &offset, &val1, &val2) == NULL)
						goto __forest_end;

					offset = (offset - 1) * 44;

					*GET_REAL_ADDR(pkmn_addr[HP] + offset) = (uint8_t)((val1 & 0xff00) >> 8);
					*GET_REAL_ADDR(pkmn_addr[HP] + 1 + offset) = (uint8_t)(val1 & 0xff);
					if (val2 == 0xffffffffU)
						val2 = val1;
					*GET_REAL_ADDR(pkmn_addr[CHP] + offset) = (uint8_t)((val2 & 0xff00) >> 8);
					*GET_REAL_ADDR(pkmn_addr[CHP] + 1 + offset) = (uint8_t)(val2 & 0xff);
				}
				else if (strncmp(p, "att1", 4) == 0 && non_alnum(p[4]))
				{
					uint32_t	val1;
					uint32_t	val2;
					uint32_t	offset;

/*
	if val1 == 0xffffffffU
		random attack
*/
					if (va_parse_u32(p + 4, 2, 3, &offset, &val1, &val2) == NULL)
						goto __forest_end;

					offset = (offset - 1) * 44;
					*GET_REAL_ADDR(pkmn_addr[ATT1] + offset) = (uint8_t)(val1 & 0xff);
					if (val2 == 0xffffffffU)
						val2 = *(g_memmap.fixed_rom + pkmn_addr[ROM_ATT] + 5 + (6 * (val1 - 1)));
					*GET_REAL_ADDR(pkmn_addr[PP1]  + offset) = (uint8_t)(val2 & 0xff);
				}
				else if (strncmp(p, "att2", 4) == 0 && non_alnum(p[4]))
				{
					unsigned int	val1;
					unsigned int	val2;
					unsigned int	offset;

					if (va_parse_u32(p + 4, 2, 3, &offset, &val1, &val2) == NULL)
						goto __forest_end;

					offset = (offset - 1) * 44;
					*GET_REAL_ADDR(pkmn_addr[ATT2] + offset) = (uint8_t)(val1);
					if (val2 == 0xffffffffU)
						val2 = *(g_memmap.fixed_rom + pkmn_addr[ROM_ATT] + 5 + (6 * (val1 - 1)));
					*GET_REAL_ADDR(pkmn_addr[PP2]  + offset) = (uint8_t)(val2);
				}
				else if (strncmp(p, "att3", 4) == 0 && non_alnum(p[4]))
				{
					unsigned int	val1;
					unsigned int	val2;
					unsigned int	offset;

					if (va_parse_u32(p + 4, 2, 3, &offset, &val1, &val2) == NULL)
						goto __forest_end;

					offset = (offset - 1) * 44;
					*GET_REAL_ADDR(pkmn_addr[ATT3] + offset) = (uint8_t)(val1);
					if (val2 == 0xffffffffU)
						val2 = *(g_memmap.fixed_rom + pkmn_addr[ROM_ATT] + 5 + (6 * (val1 - 1)));
					*GET_REAL_ADDR(pkmn_addr[PP3]  + offset) = (uint8_t)(val2);
				}
				else if (strncmp(p, "att4", 4) == 0 && non_alnum(p[4]))
				{
					unsigned int	val1;
					unsigned int	val2;
					unsigned int	offset;

					if (va_parse_u32(p + 4, 2, 3, &offset, &val1, &val2) == NULL)
						goto __forest_end;

					offset = (offset - 1) * 44;
					*GET_REAL_ADDR(pkmn_addr[ATT4] + offset) = (uint8_t)(val1);
					if (val2 == 0xffffffffU)
						val2 = *(g_memmap.fixed_rom + pkmn_addr[ROM_ATT] + 5 + (6 * (val1 - 1)));
					*GET_REAL_ADDR(pkmn_addr[PP4] + offset) = (uint8_t)(val2);
				}
				else if (strncmp(p, "id", 2) == 0 && non_alnum(p[2]))
				{
					unsigned int	offset;
					unsigned int	val;

					if (va_parse_u32(p + 2, 2, 2, &offset, &val) == NULL)
						goto __forest_end;

					offset = (offset - 1) * 44;
					*GET_REAL_ADDR(pkmn_addr[ID] + offset) = (uint8_t)((val & 0xff00) >> 8);
					*GET_REAL_ADDR(pkmn_addr[ID] + 1 + offset) = (uint8_t)(val & 0xff);
				}
				else if (strncmp(p, "xp", 2) == 0 && non_alnum(p[2]))
				{
					unsigned int	offset;
					unsigned int	val;

					// POKEMON ROUGE : ca marche pas, addresse ?

					if (va_parse_u32(p + 2, 2, 2, &offset, &val) == NULL)
						goto __forest_end;

					offset = (offset - 1) * 44;
					*GET_REAL_ADDR(pkmn_addr[XP] + offset) = (uint8_t)((val & 0xff0000) >> 16);
					*GET_REAL_ADDR(pkmn_addr[XP] + 1 + offset) = (uint8_t)((val & 0xff00) >> 8);
					*GET_REAL_ADDR(pkmn_addr[XP] + 2 + offset) = (uint8_t)(val & 0xff);
				}
				else if (strncmp(p, "niv", 3) == 0 && non_alnum(p[3]))
				{
					unsigned int	offset;
					unsigned int	val;

					if (va_parse_u32(p + 3, 2, 2, &offset, &val) == NULL)
						goto __forest_end;

					offset = (offset - 1) * 44;
					*GET_REAL_ADDR(pkmn_addr[LVL] + offset) = (uint8_t)(val);
				}
				else if (strncmp(p, "for", 3) == 0)
				{
					unsigned int	offset;
					unsigned int	val;

					if (va_parse_u32(p + 3, 2, 2, &offset, &val) == NULL)
						goto __forest_end;

					offset = (offset - 1) * 44;
					*GET_REAL_ADDR(pkmn_addr[ATT] + offset) = ((val & 0xff00u) >> 8);
					*GET_REAL_ADDR(pkmn_addr[ATT] + 1 + offset) = (val & 0xff);
				}
				else if (strncmp(p, "def", 3) == 0 && non_alnum(p[3]))
				{
					unsigned int	offset;
					unsigned int	val;

					if (va_parse_u32(p + 3, 2, 2, &offset, &val) == NULL)
						goto __forest_end;

					offset = (offset - 1) * 44;
					*GET_REAL_ADDR(pkmn_addr[DEF] + offset) = (uint8_t)((val & 0xff00) >> 8);
					*GET_REAL_ADDR(pkmn_addr[DEF] + 1 + offset) = (uint8_t)(val & 0xff);
				}
				else if (strncmp(p, "vit", 3) == 0 && non_alnum(p[3]))
				{
					unsigned int	offset;
					unsigned int	val;

					if (va_parse_u32(p + 3, 2, 2, &offset, &val) == NULL)
						goto __forest_end;

					offset = (offset - 1) * 44;
					*GET_REAL_ADDR(pkmn_addr[VIT] + offset) = (uint8_t)((val & 0xff00) >> 8);
					*GET_REAL_ADDR(pkmn_addr[VIT] + 1 + offset) = (uint8_t)(val & 0xff);
				}
				else if (strncmp(p, "spe", 3) == 0 && non_alnum(p[3]))
				{
					unsigned int	offset;
					unsigned int	val;

					if (va_parse_u32(p + 3, 2, 2, &offset, &val) == NULL)
						goto __forest_end;

					offset = (offset - 1) * 44;
					*GET_REAL_ADDR(pkmn_addr[SPE] + offset) = (uint8_t)((val & 0xff00) >> 8);
					*GET_REAL_ADDR(pkmn_addr[SPE] + 1 + offset) = (uint8_t)(val & 0xff);
				}
				else if (strncmp(p, "do", 2) == 0 && non_alnum(p[2]))
				{
					uint32_t	n;

					if ((p = va_parse_u32(p + 2, 1, 1, &n)) == NULL)
						goto __forest_end;

					while (*p == ' ') p++;
					if (*p == '(') {
						p++;
						while (*p == ' ') p++;
					}
					
					if (pkmn == PKMN_GRE)
						set_string_green(p, 5, GET_REAL_ADDR(pkmn_addr[OWNER] + (n * 6)));
					else
						set_string_red(p, 10, 1, GET_REAL_ADDR(pkmn_addr[OWNER] + (n * 11)));
				}
				else if (strncmp(p, "m_nom", 5) == 0 && non_alnum(p[5]))
				{
					p += 5;
					while (*p == ' ') p++;
					if (*p == '(') {
						p++;
						while (*p == ' ') p++;
					}

					if (pkmn == PKMN_GRE) {
						printf("s = \"%s\"\n", p);
						set_string_green(p, 5, GET_REAL_ADDR(0xc598));
						set_string_green(p, 5, GET_REAL_ADDR(0xcc54));
						set_string_green(p, 5, GET_REAL_ADDR(0xcc5a));
					}
					else
						set_string_red(p, 10, 0, GET_REAL_ADDR(pkmn_addr[USER_NAME]));
				}
				else if (strncmp(p, "r_nom", 5) == 0 && non_alnum(p[5]))
				{
					p += 5;
					while (*p == ' ') p++;
					if (*p == '(') {
						p++;
						while (*p == ' ') p++;
					}

					if (pkmn == PKMN_GRE)
						set_string_green(p, 5, GET_REAL_ADDR(pkmn_addr[RIVAL_NAME]));
					else
						set_string_red(p, 10, 0, GET_REAL_ADDR(pkmn_addr[RIVAL_NAME]));
				}
				else if (strncmp(p, "string", 6) == 0 && non_alnum(p[6]))
				{
					uint32_t	addr;
					uint8_t		*mem;

					if ((p = va_parse_u32(p + 6, 1, 1, &addr)) == NULL)
						goto __forest_end;

					while (*p == ' ') p++;
					if (*p == ',') {
						p++;
						while (*p == ' ') p++;
					}

					mem = GET_REAL_ADDR((unsigned short)(addr & 0xffffU));
					if (pkmn == PKMN_GRE)
						set_string_green(p, 39, mem);
					else
						set_string_red(p, 39, 0, mem);
				}
				else if (strncmp(p, "nom", 3) == 0 && non_alnum(p[3]))
				{
					uint8_t		count = 0;
					uint32_t	offset;

					if ((p = va_parse_u32(buf + 3, 1, 1, &offset)) == NULL)
						goto __forest_end;

					if (pkmn == PKMN_GRE)
						offset = (offset - 1) * 6;
					else
						offset = (offset - 1) * 11;

					ptr = (uint8_t*)GET_REAL_ADDR(pkmn_addr[NAME] + offset);
					if (pkmn == PKMN_GRE)
					{
						static const unsigned char	boin[58] = {
							1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,
							0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
							0,0,0,0,0,0,
							1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,
							0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0
						};

						for (; ; p++, count++)
						{
							if (count == 5 || *p == '\0' || *p > 'z' || (*p < '0' && *p != '-'))
								break ;
							if (*p >= '0' && *p <= '9') {
								*(ptr++) = (*p - '0') + 0xf6;
							}
							else if (*p == '-') {
								*(ptr++) = 0xe3; // -
							}
							else if (*p == ' ') {
								*(ptr++) = 0x0; // ' '
							}
							else
							{
								if (*p >= 'a')
								{
									if (*p == 't' && (p[1] == 's' || p[1] == 'S')
										&& (p[2] == 'u' || p[2] == 'U'))
									{
										p += 2;
										*(ptr++) = 0xc2; // TSU
									}
									else if (*p == 'n' && (p[1] == '\'' || p[1] == '\0'
										|| (!boin[p[1] - 'a'] && p[1] != 'y' && p[1] != 'Y')))
									{
										if (p[1] == '\'' || p[1] == 'y' || p[1] == 'Y')
											p++;
										*(ptr++) = 0xde; // N
									}
									else
									{
										if (get_hiragana(&p, &ptr, &count) == -1) {
											puts("error");
											break;
										}
									}
								}
								else if (*p >= 'A' && *p <= 'Z')
								{
									if (*p == 'T' && (p[1] == 's' || p[1] == 'S')
										&& (p[2] == 'u' || p[2] == 'U'))
									{
										p += 2;
										*(ptr++) = 0x91; // TSU
									}
									else if (*p == 'N' && (p[1] == '\'' || p[1] == '\0'
										|| (!boin[p[1] - 'A'] && p[1] != 'y' && p[1] != 'Y')))
									{
										if (p[1] == '\'' || p[1] == 'y' || p[1] == 'Y')
											p++;
										*(ptr++) = 0xab; // N
									}
									else
									{
										if (get_katakana(&p, &ptr, &count) == -1) {
											puts("error");
											break;
										}
									}
								}
								else
								{
									puts("error");
									break;
								}
							}
						}
						*(ptr) = 0x50;
					}
					else
					{
						for (count = 0; *p && *p != ' ' && count != 11; p++, count++)
						{
							if (*p >= 'a' && *p <= 'z')
								*(ptr++) = (uint8_t)((*p - 'a') + 0xa0);
							else if (*p >= 'A' && *p <= 'Z')
								*(ptr++) = (uint8_t)((*p - 'A') + 0x80);
							else if (*p >= '0' && *p <= '9')
								*(ptr++) = (uint8_t)((*p - '0') + 0xf6);
							else
							{
								switch (*p)
								{
									case ' ': *(ptr++) = 229U; break;
									case '(': *(ptr++) = 154U; break;
									case ')': *(ptr++) = 155U; break;
									case ':': *(ptr++) = 156U; break;
									case ';': *(ptr++) = 157U; break;
									case '[': *(ptr++) = 158U; break;
									case ']': *(ptr++) = 159U; break;
									case '?': *(ptr++) = 230U; break;
									case '!': *(ptr++) = 231U; break;
									case '.': *(ptr++) = 232U; break;
									default: puts("illegal char"); goto __forest_end;
								}
							}
						}
						while (count != 11)
						{
							*(ptr++) = 0x50;
							count++;
						}
					}
				}
				else if (strncmp(p, "pkmn_sup", 8) == 0 && non_alnum(p[8]))
				{
					uint8_t		*end;
					uint8_t		*ptr;
					uint8_t		*ptr2;
					uint8_t		*end2;
					uint8_t		exit;
					uint32_t	val;
					uint32_t	num, offset, offset2, of2val;
	
					if (va_parse_u32(buf + 8, 0, 1, &num) == NULL)
						goto __forest_end;
/*					
					num		-= 1;
					offset	= num * 44;
					if (pkmn == PKMN_GRE)
						offset2 = num * 6;
					else
						offset2 = num * 11;
*/					
					of2val = (pkmn == PKMN_GRE) ? 6 : 11;
					if (num == 0 || num == 0xffffffffU)
					{
						num = 0;
						exit = 0;
						offset = 0;
						offset2 = 0;
					}
					else if (num > 6)
					{
						puts("error: argument > 6");
						goto __forest_end;
					}
					else
					{
						num--;
						offset = num * 44;
						offset2 = num * of2val;
						exit = 1;
					}
					ptr = GET_REAL_ADDR(pkmn_addr[NO] + offset);
					end = ptr + 44;
					ptr2 = GET_REAL_ADDR(pkmn_addr[NAME] + offset2);
					end2 = ptr2 + of2val;
					for (; ; num++, end += 44, end2 += of2val)
					{
						for (; ptr != end; ptr++)		*ptr = 0;
						for (; ptr2 != end2; ptr2++)	*ptr2 = 0;
						if (num == 5)					break;
						else if (exit == 1)
						{
							ptr = GET_REAL_ADDR(pkmn_addr[HEAD]);
							val = *ptr;
							*ptr = val - 1;
							ptr += 1 + num;
							//ptr = GET_REAL_ADDR(pkmn_addr[NAME]);
							memmove(ptr2 - of2val, ptr2, of2val);
							memset(ptr2, 0, of2val);
							memmove(ptr, ptr + 1, 7 - num);
							break;
						}
					}
					if (exit == 0)
					{
						ptr = (uint8_t*)GET_REAL_ADDR(pkmn_addr[HEAD]);
						*ptr = 0;
						ptr[1] = 0xffU;
					}
/*
					ptr = (uint8_t*)GET_REAL_ADDR(pkmn_addr[NO] + offset);
					end = ptr + 44;

					while (ptr != end)
						*(ptr++) = 0;

					ptr = (uint8_t*)GET_REAL_ADDR(pkmn_addr[NAME] + offset2);
					if (pkmn == PKMN_GRE)
						end = ptr + 6;
					else
						end = ptr + 11;

					while (ptr != end)
						*(ptr++) = 0;

					ptr = (uint8_t*)GET_REAL_ADDR(pkmn_addr[HEAD]);
					val = *ptr;
					*ptr = val - 1;
					ptr += 1 + num;
					memmove(ptr, ptr + 1, 7 - num);
*/
				}
				else if (strncmp(p, "maj", 3) == 0 && non_alnum(p[3]))
				{
					int			loop = 6;
					int			count = 0;
					uint8_t		*ptr2;

					ptr2 = (uint8_t*)GET_REAL_ADDR(pkmn_addr[HEAD] + 1);
					ptr = (uint8_t*)GET_REAL_ADDR(pkmn_addr[NO]);
					for (; loop; loop--, ptr += 44, ptr2++)
					{
						if (*ptr)
						{
							*ptr2 = *ptr;
							count++;
						}
						else
							break;
					}
					*ptr2 = 0xffu;
					*GET_REAL_ADDR(pkmn_addr[HEAD]) = (uint8_t)count;
				}
				else if (strncmp(p, "obj_pop", 7) == 0 && non_alnum(p[7]))
				{
					uint8_t		*ptr;

					ptr = GET_REAL_ADDR(pkmn_addr[OBJ]);
					unsigned int offset = *ptr;
					if (offset == 0)
						goto __forest_end;
					offset--;
					*ptr = offset;
					ptr += 3 + offset * 2;
					*(--ptr) = 0x0;
					*(--ptr) = 0xffu;
				}
				else if (strncmp(p, "obj_push", 8) == 0 && non_alnum(p[8]))
				{
					uint32_t	offset;
					uint32_t	object = 0;
					uint32_t	quantity = 1;
					uint8_t		*ptr;

					if (va_parse_u32(p + 8, 2, 2, &object, &quantity) == NULL)
						goto __forest_end;

					ptr = GET_REAL_ADDR(pkmn_addr[OBJ]);
					offset = *ptr;
					offset++;
					*ptr = offset;
					ptr += 1 + ((offset * 2) - 2);

					*(ptr++) = (uint8_t)object;
					*(ptr++) = (uint8_t)quantity;
					*ptr = 0xffu;
				}
				else if (strncmp(p, "stat", 4) == 0 && non_alnum(p[4]))
				{
					unsigned int	att;
					unsigned int	def;
					unsigned int	vit;
					unsigned int	spe;
					unsigned int	offset;

					if (va_parse_u32(p + 4, 2, 5, &offset, &att, &def, &vit, &spe) == NULL)
						goto __forest_end;

					offset = ((offset - 1) * 44);
					*GET_REAL_ADDR(pkmn_addr[ATT] + offset) = (uint8_t)((att & 0xff00) >> 8);
					*GET_REAL_ADDR(pkmn_addr[ATT] + 1 + offset) = (uint8_t)(att & 0xff);
					if (def != 0xffffffffU) {
						*GET_REAL_ADDR(pkmn_addr[DEF] + offset) = (uint8_t)((def & 0xff00) >> 8);
						*GET_REAL_ADDR(pkmn_addr[DEF] + 1 + offset) = (uint8_t)(def & 0xff);
						if (vit != 0xffffffffU) {
							*GET_REAL_ADDR(pkmn_addr[VIT] + offset) = (uint8_t)((vit & 0xff00) >> 8);
							*GET_REAL_ADDR(pkmn_addr[VIT] + 1 + offset) = (uint8_t)(vit & 0xff);
							if (spe != 0xffffffffU) {
								*GET_REAL_ADDR(pkmn_addr[SPE] + offset) = (uint8_t)((spe & 0xff00) >> 8);
								*GET_REAL_ADDR(pkmn_addr[SPE] + 1 + offset) = (uint8_t)(spe & 0xff);
							}
						}
					}
				}
				else if (strncmp(p, "argent", 4) == 0 && non_alnum(p[6]))
				{
					unsigned int cash = 0;

					if (va_parse_u32(p + 4, 1, 1, &cash) == NULL)
						goto __forest_end;

					if (pkmn == PKMN_GRE)
					{
						*GET_REAL_ADDR(pkmn_addr[CASH]) = (uint8_t)((cash & 0xffff00) >> 16);
						*GET_REAL_ADDR(pkmn_addr[CASH] + 1) = (uint8_t)((cash & 0xff00) >> 8);
						*GET_REAL_ADDR(pkmn_addr[CASH] + 2) = (uint8_t)(cash & 0xff);
					}
					else
					{
						*GET_REAL_ADDR(pkmn_addr[CASH]) = (uint8_t)((cash & 0xff00) >> 8);
						*GET_REAL_ADDR(pkmn_addr[CASH] + 1) = (uint8_t)(cash & 0xff);
					}
				}
				else
					write(1, "syntax error\n", 13);
			}
			else
				write(1, "syntax error\n", 13);
__forest_end:
			if (j)
				add_hist(&hist, buf, j, &hsize);
			curh = NULL;
			buf[j = 0] = '\0';
			i = 0;
		}
	}
	term_noecho_mode(0);
	write(2, "\e[?25h", 6);
}


