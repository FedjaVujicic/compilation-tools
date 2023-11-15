.section program
add %r1, %r2
.word 0x123, 15, 0x1234F, 68
.skip 1
.skip 4
.skip 8
.ascii "abcde\n"
a:
add %r1, %r2
