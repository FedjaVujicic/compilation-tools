# xchg, add, sub, mul, div, not, and, or, xor, shl, shr

.section my_code

my_start:
ld $0xF0000000, %sp

call my_func
call incrementing_loop

halt

my_func:
ld $2, %r1
ld $1, %r2
ld $3, %r3
ld $0xFFFFFFFA, %r4

xchg %r1, %r2
add %r3, %r1
div %r2, %r1
mul %r1, %r1
sub %r3, %r1

not %r4
shl %r3, %r4
shr %r2, %r4
or %r1, %r4
and %r3, %r4
shl %r1, %r4
xor %r3, %r4
sub %r1, %r4

ret


incrementing_loop:
ret

.end
