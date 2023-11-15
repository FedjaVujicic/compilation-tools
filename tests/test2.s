
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

.word asdf, qwer, zxcv




asdf:halt
zxcv:halt