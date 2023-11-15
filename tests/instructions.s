.section code
halt
int
iret
ret
push %r1
pop %r3
xchg %r1, %r2
add %r1, %r2
sub %r1, %r2
mul %r1, %r2
div %r1, %r2
not %r1
and %r1, %r2
or %r1, %r2
xor %r1, %r2
shl %r1, %r2
shr %r1, %r2
csrrd %handler, %r13
csrwr %r13, %cause
