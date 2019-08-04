#ifndef CHEAT_H
# define CHEAT_H

enum pkmn_offset_e
{
	HEAD,NO,STATUS,CHP,ATT1,ATT2,ATT3,ATT4,ID,
	XP,PP1,PP2,PP3,PP4,LVL,HP,ATT,DEF,VIT,
	SPE,NAME,OBJ,OBJ_PC,CASH,CBT_HP,ADV_HP,
	CBT_ATT,CBT_DEF,CBT_VIT,CBT_SPE,CBT_CHP,
	CBT_ATT_2,CBT_DEF_2,CBT_VIT_2,CBT_SPE_2,
	ROM_ATT,ROM_ATT_NAME,ROM_POK_NAME,OWNER,
	BADGES,RIVAL_NAME,USER_NAME,CSTAT,EV,IV_HP,
	IV_FOR,IV_VIT,IV_DEF,IV_SPE
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



extern int		log_file;
extern void		close_log_file_and_exit(int sig);

extern int		term_noecho_mode(int stat);
extern uint32_t	atoi_hexa(char **s, int *err);
extern int	non_alnum(char c);
extern void	write_dump_switchable_mem_fd(int fd, void *start, unsigned int length, char *buf, const char *title);
extern void	write_dump_fd(int fd, void *start, unsigned int length, char *buf, const char *title);
extern char	*va_parse_u32(char *p, int min_arg, int max_arg, ...);
extern void	load_hist(char *s, t_hist *h);
extern void	add_hist(t_hist **hist, char *str, unsigned int len, unsigned int *hsize);

extern int	is_missingno(uint32_t n);
extern int	set_string_red(char *input, uint8_t maxlen, int fill, uint8_t *output);
extern int	set_string_green(char *input, uint8_t maxlen, uint8_t *output);

#endif
