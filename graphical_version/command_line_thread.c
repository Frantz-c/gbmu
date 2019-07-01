
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
#include "memory_map.h"
#include "test_hash.c"
/*
#include "SDL.h"
#include "test.h"
#include "registers.h"
#include "execute.h"
*/

/*
	prochainement :

	addr pokemon 1 proprietaire = 0xd233
	0xcf45 -> nom ?
*/

extern int		log_file;

extern void		close_log_file_and_exit(int sig);

static int		term_noecho_mode(int stat)
{
	static int				mode = -1;
	static struct termios	default_mode;
	static struct termios	noecho_mode;

	if (mode == -1)
	{
		if (tcgetattr(0, &default_mode) == -1)
			return (-1);
		noecho_mode = default_mode;
		noecho_mode.c_lflag &= ~(ECHO | ICANON);
		noecho_mode.c_cc[VMIN] = 1;
		noecho_mode.c_cc[VTIME] = 5;
		mode = 0;
	}
	if (mode == stat)
		return (0);
	if (stat == 1)
		return (tcsetattr(0, TCSANOW, &noecho_mode));
	if (stat == 0)
		return (tcsetattr(0, TCSANOW, &default_mode));
	return (-1);
}

static inline char		*left_trim(char *s, int *type)
{
	while (*s == ' ' || *s == '\t')
		s++;
	if (*s == '0')
	{
		if (s[1] == 'x')
		{
			*type = 2;
			s += 2;
			while (*s == '0')
				s++;
			return (s);
		}
		*type = 1;
		s++;
		while (*s == '0')
			s++;
		return (s);
	}
	else
	{
		*type = 0;
	}
	if (*s < '0' || *s > '9')
		return (NULL);
	return (s);
}

static unsigned int				get_base_value(char c)
{
	if (c >= 'a' && c <= 'f')
		return (c - ('a' - 10));
	if (c >= 'A' && c <= 'F')
		return (c - ('a' - 10));
	return (c - '0');
}

static inline unsigned int		ft_strtoi(char **s, int type)
{
	unsigned int	n;

	n = 0;
	if (type == 0)
	{
		while (1)
		{
			if (**s < '0' || **s > '9')
				break ;
			n *= 10;
			n += **s - '0';
			(*s)++;
		}
	}
	else if (type == 1)
	{
		while (1)
		{
			if (**s < '0' || **s > '6')
				break ;
			n *= 8;
			n += **s - '0';
			(*s)++;
		}
	}
	else
	{
		while (1)
		{
			if (**s > 'f' || (**s > 'F' && **s < 'a')
					|| (**s > '9' && **s < 'A') || **s < '0')
				break ;
			n *= 16;
			n += get_base_value(**s);
			(*s)++;
		}
	}
	return (n);
}

static unsigned int		atoi_hexa(char **s, int *err)
{
	int	type; // 0 = base 10, 1 = octal, 2 = hexa

	if ((*s = left_trim(*s, &type)) == NULL)
	{
		if (err)
			*err = 1;
		return (0);
	}
	if (err)
		*err = 0;
	return (ft_strtoi(s, type));
}

static int		non_alnum(char c)
{
	return (c < '0' || (c > '9' && c < 'A') || (c > 'Z' && c < 'a') || c > 'Z');
}

static void		write_dump_switchable_mem_fd(int fd, void *start, unsigned int length, char *buf, const char *title)
{
	unsigned int	j;
	uint8_t			*ptr;

	ptr = GET_REAL_ADDR((uint16_t)start);
	memcpy(buf, title, (j = strlen(title)));
	for (uint8_t *ptr_end = ptr + length; ptr != ptr_end; start++, ptr++)
	{
		if (((uint16_t)start & 0xf) == 0)
			j += sprintf(buf + j, "\n%p:  ", start);
		j += sprintf(buf + j, "%4hhx ", *ptr);
		if (j >= 0xfff0)
		{
			write(fd, buf, j);
			j = 0;
		}
	}
	if (j)
		write(fd, buf, j);
}

