[1mfile  : [0;33mbinary.gb[0m
[1mlength: [0;33m32768 octets[0m
[1mdisassembled size: [0;33m96[0m

0x150:  call 0x0190
0x153:  call 0x01a6
0x156:  ld   A, (0xc000)
0x159:  ld   (0xa000), A
0x15c:  ld   A, (0xc000)
0x15f:  ld   (0xa000), A
0x162:  ld   A, (0xc002)
0x165:  ld   (0xc002), A
0x168:  nop
0x169:  nop
0x16a:  nop
0x16b:  nop
0x16c:  nop
0x16d:  nop
0x16e:  nop
0x16f:  nop
0x170:  nop
0x171:  nop
0x172:  nop
0x173:  nop
0x174:  nop
0x175:  nop
0x176:  nop
0x177:  nop
0x178:  nop
0x179:  nop
0x17a:  nop
0x17b:  nop
0x17c:  nop
0x17d:  nop
0x17e:  nop
0x17f:  nop
0x180:  nop
0x181:  nop
0x182:  nop
0x183:  nop
0x184:  nop
0x185:  nop
0x186:  nop
0x187:  nop
0x188:  nop
0x189:  nop
0x18a:  nop
0x18b:  nop
0x18c:  nop
0x18d:  nop
0x18e:  nop
0x18f:  nop
0x190:  push HL
0x191:  push AF
0x192:  ld   A, (0xc000)
0x195:  ld   H, A
0x196:  ld   A, (0xc000)
0x199:  ld   L, A
0x19a:  dec  HL
0x19b:  ld   A, H
0x19c:  ld   (0xc000), A
0x19f:  ld   A, L
0x1a0:  ld   (0xc000), A
0x1a3:  pop  AF
0x1a4:  pop  HL
0x1a5:  ret
0x1a6:  push AF
0x1a7:  ld   A, (0xc002)
0x1aa:  dec  A
0x1ab:  ld   (0xc002), A
0x1ae:  pop  AF
0x1af:  ret
