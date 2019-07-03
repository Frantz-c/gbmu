#include <stdio.h>
#include <time.h>
#include <stdlib.h>

static int	hash_phonem(char **s, unsigned char **mem, int *cnt, unsigned char doubl)
{
	static const int	boin[] = {
		1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
		0,0,0,0,0,0,
		1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0
	};
	static const char hash_alpha[] = {
		/*A*/  0, /*B*/ 70, /*C*/ 10, /*D*/ 40, /*E*/  3,
		/*F*/ 45, /*G*/ 55, /*H*/ 90, /*I*/  1, /*J*/ 75,
		/*K*/ 25, /*L*/ -1, /*M*/ 15, /*N*/ 50, /*O*/  4,
		/*P*/ 60, /*Q*/ -1, /*R*/ 35, /*S*/ 30, /*T*/ 65,
		/*U*/  2, /*V*/ -1, /*W*/  5, /*X*/ 20, /*Y*/ 80,
		/*Z*/  85, -1, -1, -1, -1, -1, -1,
		/*a*/  0, /*b*/ 70, /*c*/ 10, /*d*/ 40, /*e*/  3,
		/*f*/ 45, /*g*/ 55, /*h*/ 90, /*i*/  1, /*j*/ 75,
		/*k*/ 25, /*l*/ -1, /*m*/ 15, /*n*/ 50, /*o*/  4,
		/*p*/ 60, /*q*/ -1, /*r*/ 35, /*s*/ 30, /*t*/ 65,
		/*u*/  2, /*v*/ -1, /*w*/  5, /*x*/ 20, /*y*/ 80,
		/*z*/  85
	};

	int					hash = 0;
	int					count = 0;
	register int		c;
	char				last = 0;

	while (!boin[c = (**s - 'A')])
	{
		if (hash_alpha[c] == -1)
			return (-1);
		if (**s == last) {
			hash = 0;
			count = 0;
			**mem = doubl;// df ac
			(*mem)++;
			if (*cnt == 4)
				return (-2);
			(*cnt)++;
		}
		last = **s;
		hash += hash_alpha[c];
		(*s)++;

		if (**s < 'A' || (**s > 'Z' && **s < 'a') || **s > 'z')
			return (-1);
		if (count == 2)
			return (-1);
		count++;
	}
	hash += hash_alpha[c];
	//(*s)++;
	return (hash);
}

