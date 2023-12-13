.global qwer

.section code1
.word asdf
halt
int
iret
ret
push %r1
pop %r3
xchg %r1, %r2
add %r1, %r2
sub %r1, %r2
halt
halt

.section code2
mul %r1, %r2
div %r1, %r2
not %r1
and %r1, %r2
or %r1, %r2
xor %r1, %r2
shl %r1, %r2
zxcv:
shr %r1, %r2
csrrd %handler, %r13
csrwr %r13, %cause
jmp 0x1234
beq %r3, %r5, asdf
bne %r2, %r10, qwer
bgt %r1, %r2, 4572
halt
halt

.section code3
call function
st %r1, 0x12345678
st %r1, label
st %r1, [%r2]
st %r1, [%r2 + 0x123]
ld $0x12345678, %r1
ld $label, %r1
ld 0x12345678, %r1
.word zxcv
ld label, %r1
ld [%r2], %r1
ld [%r2 + 0x123], %r1
.ascii "a"
asdf:
jmp 0xBBBB
halt
halt

.section code4
jmp qwer
qwer:
jmp 0xAAAA
jmp 0xCCCC
.word asdf
.word aezakmi
halt
halt

.section code5
halt
halt
halt
halt

function:
halt

label:
halt