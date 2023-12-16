# LD, ST, CSRRD, CSRWR, PUSH, POP
.section my_code

my_start:
#ld $0xF0000000, %r14
#ld $0x12345678, %r1
#ld $my_func, %r2
#ld my_data1, %r3
#ld [%r3], %r4
#ld [%r3 + 0x4], %r5
#csrwr %r5, %cause
#csrrd %cause, %r9
#push %r1
#pop %r13

# st
ld $my_data3, %r4
ld $0x11111111, %r1
ld $0x22222222, %r2
ld $0x33333333, %r3
st %r1, my_data3
st %r2, [%r4 + 4]
st %r3, [%r4 + 8]
ld [%r4 + 0], %r5
ld [%r4 + 4], %r6
ld [%r4 + 0x8], %r7



halt

my_data1:
.word my_data2

my_data2:
.word 0xAABBCCDD
.word 0xEEEEEEEE

my_data3:
.word 0x00000011


my_func:
#ret

.end