static int		set_katakana(int hash, unsigned char **mem, int *count)
{
	static const unsigned char	kana_code2[105] = {
		0xad,	0x0,	0xae,	0x0,	0xaf,	// ja,  ji,  ju,    , jo
		0x0,	0x0,	0x0,	0x0,	0x0,
		0x0,	0x0,	0x0,	0x0,	0x0,
		0x0,	0x0,	0x0,	0x0,	0x0,
		0xad,	0x0,	0xae,	0x0,	0xaf,	// mya,    , myu,   , myo
		0xad,	0x0,	0xae,	0x0,	0xaf,	// cha, chi, chu,   , cho
		0xad,	0x0,	0xae,	0x0,	0xaf,	// kya,	   , kyu,   , kyo
		0xad,	0x0,	0xae,	0x0,	0xaf,	// sya,	shi, syu,   , syo
		0xad,	0x0,	0xae,	0x0,	0xaf,	// rya, dji, ryu,   , ryo
		0xad,	0x0,	0xae,	0x0,	0xaf,	// sha,	shi, shu,   , sho
		0x0,	0x0,	0x0,	0x0,	0x0,	//    , dzi, dzu,   ,
		0xad,	0x0,	0xae,	0x0,	0xaf,	// nya,    , nyu,   , nyo
		0xad,	0x0,	0xae,	0x0,	0xaf,	// gya,    , gyu,   , gyo
		0xad,	0x0,	0xae,	0x0,	0xaf,	// pya,    , pyu,   , pyo
		0x0,	0x0,	0x0,	0x0,	0x0,
		0xad,	0x0,	0xae,	0x0,	0xaf,	// bya,    , byu,   , byo
		0xad,	0x0,	0xae,	0x0,	0xaf,	// jya,    , jyu,   , jyo
		0x0,	0x0,	0x0,	0x0,	0x0,
		0x0,	0x0,	0x0,	0x0,	0x0,
		0xad,	0x0,	0xae,	0x0,	0xaf	// hya,    , hyu,   , hyo
	};
	static const unsigned char	kana_code[185] = {
		0x80,	0x81,	0x82,	0x83,	0x84,	//   a   i   u   e   o
		0xa9,	0x0,	0x0,	0x0,	0xaa,	//  wa              wo
		0x0,	0x0,	0x0,	0x0,	0x0,	//	c...
		0x9d,	0x9e,	0x9f,	0xa0,	0xa1,	//  ma  mi  mu  me  mo
		0xe9,	0xb0,	0xea,	0xeb,	0xf4,	//  xa  xi  xu  xe  xo
		0x85,	0x86,	0x87,	0x88,	0x89,	//  ka  ki  ku  ke  ko
		0x8a,	0x8b,	0x8c,	0x8d,	0x8e,	//  sa shi  su  se  so
		0xa5,	0xd8,	0xa6,	0xa7,	0xa8,	//  ra  ri  ru  re  ro
		0x0f,	0x10,	0x11,	0x12,	0x13,	//  da dji dzu  de  do
		0x0,	0x0,	0x9b,	0x0,	0x0,	//          fu
		0x94,	0x95,	0x96,	0x97,	0x98,	//  na  ni  nu  ne  no
		0x05,	0x06,	0x07,	0x08,	0x09,	//  ga  gi  gu  ge  go
		0x40,	0x41,	0x42,	0x47,	0x43,	//  pa  pi  pu  pe  po
		0x8f,	0x90,	0x91,	0x92,	0x93,	//  ta chi tsu  te  to
		0x19,	0x1a,	0x1b,	0x3d,	0x1c,	//  ba  bi  bu  be  bo

		0xb,	0xb,	0xb,	0x0,	0xb,	//  ja  ji  ju      jo
		0xa2,	0x0,	0xa3,	0x0,	0xa4,	//  ya      yu      yo
		0x0a,	0x0b,	0x0c,	0x0d,	0x0e,	//  za  ji  zu  ze  zo
		0x99,	0x9a,	0x9b,	0xcd,	0x9c,	//  ha  hi  hu  he  ho
		0x9e,	0x0,	0x9e,	0x0,	0x9e,	// mya,    , myu,   , myo
		0x90,	0x90,	0x90,	0x0,	0x90,	// cha, chi, chu,   , cho
		0x86,	0x0,	0x86,	0x0,	0x86,	// kya,	   , kyu,   , kyo
		0x8b,	0x8b,	0x8b,	0x0,	0x8b,	// sya,	shi, syu,   , syo
		0xd8,	0x10,	0xd8,	0x0,	0xd8,	// rya, dji, ryu,   , ryo
		0x8b,	0x8b,	0x8b,	0x0,	0x8b,	// sha,	shi, shu,   , sho
		0x0,	0x10,	0x11,	0x0,	0x0,	//    , dzi, dzu,   ,
		0x95,	0x0,	0x95,	0x0,	0x95,	// nya,    , nyu,   , nyo
		0x06,	0x0,	0x06,	0x0,	0x06,	// gya,    , gyu,   , gyo
		0x41,	0x0,	0x41,	0x0,	0x41,	// pya,    , pyu,   , pyo
		0x0,	0x0,	0x0,	0x0,	0x0,
		0x1a,	0x0,	0x1a,	0x0,	0x1a,	// bya,    , byu,   , byo
		0x0b,	0x0,	0x0b,	0x0,	0x0b,	// jya,    , jyu,   , jyo
		0x0,	0x0,	0x0,	0x0,	0x0,
		0x0,	0x0,	0x0,	0x0,	0x0,
		0x9a,	0x0,	0x9a,	0x0,	0x9a	// hya,    , hyu,   , hyo
	};

	if (kana_code[hash] == 0)
		return (-1);
	**mem = kana_code[hash];
	(*mem)++;
	if (hash >= 75)
	{
		if (kana_code2[hash - 75] == 0)
			return (0);

		(*count)++;
		if (*count == 5)
			return (0);

		**mem = kana_code2[hash - 75];
		(*mem)++;
	}
	return (0);
}


