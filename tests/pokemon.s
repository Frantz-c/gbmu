[1mfile  : [0;33m../ressources/roms/Pokemon_Rouge.gb[0m
[1mlength: [0;33m1048576 octets[0m
[1mdisassembled size: [0;33m3328[0m

rst  0x38
jr   NZ, -0x4f
inc  SP
rst  0x38
jr   NZ, -0x5f
inc  SP
rst  0x38
jr   NZ, -0x7f
inc  SP
rst  0x38
daa
add  H
scf
ld   H, -0x7c
ld   (HL), 0x25
add  E
dec  (HL)
inc  H
add  E
inc  (HL)
inc  HL
add  D
inc  SP
ld   (HL++), A
add  C
ld   (HL--), A
rst  0x38
jr   NZ, 0x51
ld   A, (HL++)
rst  0x38
ld   HL, 0x2b41
jr   NZ, 0x61
ld   A, (HL++)
rst  0x38
jr   NZ, -0x7f
stop
rst  0x38
jr   NZ, -0x7e
inc  HL
rst  0x38
jr   NZ, -0x7e
dec  H
rst  0x38
jr   NZ, -0x7e
ld   H, -0x01
jr   NZ, -0x5f
stop
rst  0x38
jr   NZ, -0x5e
ld   DE, 0x20ff
and  D
ld   D, B
rst  0x38
jr   NZ, -0x5f
jr   0x20
ld   SP, -0x00cd
ld   (HL++), A
sub  C
jr   Z, 0x20
ld   (HL), C
jr   -0x01
jr   NZ, -0x6f
ld   (HL++), A
rst  0x38
jr   NZ, 0x71
ld   (HL++), A
rst  0x38
jr   NZ, 0x61
ld   (HL++), A
rst  0x38
ld   (HL), E
ld   B, E
add  E
ld   B, E
sub  E
ld   B, E
and  E
ld   B, E
or   E
ld   B, E
jp   0xc343
ld   B, E
jp   0xc343
ld   B, E
ld   (BC), A
ld   B, (HL)
adc  D
adc  A, -0x01
cp   -0x13
call C, 0xa9cb
add  A
ld   H, L
ld   B, H
inc  SP
ld   (HL++), A
ld   DE, 0x4602
adc  D
adc  A, -0x11
rst  0x38
cp   -0x12
[1;31mILLEGAL INSTRUCTION: (0xdd)[0m
res  5, C
add  A
ld   H, L
ld   B, E
ld   (HL++), A
ld   DE, 0x6913
cp   L
xor  -0x12
rst  0x38
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xed)[0m
sbc  A, -0x01
rst  0x38
xor  -0x12
[1;31mILLEGAL INSTRUCTION: (0xdb)[0m
sub  (HL)
ld   SP, 0x4602
adc  D
call 0xfeef
sbc  A, -0x01
xor  -0x24
cp   D
sbc  B
halt
ld   D, H
ld   (HL--), A
stop
ld   BC, 0x4523
ld   H, A
adc  D
call 0xf7ee
ld   A, A
xor  -0x24
xor  B
halt
ld   D, H
ld   (HL--), A
stop
ld   HL, 0x33e2
jr   Z, -0x1f
ld   (HL++), A
rst  0x38
ld   (0x1410), A
call C, 0xe310
ld   B, C
ld   D, C
ld   (HL), E
sub  C
ld   (HL), C
ld   (0xff00+C), A
inc  BC
ld   BC, 0x0121
[1;31mILLEGAL INSTRUCTION: (0xe3)[0m
ld   (HL), C
sub  C
ld   D, C
ld   (HL), A
set  7, A
ld   (HL++), A
rst  0x30
inc  H
ld   (HL++), A
rst  0x30
inc  (HL)
inc  H
rst  0x30
ld   B, H
jr   Z, -0x0c
ld   D, L
jr   Z, -0x0f
ld   B, H
rst  0x38
add  HL, HL
pop  AF
ld   B, H
jr   Z, -0x2f
ld   B, E
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xec)[0m
ld   (BC), A
jr   Z, -0x1f
ld   B, B
rlca
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xec)[0m
ld   (BC), A
ld   (HL++), A
ld   (0x0000), SP
jr   Z, -0x4f
ld   B, C
rlca
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xec)[0m
ld   (BC), A
stop
ld   A, (HL--)
inc  H
ld   A, (0xff00+C)nop
ld   (BC), A
stop
ld   (HL++), A
jr   Z, -0x1e
nop
ld   (BC), A
stop
ld   (0xecff), SP
ld   (BC), A
inc  H
pop  AF
add  B
rlca
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xec)[0m
ld   (BC), A
ld   HL, 0x0008
nop
inc  H
and  C
ld   H, C
rlca
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xec)[0m
ld   (BC), A
inc  H
pop  HL
nop
rlca
jr   Z, -0x0e
ld   (0xff00+0x07), A
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xec)[0m
ld   (BC), A
ld   HL, 0x0008
nop
inc  H
sub  C
pop  BC
ld   B, 0x28
and  D
and  C
rlca
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xec)[0m
ld   (BC), A
stop
ld   E, D
cpl
pop  AF
nop
inc  BC
stop
ld   (0x22ff), SP
pop  AF
ld   D, H
inc  L
ld   (HL), C
inc  HL
ld   (HL++), A
or   C
ld   D, H
inc  L
ld   H, C
inc  HL
ld   H, 0x41
ld   D, H
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xec)[0m
ld   (BC), A
jr   NZ, -0x6f
ret  NZ
rlca
jr   NZ, -0x7f
ret  NC
rlca
jr   NZ, -0x6f
ret  NZ
rlca
inc  L
and  C
ret  NC
rlca
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xec)[0m
ld   (BC), A
inc  H
[1;31mILLEGAL INSTRUCTION: (0xf4)[0m
nop
rlca
ld   (HL++), A
[1;31mILLEGAL INSTRUCTION: (0xe4)[0m
nop
ld   B, 0x22
[1;31mILLEGAL INSTRUCTION: (0xe4)[0m
add  B
ld   B, 0x22
[1;31mILLEGAL INSTRUCTION: (0xe4)[0m
ret  NZ
ld   B, 0x22
[1;31mILLEGAL INSTRUCTION: (0xe4)[0m
nop
rlca
ld   (HL++), A
[1;31mILLEGAL INSTRUCTION: (0xe4)[0m
and  B
rlca
cpl
ld   A, (0xff00+C)ld   (0xff00+0x07), A
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xec)[0m
ld   (BC), A
inc  H
ld   (0x0000), SP
ld   (HL++), A
call NC, 0x0701
ld   (HL++), A
call NZ, 0x0601
ld   (HL++), A
call NZ, 0x0681
ld   (HL++), A
call NZ, 0x06c1
ld   (HL++), A
call NZ, 0x0701
ld   (HL++), A
call NZ, 0x07a1
cpl
jp   NC, 0x07e1
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xec)[0m
ld   (BC), A
stop
rla
cpl
ld   A, (0xff00+0xf0)
inc  B
cpl
ld   A, (0xff00+C)ld   D, B
ld   B, 0x10
ld   (0xecff), SP
nop
stop
inc  D
inc  H
ld   A, (0xff00+C)nop
ld   B, -0x02
inc  B
add  A, 0x44
cpl
di
nop
ld   B, 0x10
ld   (0xecff), SP
ld   (BC), A
stop
inc  D
inc  H
ld   A, (0xff00+C)nop
ld   B, 0x24
ld   A, (0xff00+C)nop
ld   B, 0x10
rla
cpl
ld   A, (0xff00+C)nop
ld   B, 0x10
ld   (0xecff), SP
ld   (BC), A
stop
dec  D
cpl
ld   A, (0xff00+0xf0)
inc  B
cpl
ld   A, (0xff00+C)ld   D, B
ld   B, 0x10
ld   (0xecff), SP
ld   (BC), A
cpl
ld   A, (0xff00+C)ret  NZ
rlca
cpl
nop
nop
nop
inc  HL
and  C
add  B
rlca
inc  HL
and  C
nop
rlca
inc  HL
and  C
ld   B, B
rlca
inc  HL
and  C
nop
rlca
inc  HL
and  C
add  B
rlca
inc  HL
and  C
nop
rlca
inc  HL
and  C
ret  NZ
rlca
jr   Z, -0x5f
nop
rlca
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xec)[0m
ld   (BC), A
inc  H
ld   A, (0xff00+0x00)
ld   B, 0x24
ld   A, (0xff00+0x00)
inc  B
inc  H
ld   A, (0xff00+0x00)
ld   (BC), A
ld   HL, 0x0000
nop
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xec)[0m
ld   (BC), A
ld   H, -0x10
nop
rlca
inc  H
nop
nop
nop
ld   H, -0x10
nop
rlca
ld   HL, 0x0000
nop
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xec)[0m
ld   BC, 0x1710
cpl
rst  0x10
nop
ld   B, 0x2f
or   A
add  B
dec  B
cpl
add  A
nop
dec  B
cpl
ld   B, A
add  B
inc  B
cpl
rla
nop
inc  B
stop
ld   (0xecff), SP
ld   (BC), A
inc  H
nop
nop
nop
ld   (HL++), A
pop  AF
add  B
ld   B, 0x21
nop
nop
nop
inc  H
pop  AF
add  B
rlca
inc  H
nop
nop
nop
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xec)[0m
ld   (BC), A
stop
inc  L
inc  H
ld   A, (0xff00+C)nop
dec  B
stop
ld   (HL++), A
ld   (HL++), A
pop  AF
nop
dec  B
stop
ld   (0x0021), SP
nop
nop
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xec)[0m
ld   BC, 0x1710
cpl
rst  0x10
nop
dec  B
cpl
or   A
add  B
dec  B
cpl
add  A
nop
ld   B, 0x2f
ld   B, A
add  B
ld   B, 0x2f
rla
nop
rlca
stop
ld   (0xecff), SP
ld   BC, 0x1710
cpl
rst  0x10
nop
rlca
cpl
or   A
add  B
ld   B, 0x2f
add  A
nop
ld   B, 0x2f
ld   B, A
add  B
dec  B
cpl
rla
nop
dec  B
stop
ld   (0xecff), SP
ld   BC, 0x1610
cpl
jp   NC, 0x0500
stop
ld   (0xecff), SP
ld   (BC), A
stop
sub  L
cpl
ld   A, (0xff00+C)nop
inc  B
stop
ld   (0x22ff), SP
pop  AF
ld   (HL--), A
ld   (HL++), A
nop
nop
ld   (HL++), A
pop  AF
ld   (HL++), A
ld   HL, 0x0000
rst  0x38
ld   (HL++), A
pop  AF
ld   (DE), A
ld   (HL++), A
nop
nop
ld   (HL++), A
and  C
ld   (DE), A
ld   (HL++), A
nop
nop
ld   (HL++), A
pop  DE
ld   (DE), A
ld   (HL++), A
nop
nop
ld   (HL++), A
add  C
ld   (DE), A
ld   (HL++), A
nop
nop
ld   (HL++), A
or   C
ld   (DE), A
ld   (HL++), A
nop
nop
ld   (HL++), A
ld   H, C
ld   (DE), A
ld   (HL++), A
nop
nop
ld   (HL++), A
sub  C
ld   (DE), A
ld   (HL++), A
nop
nop
ld   (HL++), A
ld   B, C
ld   (DE), A
ld   (HL++), A
nop
nop
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xec)[0m
inc  BC
stop
ld   E, D
inc  H
ld   A, (0xff00+0x00)
dec  B
stop
ld   (0x0024), SP
nop
nop
cpl
ld   A, (0xff00+0x00)
dec  B
ld   HL, 0x0000
nop
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xec)[0m
inc  BC
inc  H
ld   A, (0xff00+0x01)
inc  B
inc  H
nop
nop
nop
cpl
ld   A, (0xff00+0x01)
inc  B
ld   HL, 0x0000
nop
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xec)[0m
nop
stop
rla
cpl
jp   NC, 0x0700
stop
ld   (0x24ff), SP
and  D
inc  HL
jr   Z, -0x0f
inc  (HL)
cpl
nop
nop
ld   (HL++), A
rst  0x30
inc  H
ld   (HL++), A
rst  0x30
inc  (HL)
inc  H
rst  0x30
ld   B, H
jr   Z, -0x0c
ld   D, L
jr   Z, -0x0f
ld   B, H
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xec)[0m
ld   (BC), A
cpl
ld   A, (0xff00+0x00)
dec  B
inc  H
nop
nop
nop
cpl
ld   A, (0xff00+0x00)
dec  B
cpl
ld   A, (0xff00+0x00)
dec  B
cpl
ld   A, (0xff00+0x00)
dec  B
cpl
ld   A, (0xff00+0x00)
dec  B
cpl
ld   A, (0xff00+C)nop
dec  B
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xec)[0m
inc  BC
cpl
ld   A, (0xff00+0x82)
inc  B
inc  H
nop
nop
nop
cpl
ld   A, (0xff00+0x82)
inc  B
cpl
ld   A, (0xff00+0x82)
inc  B
cpl
ld   A, (0xff00+0x82)
inc  B
cpl
ld   A, (0xff00+0x82)
inc  B
cpl
ld   A, (0xff00+C)add  D
inc  B
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xec)[0m
ld   BC, 0x3a10
inc  H
ld   A, (0xff00+C)nop
dec  B
stop
ld   (HL++), A
inc  H
ld   (0xff00+C), A
nop
dec  B
stop
ld   A, (HL--)
inc  H
ld   A, (0xff00+C)nop
rlca
stop
ld   (HL++), A
cpl
ld   (0xff00+C), A
nop
rlca
stop
ld   (0xecff), SP
ld   (BC), A
cpl
di
jr   NC, 0x07
jr   Z, 0x65
jr   NC, 0x07
cpl
[1;31mILLEGAL INSTRUCTION: (0xf4)[0m
nop
rlca
cpl
ld   (HL), H
nop
rlca
cpl
ld   B, H
nop
rlca
cpl
inc  H
nop
rlca
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   A, (0xff00+0x2f)
ld   (0xff00+0x80), A
rlca
cpl
ld   A, (0xff00+0x84)
rlca
cpl
jp   0x05e0
cpl
call NZ, 0x0600
ld   A, (HL++)
ld   L, H
add  B
rlca
jr   Z, 0x71
add  H
rlca
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
dec  B
cpl
and  B
ld   B, C
rlca
cpl
or   B
ld   B, E
rlca
cpl
sub  E
or   C
dec  B
cpl
sub  H
pop  BC
dec  B
ld   A, (HL++)
ld   C, H
ld   B, C
rlca
jr   Z, 0x31
ld   B, (HL)
rlca
rst  0x38
ld   (HL++), A
ld   A, (0xff00+C)ld   C, H
ld   H, -0x20
ld   A, (HL--)
cpl
ret  NC
ld   A, (HL--)
jr   Z, -0x30
inc  L
ld   H, -0x1a
ld   C, H
inc  L
ld   A, L
ld   C, H
cpl
[1;31mILLEGAL INSTRUCTION: (0xd3)[0m
ld   C, H
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   A, (0xff00+0x2f)
rst  0x30
and  B
rlca
ld   H, -0x1a
and  E
rlca
ld   A, (HL++)
[1;31mILLEGAL INSTRUCTION: (0xf4)[0m
and  B
rlca
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
and  L
ld   A, (HL++)
or   -0x28
rlca
inc  H
[1;31mILLEGAL INSTRUCTION: (0xe3)[0m
rst  0x10
rlca
cpl
ld   A, (0xff00+C)ret  C
rlca
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
dec  B
ld   (HL++), A
ld   (0x0000), SP
cpl
and  A
and  C
ld   B, 0x26
add  (HL)
and  D
ld   B, 0x2a
ld   (HL), H
and  C
ld   B, -0x04
ld   E, A
ld   A, (HL++)
halt
sub  0x06
inc  H
add  E
reti
ld   B, 0x2f
and  D
rst  0x10
ld   B, -0x01
ld   (HL++), A
ld   A, (0xff00+C)inc  A
jr   Z, -0x1c
ld   A, 0x2f
rst  0x10
inc  A
ld   H, -0x3b
dec  SP
ld   H, -0x1c
dec  A
jr   Z, -0x4a
inc  A
ld   H, -0x2c
dec  A
jr   Z, -0x3f
dec  SP
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   A, (0xff00+0x2f)
rst  0x30
ret  NZ
rlca
ld   H, -0x1c
pop  BC
rlca
ld   A, (HL++)
or   -0x40
rlca
inc  H
[1;31mILLEGAL INSTRUCTION: (0xd3)[0m
jp   NZ, 0x2807
pop  BC
ret  NZ
rlca
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   E, A
cpl
sub  A
add  C
rlca
ld   H, -0x7c
add  B
rlca
ld   A, (HL++)
sub  (HL)
add  C
rlca
cpl
add  E
add  C
rlca
rst  0x38
inc  HL
ld   A, (0xff00+C)inc  A
dec  L
and  0x2c
cpl
rst  0x10
inc  A
jr   Z, -0x3f
inc  L
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   A, (0xff00+0x2f)
rst  0x30
add  B
ld   B, 0x2a
and  -0x7c
ld   B, 0x2f
rst  0x10
sub  B
ld   B, 0x28
push DE
sub  B
ld   B, 0x26
call NZ, 0x0688
dec  H
[1;31mILLEGAL INSTRUCTION: (0xd3)[0m
ld   (HL), B
ld   B, 0x24
[1;31mILLEGAL INSTRUCTION: (0xd3)[0m
ld   H, B
ld   B, 0x28
pop  BC
ld   B, B
ld   B, -0x01
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
dec  B
cpl
or   A
ld   B, C
ld   B, 0x2a
sub  (HL)
ld   B, D
ld   B, 0x2f
and  A
ld   D, C
ld   B, 0x28
and  L
ld   D, C
ld   B, 0x26
sub  H
ld   B, A
ld   B, 0x25
and  E
ld   SP, 0x2406
sub  E
ld   (HL++), A
ld   B, 0x28
ld   (HL), C
ld   BC, -0x00fa
cpl
[1;31mILLEGAL INSTRUCTION: (0xe4)[0m
inc  A
ld   A, (HL++)
rst  0x00
ld   C, H
ld   A, (HL++)
rst  0x00
inc  A
inc  L
or   A
ld   C, H
cpl
and  D
ld   E, H
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   A, (0xff00+0x26)
rst  0x30
and  B
rlca
jr   Z, -0x1a
and  H
rlca
inc  H
sub  -0x60
rlca
cpl
[1;31mILLEGAL INSTRUCTION: (0xd3)[0m
jr   NZ, 0x07
jr   Z, -0x3d
inc  HL
rlca
ld   (HL++), A
jp   NZ, 0x0728
jr   Z, -0x4f
jr   NC, 0x07
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   A, (BC)
inc  H
ld   (0x0000), SP
ld   H, -0x59
ld   B, C
rlca
jr   Z, -0x7a
ld   B, E
rlca
inc  H
halt
ld   B, C
rlca
dec  L
add  E
jp   NZ, 0x2706
ld   (HL), E
pop  BC
ld   B, 0x23
add  D
call Z, 0x2806
ld   (HL), C
ret  C
ld   B, -0x01
ld   (HL++), A
ld   A, (0xff00+C)ld   C, H
ld   H, -0x1a
ld   A, (HL--)
inc  H
rst  0x10
ld   A, (HL--)
ld   H, -0x2a
inc  L
jr   Z, -0x1b
inc  A
inc  L
jp   NC, 0x283d
pop  DE
inc  L
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
and  L
ld   H, -0x0c
ld   B, B
rlca
cpl
[1;31mILLEGAL INSTRUCTION: (0xe3)[0m
jr   NC, 0x07
inc  H
[1;31mILLEGAL INSTRUCTION: (0xf4)[0m
ld   B, B
rlca
dec  H
or   E
ld   C, B
rlca
jr   Z, -0x2f
ld   D, B
rlca
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   (HL), A
ld   H, -0x3d
ld   (DE), A
rlca
cpl
or   E
inc  B
rlca
inc  HL
jp   0x0712
inc  H
jp   0x0721
jr   Z, -0x4f
ld   (HL--), A
rlca
rst  0x38
jr   Z, -0x2a
inc  L
inc  L
add  A, 0x3c
ld   A, (HL++)
or   (HL)
inc  L
jr   Z, -0x6f
inc  E
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   A, (0xff00+0x24)
rst  0x30
ld   (0x2606), SP
and  0x00
ld   B, 0x26
rst  0x10
ld   A, (0xff00+0x05)
ld   H, -0x3c
ld   (0xff00+0x05), A
dec  H
[1;31mILLEGAL INSTRUCTION: (0xd3)[0m
ret  NZ
dec  B
inc  H
[1;31mILLEGAL INSTRUCTION: (0xd3)[0m
and  B
dec  B
jr   Z, -0x1f
add  B
dec  B
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   A, (BC)
inc  H
rst  0x00
inc  B
dec  B
ld   H, -0x5a
ld   (BC), A
dec  B
ld   H, -0x69
pop  AF
inc  B
inc  H
or   H
pop  HL
inc  B
dec  H
and  E
jp   NZ, 0x2404
or   E
and  E
inc  B
jr   Z, -0x3f
add  D
inc  B
rst  0x38
inc  L
[1;31mILLEGAL INSTRUCTION: (0xe4)[0m
ld   C, H
ld   A, (HL++)
rst  0x00
ld   E, H
inc  L
or   (HL)
ld   C, H
cpl
and  D
ld   E, H
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
pop  AF
inc  H
rst  0x30
ret  NZ
rlca
inc  L
and  -0x3e
rlca
ld   H, -0x4b
add  B
ld   B, 0x24
call NZ, 0x0670
inc  H
or   L
ld   H, B
ld   B, 0x28
pop  BC
ld   B, B
ld   B, -0x01
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
call Z, 0xc723
add  C
rlca
inc  L
or   (HL)
add  B
rlca
ld   H, -0x5b
ld   B, C
ld   B, 0x24
call NZ, 0x0632
ld   H, -0x4b
ld   HL, 0x2806
and  C
ld   (BC), A
ld   B, -0x01
inc  HL
[1;31mILLEGAL INSTRUCTION: (0xe4)[0m
inc  A
inc  L
sub  0x2c
inc  H
[1;31mILLEGAL INSTRUCTION: (0xe4)[0m
inc  A
jr   Z, -0x49
ld   E, H
cpl
jp   NZ, 0xff5d
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ret
jr   Z, -0x09
add  B
ld   B, 0x22
rst  0x30
ld   H, B
ld   B, 0x21
rst  0x20
ld   B, B
ld   B, 0x21
rst  0x20
jr   NZ, 0x06
cpl
pop  DE
nop
ld   B, 0x24
rst  0x00
ld   B, B
rlca
inc  H
and  A
jr   NC, 0x07
cpl
sub  C
jr   NZ, 0x07
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   A, C
ld   A, (HL++)
rst  0x20
add  D
ld   B, 0x22
rst  0x20
ld   H, D
ld   B, 0x21
rst  0x10
ld   B, D
ld   B, 0x21
rst  0x10
ld   (HL++), A
ld   B, 0x2f
pop  BC
ld   (BC), A
ld   B, 0x24
or   A
ld   B, D
rlca
ld   (HL++), A
sub  A
ld   (HL--), A
rlca
cpl
add  C
ld   (HL++), A
rlca
rst  0x38
inc  H
ld   (HL), H
ld   HL, 0x7424
stop
inc  H
ld   (HL), C
jr   NZ, -0x01
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
push AF
inc  H
di
jr   0x07
cpl
push HL
sbc  B
rlca
jr   Z, -0x6f
ld   E, B
rlca
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
and  B
dec  H
or   E
ld   (0x2f07), SP
push BC
adc  B
rlca
jr   Z, 0x71
ld   C, B
rlca
rst  0x38
inc  HL
and  C
inc  E
ld   L, -0x6c
inc  L
jr   Z, -0x7f
inc  E
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
and  L
inc  H
pop  HL
nop
rlca
inc  H
ld   A, (0xff00+C)add  B
rlca
ld   (HL++), A
sub  D
ld   B, B
rlca
jr   Z, -0x1f
nop
ld   B, -0x01
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   A, (BC)
inc  H
or   C
pop  HL
ld   B, 0x23
jp   NZ, 0x06e1
inc  HL
ld   H, D
add  C
ld   B, 0x28
or   C
pop  HL
dec  B
rst  0x38
ld   (HL++), A
ld   H, C
ld   (HL--), A
ld   (HL++), A
ld   H, C
ld   HL, 0x6128
ld   DE, -0x0301
ld   A, (0x8326)
ld   B, A
ld   (BC), A
cpl
ld   H, D
ld   H, 0x02
inc  H
ld   D, D
ld   B, L
ld   (BC), A
add  HL, HL
ld   H, E
ld   B, 0x02
cpl
add  D
dec  H
ld   (BC), A
cpl
ld   B, D
rlca
ld   (BC), A
rst  0x38
jr   Z, -0x2c
adc  H
inc  H
ld   (0xff00+C), A
sbc  H
cpl
add  A, -0x74
jr   Z, -0x1c
xor  H
cpl
rst  0x10
sbc  H
cpl
ld   A, (0xff00+C)xor  H
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   A, (0xff00+0x24)
di
ld   (0xff00+0x06), A
cpl
[1;31mILLEGAL INSTRUCTION: (0xe4)[0m
ld   B, B
ld   B, 0x28
pop  BC
jr   NZ, 0x06
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   A, (BC)
inc  HL
jp   0x0683
ld   L, -0x4c
ld   (BC), A
ld   B, 0x28
and  C
ld   BC, -0x00fa
inc  H
[1;31mILLEGAL INSTRUCTION: (0xd3)[0m
ld   E, H
cpl
and  0x4c
jr   Z, -0x4f
ld   E, H
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   A, (BC)
ld   H, -0x1e
nop
dec  B
ld   H, -0x1d
add  B
dec  B
ld   H, -0x2d
ld   (HL), B
dec  B
jr   Z, -0x5f
ld   H, B
dec  B
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
push AF
ld   H, -0x1e
add  D
inc  B
ld   H, -0x2d
ld   BC, 0x2605
or   D
ld   (0xff00+C), A
inc  B
jr   Z, -0x7f
pop  BC
inc  B
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
call Z, 0xf124
nop
rlca
inc  H
pop  HL
add  B
rlca
inc  H
pop  DE
ld   B, B
rlca
inc  H
pop  HL
ld   B, B
rlca
inc  H
pop  AF
add  B
rlca
inc  H
pop  DE
nop
rlca
inc  H
pop  AF
ld   BC, 0x2407
pop  DE
add  D
rlca
inc  H
pop  BC
ld   B, D
rlca
jr   Z, -0x4f
ld   B, C
rlca
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   B, H
inc  L
ld   (0x0000), SP
inc  H
pop  AF
ld   BC, 0x2407
pop  HL
add  D
rlca
inc  H
pop  DE
ld   B, C
rlca
inc  H
pop  HL
ld   B, C
rlca
inc  H
pop  AF
add  D
rlca
jr   Z, -0x2f
ld   BC, -0x00f9
cpl
ld   (0x2400), SP
ld   (0x2400), SP
pop  DE
ld   C, H
inc  H
or   C
inc  L
inc  H
pop  DE
inc  A
inc  H
or   C
inc  A
inc  H
pop  BC
inc  L
jr   Z, -0x5f
ld   C, H
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
call Z, 0xf528
nop
ld   B, 0x22
jp   NC, 0x0638
ld   (HL++), A
jp   NZ, 0x0630
ld   (HL++), A
jp   NZ, 0x0628
ld   (HL++), A
or   D
jr   NZ, 0x06
ld   (HL++), A
or   D
stop
ld   B, 0x22
and  D
jr   0x06
ld   (HL++), A
or   D
stop
ld   B, 0x28
pop  BC
jr   NZ, 0x06
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   B, H
inc  L
jp   0x05c0
inc  HL
or   C
ld   SP, HL
dec  B
ld   (HL++), A
and  C
pop  AF
dec  B
ld   (HL++), A
and  C
jp   (HL)
dec  B
ld   (HL++), A
sub  C
pop  HL
dec  B
ld   (HL++), A
sub  C
reti
dec  B
ld   (HL++), A
add  C
pop  DE
dec  B
ld   (HL++), A
sub  C
reti
dec  B
jr   Z, -0x6f
pop  HL
dec  B
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xec)[0m
nop
jr   Z, -0x0b
add  B
inc  B
ld   (HL++), A
pop  HL
ld   (0xff00+0x05), A
jr   Z, -0x2f
call C, 0xff05
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
and  L
daa
sub  L
ld   B, C
inc  B
ld   (HL++), A
add  C
ld   HL, 0x2805
ld   H, C
ld   A, (DE)
dec  B
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
adc  B
dec  H
ld   A, (0xff00+C)ld   D, B
ld   B, 0x29
pop  DE
ld   H, B
ld   B, 0x25
ld   (0xff00+C), A
ld   (DE), A
ld   B, 0x29
pop  BC
ld   (HL++), A
ld   B, 0x25
ld   A, (0xff00+C)stop
ld   B, 0x26
pop  DE
jr   NZ, 0x06
cp   0x02
ld   A, (HL++)
ld   C, E
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   B, B
inc  H
ld   (0x0000), SP
dec  H
ld   A, (0xff00+C)ld   D, C
ld   B, 0x29
pop  DE
ld   H, C
ld   B, 0x25
ld   (0xff00+C), A
inc  D
ld   B, 0x28
pop  BC
inc  H
ld   B, 0x25
ld   A, (0xff00+C)ld   DE, 0x2c06
pop  DE
ld   HL, 0x2506
ld   (0xff00+C), A
inc  D
ld   B, 0x28
pop  BC
inc  H
ld   B, 0x25
ld   A, (0xff00+C)ld   DE, 0x2406
pop  DE
ld   HL, -0x00fa
ld   H, -0x2e
inc  E
add  HL, HL
or   C
inc  L
jr   Z, -0x3e
inc  L
add  HL, HL
or   C
inc  A
ld   H, -0x3e
inc  L
add  HL, HL
and  D
inc  A
daa
jp   NZ, 0x252c
and  C
inc  A
add  HL, HL
jp   NZ, 0x242c
and  C
inc  A
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
and  B
inc  H
di
nop
ld   B, 0x28
push DE
ld   H, B
rlca
inc  HL
ld   (0xff00+C), A
jr   NZ, 0x07
jr   Z, -0x2f
stop
rlca
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   E, D
dec  H
or   E
pop  AF
ld   B, 0x27
push BC
ld   D, D
rlca
inc  HL
and  D
ld   DE, 0x2807
or   C
ld   BC, -0x00fa
inc  HL
and  D
inc  A
inc  L
sub  H
inc  L
inc  HL
add  D
inc  E
jr   Z, 0x71
inc  L
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   A, (0xff00+0x28)
rst  0x30
ld   (0xff00+0x06), A
ld   H, -0x1a
push HL
ld   B, 0x23
[1;31mILLEGAL INSTRUCTION: (0xf4)[0m
ld   (0xff00+0x06), A
inc  HL
or   -0x30
ld   B, 0x23
[1;31mILLEGAL INSTRUCTION: (0xe3)[0m
ret  NZ
ld   B, 0x24
ld   A, (0xff00+C)or   B
ld   B, 0x2f
and  D
ret  Z
ld   B, -0x01
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
dec  B
inc  HL
ld   (0x0000), SP
jr   Z, -0x59
and  C
ld   B, 0x26
add  (HL)
and  E
ld   B, 0x23
ld   (HL), H
and  C
ld   B, 0x23
halt
sub  C
ld   B, 0x23
add  E
add  D
ld   B, 0x24
and  D
ld   (HL), C
ld   B, 0x2f
ld   (HL), D
adc  C
ld   B, -0x01
ld   (HL++), A
ld   A, (0xff00+C)inc  A
jr   Z, -0x1c
ld   A, 0x28
rst  0x10
inc  A
dec  H
push BC
dec  SP
inc  HL
call NC, 0x222c
or   (HL)
inc  A
inc  HL
and  H
inc  L
jr   Z, -0x6f
inc  A
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   A, (0xff00+0x2f)
or   0x65
dec  B
ld   A, (HL++)
[1;31mILLEGAL INSTRUCTION: (0xe4)[0m
ld   A, H
dec  B
inc  HL
jp   NZ, 0x055c
cpl
or   D
inc  A
dec  B
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   E, D
ld   L, -0x2a
inc  BC
dec  B
add  HL, HL
or   H
dec  DE
dec  B
inc  H
sub  D
ld   A, (0x2f04)
and  D
[1;31mILLEGAL INSTRUCTION: (0xdb)[0m
inc  B
rst  0x38
inc  L
and  0x4c
dec  HL
rst  0x10
ld   E, H
cpl
jp   NZ, 0xff4c
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   A, (0xff00+0x24)
rst  0x30
and  B
ld   B, 0x28
and  -0x5c
ld   B, 0x24
sub  -0x60
ld   B, 0x2c
[1;31mILLEGAL INSTRUCTION: (0xd3)[0m
jr   NZ, 0x06
jr   Z, -0x3d
inc  H
ld   B, 0x24
jp   NZ, 0x0620
jr   Z, -0x4f
stop
ld   B, -0x01
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   E, D
inc  H
rst  0x20
ld   BC, 0x2806
sub  0x03
ld   B, 0x24
add  A, 0x01
ld   B, 0x2c
jp   0x0581
jr   Z, -0x4d
add  E
dec  B
inc  H
or   D
add  D
dec  B
jr   Z, -0x5f
ld   (HL), C
dec  B
rst  0x38
daa
sub  0x5c
jr   Z, -0x1a
ld   C, H
inc  H
call NC, 0x245c
call NC, 0x274c
jp   0x284c
and  C
ld   E, H
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
dec  DE
daa
jp   NC, 0x0740
cpl
push HL
ld   H, B
rlca
cpl
pop  BC
jr   NC, 0x07
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
add  C
ld   (HL++), A
jp   NZ, 0x0701
inc  H
jp   NZ, 0x0708
cpl
rst  0x10
ld   B, C
rlca
cpl
and  D
ld   BC, -0x00f9
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   A, (0xff00+0x2f)
rst  0x10
add  B
rlca
inc  H
and  -0x60
rlca
cpl
jp   NC, 0x0740
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   E, D
cpl
rst  0x00
ld   D, E
rlca
dec  H
or   (HL)
ld   (HL), D
rlca
cpl
jp   NZ, 0x0711
rst  0x38
dec  L
or   0x4c
inc  H
and  0x3c
cpl
ld   A, (0xff00+C)ld   C, H
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   A, (0xff00+0x26)
rst  0x30
ret  NZ
ld   B, 0x2f
rst  0x20
nop
rlca
inc  H
[1;31mILLEGAL INSTRUCTION: (0xf4)[0m
ld   A, (0xff00+0x06)
inc  H
[1;31mILLEGAL INSTRUCTION: (0xe4)[0m
ld   (0xff00+0x06), A
jr   Z, -0x2f
ret  NC
ld   B, -0x01
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   A, (BC)
daa
and  -0x7f
ld   B, 0x2e
push DE
pop  BC
ld   B, 0x24
call NZ, 0x06b1
inc  H
call NC, 0x06a1
jr   Z, -0x3f
sub  C
ld   B, -0x01
ld   A, (HL++)
and  (HL)
inc  A
ld   L, -0x6c
inc  L
dec  H
and  E
inc  A
jr   Z, -0x6f
inc  L
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
and  L
inc  L
ld   A, (0xff00+C)ld   B, B
inc  B
cpl
[1;31mILLEGAL INSTRUCTION: (0xe3)[0m
and  B
inc  B
inc  H
jp   NC, 0x0490
jr   Z, -0x2f
add  B
inc  B
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
xor  0x2b
jp   NC, 0x0438
ld   L, -0x3a
sbc  B
inc  B
inc  HL
or   D
adc  B
inc  B
jr   Z, -0x4f
ld   A, B
inc  B
rst  0x38
ld   A, (HL++)
and  0x6c
cpl
jp   NC, 0x235c
jp   NZ, 0x286c
pop  DE
ld   E, H
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
inc  SP
cpl
or   -0x40
dec  B
jr   Z, -0x1d
cp   H
dec  B
ld   H, -0x2e
ret  NC
dec  B
ld   H, -0x4e
ld   (0xff00+0x05), A
ld   H, -0x3e
ld   A, (0xff00+0x05)
jr   Z, -0x4f
nop
ld   B, -0x01
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
sbc  C
ld   L, -0x3a
or   C
inc  B
daa
jp   0x04ad
dec  H
or   D
pop  BC
inc  B
jr   Z, -0x6e
pop  DE
inc  B
ld   H, -0x5e
pop  HL
inc  B
jr   Z, -0x6f
pop  AF
inc  B
rst  0x38
ld   A, (HL++)
and  0x5c
ld   A, (HL++)
sub  0x6c
inc  H
jp   NZ, 0x264c
[1;31mILLEGAL INSTRUCTION: (0xd3)[0m
ld   E, H
jr   Z, -0x4d
ld   C, H
jr   Z, -0x5f
ld   E, H
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   A, (0xff00+0x28)
[1;31mILLEGAL INSTRUCTION: (0xe4)[0m
sub  B
rlca
cpl
push AF
ret  NZ
rlca
jr   Z, -0x2f
ret  C
rlca
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
and  L
ld   A, (HL++)
call NZ, 0x0771
cpl
or   (HL)
and  D
rlca
jr   Z, -0x5f
or   A
rlca
rst  0x38
jr   Z, -0x1c
ld   C, H
ld   L, -0x3c
inc  A
jr   Z, -0x2f
inc  L
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   A, (0xff00+0x26)
ld   A, (0xff00+C)nop
ld   B, 0x26
ld   (0xff00+C), A
ld   B, B
ld   B, 0x26
jp   NC, 0x0680
ld   H, -0x1e
ret  NZ
ld   B, 0x26
jp   NC, 0x0700
ld   H, -0x3e
ld   B, B
rlca
ld   H, -0x4e
add  B
rlca
jr   Z, -0x5f
ret  NZ
rlca
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   DE, 0x0823
ld   BC, 0x2600
jp   NZ, 0x05c1
ld   H, -0x4e
ld   (BC), A
ld   B, 0x26
and  D
ld   B, C
ld   B, 0x26
or   D
add  D
ld   B, 0x26
and  D
jp   NZ, 0x2606
sub  D
ld   BC, 0x2607
and  D
ld   B, D
rlca
jr   Z, -0x7f
add  C
rlca
rst  0x38
ld   H, 0x08
ld   BC, -0x1ddb
ld   E, H
dec  H
jp   NZ, 0x254c
jp   NC, 0x253c
or   D
inc  L
dec  H
jp   NZ, 0x251c
and  D
dec  DE
dec  H
sub  D
ld   A, (DE)
jr   Z, -0x7f
jr   -0x01
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   A, (0xff00+0x24)
di
add  B
rlca
cpl
rst  0x20
nop
rlca
jr   Z, -0x2d
stop
rlca
inc  H
jp   NZ, 0x0700
inc  H
jp   NC, 0x06f0
jr   Z, -0x3f
ld   (0xff00+0x06), A
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   E, D
ld   H, -0x3d
ld   BC, 0x2e07
or   A
add  C
ld   B, 0x27
or   E
sub  D
ld   B, 0x23
and  D
add  C
ld   B, 0x24
or   D
ld   (HL), D
ld   B, 0x28
and  C
ld   H, C
ld   B, -0x01
ld   H, -0x1d
ld   E, H
ld   L, -0x2a
ld   C, H
ld   H, -0x3a
inc  A
inc  HL
or   E
ld   C, H
inc  HL
and  D
ld   E, H
jr   Z, -0x4f
ld   L, H
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
rrca
cpl
rst  0x30
nop
dec  B
cpl
rst  0x20
ld   (0x2805), SP
or   H
add  B
inc  B
cpl
and  D
ld   H, B
inc  B
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   B, H
ld   L, -0x29
add  C
inc  B
ld   L, -0x39
adc  C
inc  B
ld   A, (HL++)
or   H
ld   BC, 0x2f04
jp   NZ, 0x03e1
rst  0x38
ld   L, -0x09
ld   A, H
inc  L
or   0x6c
add  HL, HL
[1;31mILLEGAL INSTRUCTION: (0xe4)[0m
ld   A, H
cpl
ld   (0xff00+C), A
ld   L, H
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
push AF
daa
sub  -0x1f
rlca
ld   H, -0x3a
ld   (0xff00+C), A
rlca
add  HL, HL
sub  -0x1f
rlca
daa
add  A, -0x20
rlca
dec  H
or   (HL)
ld   (0xff00+C), A
rlca
daa
add  A, -0x1f
rlca
ld   H, -0x4a
ld   (0xff00+0x07), A
jr   Z, -0x5f
rst  0x18
rlca
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   B, H
ld   H, -0x3d
ret
rlca
ld   H, -0x4d
rst  0x00
rlca
ld   A, (HL++)
call NZ, 0x07c3
jr   Z, -0x4c
rst  0x00
rlca
ld   H, -0x3d
ret
rlca
cpl
and  D
push BC
rlca
rst  0x38
dec  L
add  HL, DE
ld   A, H
dec  L
rst  0x30
adc  H
inc  L
sub  0x7c
jr   Z, -0x3c
ld   L, H
cpl
or   E
ld   E, H
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   A, (0xff00+0x26)
rst  0x30
ld   B, B
rlca
inc  L
and  0x44
rlca
ld   H, -0x2b
ld   D, B
rlca
inc  H
jp   0x0760
inc  HL
jp   0x0780
jr   Z, -0x2f
and  B
rlca
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   A, (BC)
ld   H, -0x39
ld   BC, 0x2b07
or   (HL)
ld   (BC), A
rlca
ld   H, -0x5b
ld   DE, 0x2407
sub  E
ld   HL, 0x2307
and  E
ld   B, C
rlca
jr   Z, -0x6f
ld   H, D
rlca
rst  0x38
inc  HL
ld   (0xff00+C), A
inc  A
jr   Z, -0x2a
ld   C, H
dec  H
call NC, 0x2c3c
rst  0x00
ld   C, H
ld   (HL++), A
ld   (0xff00+C), A
inc  A
jr   Z, -0x2f
inc  L
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
[1;31mILLEGAL INSTRUCTION: (0xf4)[0m
cpl
ld   A, (0xff00+0x05)
rlca
ld   A, (HL++)
ld   (0xff00+0x00), A
rlca
ld   H, -0x4c
stop
rlca
inc  H
[1;31mILLEGAL INSTRUCTION: (0xd3)[0m
nop
rlca
ld   H, -0x4e
jr   NZ, 0x06
jr   Z, -0x5f
inc  H
ld   B, -0x01
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   (HL++), A
cpl
or   B
jp   0x2a06
and  B
pop  BC
ld   B, 0x26
add  H
jp   NC, 0x2406
sub  E
pop  BC
ld   B, 0x26
add  D
pop  HL
dec  B
jr   Z, 0x61
add  SP, 0x05
rst  0x38
ld   H, -0x1a
ld   C, H
cpl
sub  0x3c
ld   A, (HL++)
push BC
ld   C, D
ld   HL, 0x5bb2
cpl
jp   NZ, 0xff4c
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
ld   D, B
ld   A, (HL++)
push AF
add  B
ld   B, 0x23
ld   (0xff00+C), A
and  B
ld   B, 0x23
ld   A, (0xff00+C)ret  NZ
ld   B, 0x23
ld   (0xff00+C), A
ld   (0xff00+0x06), A
inc  HL
jp   NC, 0x0700
inc  HL
jp   NZ, 0x06e0
inc  HL
jp   NC, 0x06c0
jr   Z, -0x3f
and  B
ld   B, -0x01
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
rrca
add  HL, HL
push DE
ld   SP, 0x2306
jp   NC, 0x0652
inc  HL
ld   (0xff00+C), A
ld   (HL), C
ld   B, 0x23
or   D
sub  C
ld   B, 0x23
jp   NZ, 0x06b2
inc  HL
or   D
sub  C
ld   B, 0x23
jp   NZ, 0x0671
jr   Z, -0x4f
ld   D, C
ld   B, -0x01
ld   H, -0x1d
ld   C, H
inc  H
jp   0x253c
call NC, 0x243c
call NZ, 0x262c
or   H
inc  A
jr   Z, -0x3f
inc  L
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
and  L
inc  HL
[1;31mILLEGAL INSTRUCTION: (0xf4)[0m
ld   B, C
ld   B, 0x2d
sub  0x21
rlca
jr   Z, -0x0c
add  HL, DE
rlca
jr   Z, -0x3f
ld   A, (DE)
rlca
rst  0x38
[1;31mILLEGAL INSTRUCTION: (0xfc)[0m
call Z, 0xf424
add  B
dec  B