static void		write_dump_fd(int fd, void *start, unsigned int length, char *buf, const char *title)
{
	unsigned int	j;

	memcpy(buf, title, (j = strlen(title)));
	for (void *end = start + length; start != end; start++)
	{
		if (((uint16_t)start & 0xf) == 0)
			j += sprintf(buf + j, "\n%p:  ", start);
		j += sprintf(buf + j, "%4hhx ", *GET_REAL_ADDR((uint16_t)start));
		if (j >= 0xfff0)
		{
			write(fd, buf, j);
			j = 0;
		}
	}
	if (j)
		write(fd, buf, j);
}

// 0 < min_arg <= max_arg
static char		*va_parse_u32(char *p, int min_arg, int max_arg, ...)
{
	uint32_t	*tmp;
	int			cur = 0;
	int			err;
	va_list	ap;

	va_start(ap, max_arg);

	while (*p == ' ') p++;
	if (*p != '(' && p[-1] != ' ')
		goto __error;
	if (*p == '(') {
		p++;
		while (*p == ' ') p++;
	}
	cur++;

	tmp = va_arg(ap, uint32_t*);
	*tmp = atoi_hexa(&p, &err);
	if (err) goto __error;

	while (cur < max_arg)
	{
		while (*p == ' ') p++;
		if (*p != ',' && p[-1] != ' ')
		{
			if (min_arg <= cur && non_alnum(*p))
			{
				while (cur < max_arg)
				{
					tmp = va_arg(ap, uint32_t*);
					*tmp = 0xffffffffU;
					cur++;
				}
				while (*p == ' ') p++;
				return (p);
			}
			goto __error;
		}
		if (*p == ',') {
			p++;
			while (*p == ' ') p++;
		}
		cur++;

		tmp = va_arg(ap, uint32_t*);
		*tmp = atoi_hexa(&p, &err);
		if (err) goto __error;
	}
	va_end(ap);
	while (*p == ' ') p++;
	return (p);

__error:
	va_end(ap);
	write(1, "syntax error\n", 13);
	return (NULL);
}



enum pkmn_offset_e
{
	HEAD,NO,CHP,ATT1,ATT2,ATT3,ATT4,ID,XP,PP1,
	PP2,PP3,PP4,LVL,HP,ATT,DEF,VIT,SPE,NAME,
	OBJ,OBJ_PC,CASH
};

#define PKMN_RB		0x1
#define PKMN_GRE	0x2

# define K_UP			0x415b1b
# define K_DOWN			0x425b1b
# define K_LEFT			0x445b1b
# define K_RIGHT		0x435b1b

#define MAX_HISTORY		30

typedef struct	s_hist
{
	char			*s;
	unsigned int	l;
	struct s_hist	*next;
	struct s_hist	*prev;
}
t_hist;

static void	free_hist(t_hist *hist)
{
	free(hist->s);
	free(hist);
}

static void	load_hist(char *s, t_hist *h)
{
	memcpy(s, h->s, h->l + 1);
}

static t_hist	*new_hist(char *s, unsigned int l)
{
	t_hist *new;

	new = malloc(sizeof(t_hist));
	new->s = malloc(l + 1);
	memcpy(new->s, s, l + 1);
	new->l = l;
	new->prev = NULL;
	new->next = NULL;
	return (new);
}

static void	add_hist(t_hist **hist, char *str, unsigned int len, unsigned int *hsize)
{
	t_hist	*tmp;
	t_hist	*new;

	new = new_hist(str, len);

	if (*hist == NULL)
	{
		*hist = new;
		*hsize += 1;
		return;
	}
	if (*hsize > MAX_HISTORY)
	{
		for (tmp = *hist; tmp->next->next; tmp = tmp->next);
		free_hist(tmp->next);
		tmp->next = NULL;
	}
	else
		*hsize += 1;
	tmp = *hist;
	*hist = new;
	new->next = tmp;
	tmp->prev = new;
}