static int		set_hiragana(int hash, unsigned char **mem, int *count)
{
	static const unsigned char	kana_code2[105] = {
		0xe0,	0x0,	0xe1,	0x0,	0xe2,	// ja,  ji,  ju,    , jo
		0x0,	0x0,	0x0,	0x0,	0x0,
		0x0,	0x0,	0x0,	0x0,	0x0,
		0x0,	0x0,	0x0,	0x0,	0x0,
		0xe0,	0x0,	0xe1,	0x0,	0xe2,	// mya,    , myu,   , myo
		0xe0,	0x0,	0xe1,	0x0,	0xe2,	// cha, chi, chu,   , cho
		0xe0,	0x0,	0xe1,	0x0,	0xe2,	// kya,	   , kyu,   , kyo
		0xe0,	0x0,	0xe1,	0x0,	0xe2,	// sya,	shi, syu,   , syo
		0xe0,	0x0,	0xe1,	0x0,	0xe2,	// rya, dji, ryu,   , ryo
		0xe0,	0x0,	0xe1,	0x0,	0xe2,	// sha,	shi, shu,   , sho
		0x0,	0x0,	0x0,	0x0,	0x0,	//    , dzi, dzu,   ,
		0xe0,	0x0,	0xe1,	0x0,	0xe2,	// nya,    , nyu,   , nyo
		0xe0,	0x0,	0xe1,	0x0,	0xe2,	// gya,    , gyu,   , gyo
		0xe0,	0x0,	0xe1,	0x0,	0xe2,	// pya,    , pyu,   , pyo
		0x0,	0x0,	0x0,	0x0,	0x0,
		0xe0,	0x0,	0xe1,	0x0,	0xe2,	// bya,    , byu,   , byo
		0xe0,	0x0,	0xe1,	0x0,	0xe2,	// jya,    , jyu,   , jyo
		0x0,	0x0,	0x0,	0x0,	0x0,
		0x0,	0x0,	0x0,	0x0,	0x0,
		0xe0,	0x0,	0xe1,	0x0,	0xe2	// hya,    , hyu,   , hyo
	};
	static const unsigned char	kana_code[185] = {
		0xb1,	0xb2,	0xb3,	0xb4,	0xb5,	//   a   i   u   e   o
		0xdc,	0x0,	0x0,	0x0,	0xdd,	//  wa              wo
		0x0,	0x0,	0x0,	0x0,	0x0,	//	c...
		0xcf,	0xd0,	0xd1,	0xd2,	0xd3,	//  ma  mi  mu  me  mo
		0x0,	0x0,	0x0,	0x0,	0x0,	//  xa  xi  xu  xe  xo
		0xb6,	0xb7,	0xb8,	0xb9,	0xba,	//  ka  ki  ku  ke  ko
		0xbb,	0xbc,	0xbd,	0xbe,	0xbf,	//  sa shi  su  se  so
		0xd7,	0xd8,	0xd9,	0xda,	0xdb,	//  ra  ri  ru  re  ro
		0x30,	0x31,	0x32,	0x33,	0x34,	//  da dji dzu  de  do
		0x0,	0x0,	0xcc,	0x0,	0x0,	//          fu
		0xc5,	0xc6,	0xc7,	0xc8,	0xc9,	//  na  ni  nu  ne  no
		0x26,	0x27,	0x28,	0x29,	0x2a,	//  ga  gi  gu  ge  go
		0x44,	0x45,	0x46,	0x47,	0x48,	//  pa  pi  pu  pe  po
		0xc0,	0xc1,	0xc2,	0xc3,	0xc4,	//  ta chi tsu  te  to
		0x3a,	0x3b,	0x3c,	0x3d,	0x3e,	//  ba  bi  bu  be  bo

		0x2c,	0x2c,	0x2c,	0x0,	0x2c,	//  ja  ji  ju      jo
		0xd4,	0x0,	0xd5,	0x0,	0xd6,	//  ya      yu      yo
		0x2b,	0x2c,	0x2d,	0x2e,	0x2f,	//  za  ji  zu  ze  zo
		0xca,	0xcb,	0xcc,	0xcd,	0xce,	//  ha  hi  hu  he  ho
		0xd0,	0x0,	0xd0,	0x0,	0xd0,	// mya,    , myu,   , myo
		0xc1,	0xc1,	0xc1,	0x0,	0xc1,	// cha, chi, chu,   , cho
		0xb7,	0x0,	0xb7,	0x0,	0xb7,	// kya,	   , kyu,   , kyo
		0xbc,	0xbc,	0xbc,	0x0,	0xbc,	// sya,	shi, syu,   , syo
		0xd8,	0x31,	0xd8,	0x0,	0xd8,	// rya, dji, ryu,   , ryo
		0xbc,	0xbc,	0xbc,	0x0,	0xbc,	// sha,	shi, shu,   , sho
		0x0,	0x31,	0x32,	0x0,	0x0,	//    , dzi, dzu,   ,
		0xc6,	0x0,	0xc6,	0x0,	0xc6,	// nya,    , nyu,   , nyo
		0x27,	0x0,	0x27,	0x0,	0x27,	// gya,    , gyu,   , gyo
		0x45,	0x0,	0x45,	0x0,	0x45,	// pya,    , pyu,   , pyo
		0x0,	0x0,	0x0,	0x0,	0x0,
		0x3b,	0x0,	0x3b,	0x0,	0x3b,	// bya,    , byu,   , byo
		0x2c,	0x0,	0x2c,	0x0,	0x2c,	// jya,    , jyu,   , jyo
		0x0,	0x0,	0x0,	0x0,	0x0,
		0x0,	0x0,	0x0,	0x0,	0x0,
		0xcb,	0x0,	0xcb,	0x0,	0xcb	// hya,    , hyu,   , hyo
	};

	if (kana_code[hash] == 0)
		return (-1);
	**mem = kana_code[hash];
	(*mem)++;
	if (hash >= 75)
	{
		if (kana_code2[hash - 75] == 0)
			return (0);

		(*count)++;
		if (*count == 5)
			return (0);

		**mem = kana_code2[hash - 75];
		(*mem)++;
	}
	return (0);
}

