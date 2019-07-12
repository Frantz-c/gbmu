%define	copy_string(var, data, label)	\
	ld	HL, var			\
	ld	BC, data		\
	label:				\
		ld	A, (BC)		\
		ld	(HL++), A	\
		inc	BC			\
		cmp	0x50		\
		jrnz	label


.memblock	local_var, 0xc000, end=0xd000

.var12	username, local_var

.bank	0, 0x300

push	HL
push	BC
push	A
copy_string(username, __default_string, __loopx)
pop		A
pop		BC
pop		HL

%undef copy_string

__default_string:	.byte	0x80, 0x81, 0x82, 0x83, 0x84, 0x50