/*
 *	commandes : 
 *		get 0xaddr\n
 *		set 0xaddr=0xvalue\n
 *		dump 0xstart-0xend
 *		fdump filename
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
	unsigned short	addr;
//	unsigned short	addr2;
	unsigned char	value;
	unsigned int	tmp;
	unsigned char	*ptr;
	unsigned int	pkmn;
	int				err;
	uint16_t		pkmn_addr[23] = {0};

	write(2, "\e[?25l", 6);

	if (strncmp(g_cart.game_title, "POKEMON RED", 11) == 0
			|| strncmp(g_cart.game_title, "POKEMON BLU", 11) == 0)
	{
		pkmn = PKMN_RB;
		pkmn_addr[HEAD] = 0xd168U;
		pkmn_addr[NO] = 0xd170U;
		pkmn_addr[CHP] = 0xd171U;
		pkmn_addr[ATT1] = 0xd178U;
		pkmn_addr[ATT2] = 0xd179U;
		pkmn_addr[ATT3] = 0xd17aU;
		pkmn_addr[ATT4] = 0xd17bU;
		pkmn_addr[ID] = 0xd17cU;
		pkmn_addr[XP] = 0xd184U;
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
		pkmn_addr[NAME] = 0xd2baU;
		pkmn_addr[OBJ] = 0xd322U;
		pkmn_addr[OBJ_PC] = 0xd53fU;
		pkmn_addr[CASH] = 0xd34dU;

	}
	else if (strncmp(g_cart.game_title, "POKEMON GRE", 11) == 0)
	{
		pkmn = PKMN_GRE;
		pkmn_addr[HEAD] = 0xd123U;
		pkmn_addr[NO] = 0xd12bU;
		pkmn_addr[CHP] = 0xd12cU;
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
		pkmn_addr[NAME] = 0xd257U;
		pkmn_addr[OBJ] = 0xd2a1U;
		pkmn_addr[OBJ_PC] = 0xd4b9U;
		pkmn_addr[CASH] = 0xd2cbU;

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
//		else
//		{
//			write(1, "\e[0;41m \e[0m", 12);
//		}


		if (chr == 127 || chr == K_LEFT || chr == K_RIGHT || chr == K_UP || chr == K_DOWN)
			goto __read;

		else if (chr == '\n')
		{
			buf[j] = '\0';
			write(1, "\e[512C\n", 7);

			if (strcmp(buf, "exit") == 0)
			{
				term_noecho_mode(0);
				close_log_file_and_exit(0);
			}
			if (strncmp(buf, "rset", 4) == 0 && non_alnum(buf[4]))
			{
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

				value = atoi_hexa(&p, &err);
				if (err) {
					puts("\n\e[0;31msyntax error\e[0m");
					goto __forest_end;
				}

				*(GET_REAL_ADDR(addr)) = (uint8_t)value;
				addr++;
				
				while (*p == ' ') p++;
				if (*p == ',' || p[-1] == ' ')
					goto __next_set;
			}
			else if (strncmp(buf, "set", 3) == 0 && non_alnum(buf[3]))
			{
				uint32_t	addr, val;
				
				if (va_parse_u32(buf + 3, 2, 2, &addr, &val) == NULL)
					goto __forest_end;

				*(GET_REAL_ADDR((uint16_t)addr)) = value;
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
			else if (strcmp(buf, "help") == 0)
			{
				#define HELP	"memory functions:\n"\
								"    \e[0;33mset   \e[0m(\e[1;34m2b\e[0m addr, \e[1;34m1b\e[0m value)\n"\
								"    \e[0;33mget   \e[0m(\e[1;34m2b\e[0m addr)\n"\
								"    \e[0;33mrset  \e[0m(\e[1;34m2b\e[0m addr, \e[1;34m1b\e[0m value, ...)\n"\
								"    \e[0;33mdump  \e[0m(\e[1;34m2b\e[0m start, \e[1;34m2b\e[0m end)\n"\
								"    \e[0;33mfdump \e[0m(\e[1;34mstring\e[0m filename)\n\n"\
								"pokemon functions:\n"\
								"    \e[0;33mpok   \e[0m(n, \e[1;34m1b\e[0m numero);\n"\
								"    \e[0;33mpv    \e[0m(n, \e[1;34m2b\e[0m max, \e[1;34m2b\e[0m cur);\n"\
								"    \e[0;33matt1  \e[0m(n, \e[1;34m1b\e[0m numero, \e[1;34m1b\e[0m pp);\n"\
								"    \e[0;33matt2  \e[0m(n, \e[1;34m1b\e[0m numero, \e[1;34m1b\e[0m pp);\n"\
								"    \e[0;33matt3  \e[0m(n, \e[1;34m1b\e[0m numero, \e[1;34m1b\e[0m pp);\n"\
								"    \e[0;33matt4  \e[0m(n, \e[1;34m1b\e[0m numero, \e[1;34m1b\e[0m pp);\n"\
								"    \e[0;33mid    \e[0m(n, \e[1;34m2b\e[0m id);\n"\
								"    \e[0;33mxp    \e[0m(n, \e[1;34m3b\e[0m xp);\n"\
								"    \e[0;33mniv   \e[0m(n, \e[1;34m1b\e[0m niveau);\n"\
								"    \e[0;33mfor   \e[0m(n, \e[1;34m2b\e[0m for);\n"\
								"    \e[0;33mdef   \e[0m(n, \e[1;34m2b\e[0m def);\n"\
								"    \e[0;33mvit   \e[0m(n, \e[1;34m2b\e[0m vit);\n"\
								"    \e[0;33mspe   \e[0m(n, \e[1;34m2b\e[0m spe);\n"\
								"    \e[0;33mname  \e[0m(n, \e[1;34mstring\e[0m name);\n"\
								"    \e[0;33mupdate\e[0m(\e[0;34mvoid\e[0m);\n"\
								"    \e[0;33mdel   \e[0m(n);\n"\
								"    \e[0;33mpush  \e[0m(\e[1;34m1b\e[0m object, \e[1;34m1b\e[0m quantity);\n"\
								"    \e[0;33mpop   \e[0m(\e[0;34mvoid\e[0m);\n"\
								"    \e[0;33mstat  \e[0m(\e[1;34m2b\e[0m for, \e[1;34m2b\e[0m def, "\
									"\e[1;34m2b    \e[0m vit, \e[1;34m2b\e[0m spe);\n"\
								"    \e[0;33mcash  \e[0m(\e[1;34m2b\e[0m cash); [red/blue]\n"\
								"    \e[0;33mcash  \e[0m(\e[1;34m3b\e[0m cash); [green]\n"\
								"  n = [1-6]\n"\
								"  1b = 1 byte, 2b = 2 bytes, 3b = 3 bytes\n\n"\
								"syntax:\n"\
								"    mnemonic(a,b,...)\n"\
								"    mnemonic a b ...\n"\
								"    mnemonic a, b, ...\n"\
								"    mnemonic(a b ...)\n\n"\

				write(1, HELP, strlen(HELP));
			}
/*
 *	POKEMON ROUGE, BLEU, VERT
 */
			else if (pkmn)
			{
				p = buf;

				if (strncmp(p, "pok", 3) == 0 && non_alnum(p[3]))
				{
					unsigned int	val;
					unsigned int	offset;

					if (va_parse_u32(p + 3, 2, 2, &offset, &val) == NULL)
						goto __forest_end;

					offset = (offset - 1) * 44;
					*GET_REAL_ADDR(pkmn_addr[NO] + offset) = (uint8_t)(val & 0xffu);
				}
				else if (strncmp(p, "pv", 2) == 0 && non_alnum(p[2]))
				{
					unsigned int	val1;
					unsigned int	val2;
					unsigned int	offset;

					if (va_parse_u32(p + 2, 3, 3, &offset, &val1, &val2) == NULL)
						goto __forest_end;

					offset = (offset - 1) * 44;

					*GET_REAL_ADDR(pkmn_addr[HP] + offset) = (uint8_t)((val1 & 0xff00) >> 8);
					*GET_REAL_ADDR(pkmn_addr[HP] + 1 + offset) = (uint8_t)(val1 & 0xff);

					*GET_REAL_ADDR(pkmn_addr[CHP] + offset) = (uint8_t)((val2 & 0xff00) >> 8);
					*GET_REAL_ADDR(pkmn_addr[CHP] + 1 + offset) = (uint8_t)(val2 & 0xff);
				}
				else if (strncmp(p, "att1", 4) == 0 && non_alnum(p[4]))
				{
					unsigned int	val1;
					unsigned int	val2;
					unsigned int	offset;

					if (va_parse_u32(p + 4, 3, 3, &offset, &val1, &val2) == NULL)
						goto __forest_end;

					offset = (offset - 1) * 44;
					*GET_REAL_ADDR(pkmn_addr[ATT1] + offset) = (uint8_t)(val1 & 0xff);
					*GET_REAL_ADDR(pkmn_addr[PP1]  + offset) = (uint8_t)(val2 & 0xff);
				}
				else if (strncmp(p, "att2", 4) == 0 && non_alnum(p[4]))
				{
					unsigned int	val1;
					unsigned int	val2;
					unsigned int	offset;

					if (va_parse_u32(p + 4, 3, 3, &offset, &val1, &val2) == NULL)
						goto __forest_end;

					offset = (offset - 1) * 44;
					*GET_REAL_ADDR(pkmn_addr[ATT2] + offset) = (uint8_t)(val1);
					*GET_REAL_ADDR(pkmn_addr[PP2]  + offset) = (uint8_t)(val2);
				}
				else if (strncmp(p, "att3", 4) == 0 && non_alnum(p[4]))
				{
					unsigned int	val1;
					unsigned int	val2;
					unsigned int	offset;

					if (va_parse_u32(p + 4, 3, 3, &offset, &val1, &val2) == NULL)
						goto __forest_end;

					offset = (offset - 1) * 44;
					*GET_REAL_ADDR(pkmn_addr[ATT3] + offset) = (uint8_t)(val1);
					*GET_REAL_ADDR(pkmn_addr[PP3]  + offset) = (uint8_t)(val2);
				}
				else if (strncmp(p, "att4", 4) == 0 && non_alnum(p[4]))
				{
					unsigned int	val1;
					unsigned int	val2;
					unsigned int	offset;

					if (va_parse_u32(p + 4, 3, 3, &offset, &val1, &val2) == NULL)
						goto __forest_end;

					offset = (offset - 1) * 44;
					*GET_REAL_ADDR(pkmn_addr[ATT4] + offset) = (uint8_t)(val1);
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
				else if (strncmp(p, "name", 4) == 0 && non_alnum(p[4]))
				{
					int count = 0;
					int	offset;

					if ((p = va_parse_u32(buf + 4, 1, 1, &offset)) == NULL)
						goto __forest_end;

					if (pkmn == PKMN_GRE)
						offset = (offset - 1) * 6;
					else
						offset = (offset - 1) * 11;

					ptr = (uint8_t*)GET_REAL_ADDR(pkmn_addr[NAME] + offset);
					if (pkmn == PKMN_GRE)
					{
						/*
							0xd257

							0 - 3 =  ' '
							katakana
							4 = 'O" '
							5 = 'GA' 6 = 'GI' 7 = 'GU' 8 = 'GE' 9 = 'GO'
							a = 'ZA' b = 'JI' c = 'ZU' d = 'ZE' e = 'ZO'
							f = 'DA' 10 = 'DJI' 11 = 'DJU' 12 = 'DE' 13 = 'DO'
							14 = 'NA"' 15 = 'NI"' 16 = 'NU"' 17 = 'NE"' 18 = 'NO"'
							19 = 'BA' 1a = 'BI' 1b = 'BU' 1c = 'BO' 1d = 'MA"'
							1e - 25 -> ...

							hiragana
							26 = 'GA' 27 = 'GI' 28 = 'GU' 29 = 'GE' 2a = 'GO'
							2b = 'ZA' 2c = 'JI' 2d = 'ZU' 2e = 'ZE' 2f = 'ZO'
							30 = 'DA' 31 = 'DJI' 32 = 'DZU' 33 = 'DE' 34 = 'DO'
							35 - 39 = ...
							3a = 'BA' 3b = 'BI' 3c = 'BU' 3d = 'BE' 3e = 'BO'
							3f = ...

							katakana
							40 = 'PA' 41 = 'PI' 42 = 'PU' 43 = 'PO'
							hiragana
							44 = 'PA' 45 = 'PI' 46 = 'PU' 47 = 'PE' 48 = 'PO'

							49 - 54 = bug

					>> 115 -> ID
					>> 116 -> NO
					
							katakana
							80 = 'A' 81 = 'I' 82 = 'U' 83 = 'E' 84 = 'O'
							85 = 'KA'
							...
							8a = 'SA'
							...
							8f = 'TA'
							...
							94 = 'NA'
							...
							99 = 'HA' 9a = 'HI' 9b = 'HU' 9c = 'HO'
							...
							9d = 'MA'
							...
							a2 = 'YA' a3 = 'YU' a4 = 'YO'
							...
							a5 = 'RA' a6 = 'RU' a7 = 'RE' a8 = 'RO'
							...
							a9 = 'WA' aa = 'WO'
							ab = 'N'
							...
							ac = 'tsu'
							ad = 'ya' ae = 'yu' af = 'yo'
							b0 = 'i'
							...
							hiragana
							b1 = 'A' I U E O
							...
							b6 = 'KA' KI KU KE KO
							...
							bb = 'SA' SHI SU SE SO
							...
							c0 = 'TA' CHI TSU TE TO
							...
							c5 = 'NA' NI NU NE NO
							...
							ca = 'HA' HI HU HE HO
							...
							cf = 'MA' MI MU ME MO
							...
							d4 = 'YA' YU YO
							...
							d7 = 'RA' RI RU RE RO
							...
							dc = 'WA' WO
							...
							de = 'N'
							df = 'tsu' ya yu yo
							...

							e3 = '-'
							e4 = 'º'
							e5 = '-'
							e6 = '?'
							e7 = '!'
							e8 = 'maru'
							e9 = 'a' katakana
							ea = 'u' katakana
							eb = 'e' katakana
							ec = 'fleche droite vide'
							ed = 'fleche droite'
							ee = 'fleche bas'
							ef = 'male'
							f0 = 'yen'
							f1 = 'x'
							f2 = '.'
							f3 = '/'
							f4 = 'o' katakana
							f5 = 'femelle'
							f6 -> ff = 0 1 2 3 4 5 6 7 8 9

						*/
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
						while (1)
						{
							if (count == 11 || *p > 'z' || *p < 'a')
								break ;
							*(ptr++) = (uint8_t)((*p - 'a') + 0x80);
							p++;
							count++;
						}
						while (count != 11)
						{
							*(ptr++) = 0x50;
							count++;
						}
					}
				}
				else if (strncmp(p, "del", 3) == 0 && non_alnum(p[3]))
				{
					uint8_t	*end;
					uint32_t	num, offset, offset2;
	
					if (va_parse_u32(buf + 3, 1, 1, &num) == NULL)
						goto __forest_end;

					num		-= 1;
					offset	= num * 44;
					if (pkmn == PKMN_GRE)
						offset2 = num * 6;
					else
						offset2 = num * 11;

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
					value = *ptr;
					*ptr = value - 1;
					ptr += 1 + num;
					memmove(ptr, ptr + 1, 7 - num);
				}
				else if (strncmp(p, "update", 6) == 0 && non_alnum(p[6]))
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
				else if (strncmp(p, "pop", 3) == 0 && non_alnum(p[3]))
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
				else if (strncmp(p, "push", 4) == 0 && non_alnum(p[4]))
				{
					uint32_t	offset;
					uint32_t	object = 0;
					uint32_t	quantity = 1;
					uint8_t		*ptr;

					if (va_parse_u32(p + 4, 2, 2, &object, &quantity) == NULL)
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
					}
					if (vit != 0xffffffffU) {
						*GET_REAL_ADDR(pkmn_addr[VIT] + offset) = (uint8_t)((vit & 0xff00) >> 8);
						*GET_REAL_ADDR(pkmn_addr[VIT] + 1 + offset) = (uint8_t)(vit & 0xff);
					}
					if (spe != 0xffffffffU) {
						*GET_REAL_ADDR(pkmn_addr[SPE] + offset) = (uint8_t)((spe & 0xff00) >> 8);
						*GET_REAL_ADDR(pkmn_addr[SPE] + 1 + offset) = (uint8_t)(spe & 0xff);
					}
				}
				else if (strncmp(p, "cash", 4) == 0 && non_alnum(p[4]))
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