extern int		get_hiragana(char **s, unsigned char **mem, int *count)
{
	int	hash;

	if ((hash = hash_phonem(s, mem, count, 0xdf)) == -1) 
		return (-1);
	if (hash != -2)
	{
		if (set_hiragana(hash, mem, count) == -1)
			return (-1);
	}
	return (0);
}

extern int		get_katakana(char **s, unsigned char **mem, int *count)
{
	int	hash;

	if ((hash = hash_phonem(s, mem, count, 0xac)) == -1) 
		return (-1);
	if (hash != -2)
	{
		if (set_katakana(hash, mem, count) == -1)
			return (-1);
	}
	return (0);
}
/*
int main(void)
{
	char *str[] = {
		"a", "i", "u", "e", "o",
		"ka", "ki", "ku", "ke", "ko",
		"kya", "kyu", "kyo",
		"ga", "gi", "gu", "ge", "go",
		"gya", "gyu", "gyo",
		"sa", "si", "su", "se", "so",
		"sha", "shi", "shu", "sho",
		"sya", "syu", "syo",
		"za", "zi", "zu", "ze", "zo",
		"ja", "ji", "ju", "jo",
		"jya", "jyu", "jyo",
		"ta", "ti", "tu", "te", "to",
		"tsu",
		"cha", "chi", "chu", "cho",
		"da", "di", "du", "de", "do",
		"dji",
		"dzi", "dzu",
		"na", "ni", "nu", "ne", "no",
		"nya", "nyu", "nyo",
		"ha", "hi", "hu", "he", "ho",
		"fu",
		"hya", "hyu", "hyo",
		"ba", "bi", "bu", "be", "bo",
		"bya", "byu", "byo",
		"pa", "pi", "pu", "pe", "po",
		"pya", "pyu", "pyo",
		"ma", "mi", "mu", "me", "mo",
		"mya", "myu", "myo",
		"ya", "yu", "yo",
		"ra", "ri", "ru", "re", "ro",
		"rya", "ryu", "ryo",
		"wa", "wo",
		"xa", "xi", "xu", "xe", "xo",
		"n", NULL, " ", "-", NULL
	};
	int				hash[155] = {-1};
	unsigned int	a = 0;

	char **ptr = str;
	while (*ptr)
	{
		hash[a] = hash_kana(*ptr);
		//printf("%s -> %d\n", *ptr, hash[a]);
		a++;
		ptr++;
	}
	collision_stat(hash, a, str);
	return (0);
}
*/