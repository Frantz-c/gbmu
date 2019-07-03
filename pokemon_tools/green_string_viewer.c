
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

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

static inline unsigned int		ft_strtoi(char *s, int type)
{
	unsigned int	n;

	n = 0;
	if (type == 0)
	{
		while (1)
		{
			if (*s < '0' || *s > '9')
				break ;
			n *= 10;
			n += *s - '0';
			(s)++;
		}
	}
	else if (type == 1)
	{
		while (1)
		{
			if (*s < '0' || *s > '6')
				break ;
			n *= 8;
			n += *s - '0';
			(s)++;
		}
	}
	else
	{
		while (1)
		{
			if (*s > 'f' || (*s > 'F' && *s < 'a')
					|| (*s > '9' && *s < 'A') || *s < '0')
				break ;
			n *= 16;
			n += get_base_value(*s);
			(s)++;
		}
	}
	return (n);
}

static unsigned int		atoi_hexa(char *s, int *err)
{
	int	type; // 0 = base 10, 1 = octal, 2 = hexa

	if ((s = left_trim(s, &type)) == NULL)
	{
		if (err)
			*err = 1;
		return (0);
	}
	if (err)
		*err = 0;
	return (ft_strtoi(s, type));
}

unsigned char	*get_file_contents(const char *file, unsigned int *length)
{
	unsigned char	*content;
	const int		fd = open(file, O_RDONLY);
	struct stat		filestat;

	if (fd == -1)
	{
		fprintf(stderr, "can't open file %s\n", file);
		return (NULL);
	}
	lstat(file, &filestat);
	if (filestat.st_size == 0)
	{
		fprintf(stderr, "empty file\n");
		return (NULL);
	}
	if ((content = malloc(filestat.st_size + 1)) != NULL)
	{
		*length = read(fd, content, filestat.st_size);
		close(fd);
	}
	return (content);
}


static const char *const	kana[256] = {
	" ", NULL, NULL, NULL, NULL,
	"ガ", "ギ", "グ", "ゲ", "ゴ",	//5
	"ザ", "ジ", "ズ", "ゼ", "ゾ",	//a
	"ダ", "ヂ", "ヅ", "デ", "ド",	//f
	NULL, NULL, NULL, NULL, NULL,	//14
	"バ", "ビ", "ブ", "ボ",			//19
	NULL, NULL, NULL, NULL, NULL,	//1d
	NULL, NULL, NULL, NULL,			//22
	"が", "ぎ", "ぐ", "げ", "ご",	//26
	"ざ", "じ", "ず", "ぜ", "ぞ",	//2b
	"だ", "ぢ", "づ", "で", "ど",	//30
	NULL, NULL, NULL, NULL, NULL,	//35
	"ば", "び", "ぶ", "べ", "ぼ",	//3a
	NULL,							//3f
	"パ", "ピ", "プ", "ポ",			//40
	"ぱ", "ぴ", "ぷ", "ぺ", "ぽ",	//44
	NULL, NULL, NULL, NULL, NULL,	//49
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, "ID", "NO",
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL,
	"ア", "イ", "ウ", "エ", "オ",	//80
	"カ", "キ", "ク", "ケ", "コ",	//85
	"サ", "シ", "ス", "セ", "ソ",	//8a
	"タ", "チ", "ツ", "テ", "ト",	//8f
	"ナ", "ニ", "ヌ", "ネ", "ノ",	//94
	"ハ", "ヒ", "フ", "ホ",			//99
	"マ", "ミ", "ム", "メ", "モ",	//9d
	"ヤ", "ユ", "ヨ",				//a2
	"ラ", "ル", "レ", "ロ",			//a5
	"ワ", "ヲ", "ン",				//a9
	"ッ", "ャ", "ュ", "ョ",			//ac
	"ィ",							//b0
	"あ", "い", "う", "え", "お",	//b1
	"か", "き", "く", "け", "こ",	//b6
	"さ", "し", "す", "せ", "そ",	//bb
	"た", "ち", "つ", "て", "と",	//c0
	"な", "に", "ぬ", "ね", "の",	//c5
	"は", "ひ", "ふ", "へ", "ほ",	//ca
	"ま", "み", "む", "め", "も",	//cf
	"や", "ゆ", "よ",				//d4
	"ら", "リ", "る", "れ", "ろ",	//d7
	"わ", "を", "ん",				//dc
	"っ", "ゃ", "ゅ", "ょ",			//df
	"ー", "゜", "-", "？", "！",	//e3
	"。", "ァ", "ゥ", "ェ", "▷",	//e8
	"▶︎", "⬇︎", "♂", "円", "x",		//ed
	".", " /", "ォ", "♀",			//f2
	"０", "１", "２", "３", "４",	//f6
	"５", "６", "７", "８", "９"	//fb
};

