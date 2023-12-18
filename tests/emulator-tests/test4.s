# Functions, Interrupts, branches

.section my_code

my_start1:
ld $0xF0000000, %sp
ld $handler, %r1
csrwr %r1, %handler
ld $7, %r13
csrwr %r13, %status

int

call func1
ld $0x22222222, %r3
beq %r2, %r3, func2
my_start2:
bne %r2, %r3, func3
my_start3:

ld $0x55555555, %r5

halt

handler:
ld $0x11111111, %r1
csrrd %status, %r12
csrrd %cause, %r13
iret

func1:
ld $0x22222222, %r2
ret

func2:
ld $0x33333333, %r3
bgt %r3, %r2, my_start2

func3:
ld $0x44444444, %r4
jmp my_start3

.end
