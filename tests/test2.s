
.section program
.global a, b, c
.extern x

.section text
a: halt
b: halt
int
ret
iret
halt
halt
d: halt

jmp asdf
jmp 0x1234
jmp qwer

asdf:halt
zxcv:halt