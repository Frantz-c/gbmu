"global.gs"

.global_memlock g_variables, start=0xff80, end=0xfffe
.memlock		local_var, start=0xc000, len=512

.var2			cash,	g_variables
.var2			toto,	g_variables

```````````````````````````````````````````````````````
"frantz_part.gs"

.var1			i,		local_var

ld		A, (i)

ld		B, (cash)
ld		C, (cash + 1)

dec		BC
dec		BC
inc		A
inc		A
ld		(i), A

```````````````````````````````````````````````````````
"morgans_part.gs"

.var1			level,	local_var