int		space_only(char **s)
{
	char	*p;

	while (**s)
	{
		if (**s != ' ')
			break;
		(*s)++;
	}
	if (**s == '\0')
		return (1);
	p = *s;
	while (*p) p++;
	p--;
	while (*p == ' ')
	{
		*p = '\0';
		p--;
	}
	return (0);
}

#define BUFSIZE	64

int		main(int argc, char *argv[])
{
	unsigned char	*file;
	char			*str;
	unsigned int	length;
	unsigned int	i;
	char			buf[BUFSIZE];
	unsigned int	count;
	unsigned int	end;
	int				err[2] = {0};
	int				link = 0;
	unsigned int	j;

	if (argc < 2 || argc > 4)
	{
		fprintf(stderr, "ERROR\n");
		exit(1);
	}
	file = get_file_contents(argv[1], &length);
	if (file == NULL)
		return (1);

	if (argc > 2)
		i = atoi_hexa(argv[2], err + 0);
	else
		i = 0;

	if (argc == 4)
		end = atoi_hexa(argv[3], err + 1);
	else
		end = length;

	if (*(unsigned long*)err)
		return (1);

	for (; i != end; i++)
	{
		if (kana[file[i]])
		{
			for (j = 0, count = 0; ; count++, i++)
			{
				if (i == end)
					return (0);
				if (kana[file[i]] == NULL || j >= (BUFSIZE - 4)) {
					if (j && (link || count > 2))
					{
						buf[j] = '\0';
						str = buf;
						if (!space_only(&str)) {
							if (link)
								printf("%s", str);
							else
								printf("\n0x%x: %s", i - count, str);
						}
					}
					break;
				}
				memcpy(buf + j, kana[file[i]], strlen(kana[file[i]]));
				j += strlen(kana[file[i]]);
			}
			if (kana[file[i]])
				i--;
			if (file[i - 1] > 4)
				link = 1;
		}
		else
		{
			link = 0;
		}
	}
}
/*
	0xd257

	0 - 3 =  ' '
	katakana
	4 = 'O" '
	5 = 'GA' 6 = 'GI' 7 = 'GU' 8 = 'GE' 9 = 'GO' a = 'ZA' b = 'JI' c = 'ZU' d = 'ZE' e = 'ZO' f = 'DA' 10 = 'DJI' 11 = 'DJU' 12 = 'DE' 13 = 'DO'
	14 = 'NA"' 15 = 'NI"' 16 = 'NU"' 17 = 'NE"' 18 = 'NO"' 19 = 'BA' 1a = 'BI' 1b = 'BU' 1c = 'BO' 1d = 'MA"' 1e - 25 -> ...

	hiragana
	26 = 'GA' 27 = 'GI' 28 = 'GU' 29 = 'GE' 2a = 'GO' 2b = 'ZA' 2c = 'JI' 2d = 'ZU' 2e = 'ZE' 2f = 'ZO' 30 = 'DA' 31 = 'DJI' 32 = 'DZU' 33 = 'DE' 34 = 'DO'
	35 - 39 = ...  3a = 'BA' 3b = 'BI' 3c = 'BU' 3d = 'BE' 3e = 'BO' 3f = ...

	katakana
	40 = 'PA' 41 = 'PI' 42 = 'PU' 43 = 'PO'
	hiragana
	44 = 'PA' 45 = 'PI' 46 = 'PU' 47 = 'PE' 48 = 'PO'

	49 - 54 = bug >> 115 -> ID >> 116 -> NO

	katakana
	80 = 'A' 81 = 'I' 82 = 'U' 83 = 'E' 84 = 'O'
	85 = 'KA'
	8a = 'SA'
	8f = 'TA'
	94 = 'NA'
	99 = 'HA' 9a = 'HI' 9b = 'HU' 9c = 'HO'
	9d = 'MA'
	a2 = 'YA' a3 = 'YU' a4 = 'YO'
	a5 = 'RA' a6 = 'RU' a7 = 'RE' a8 = 'RO'
	a9 = 'WA' aa = 'WO'
	ab = 'N'
	ac = 'tsu'
	ad = 'ya' ae = 'yu' af = 'yo'
	b0 = 'i'
	...
	hiragana
	b1 = 'A' I U E O
	b6 = 'KA' KI KU KE KO
	bb = 'SA' SHI SU SE SO
	c0 = 'TA' CHI TSU TE TO
	c5 = 'NA' NI NU NE NO
	ca = 'HA' HI HU HE HO
	cf = 'MA' MI MU ME MO
	d4 = 'YA' YU YO
	d7 = 'RA' RI RU RE RO
	dc = 'WA' WO
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
