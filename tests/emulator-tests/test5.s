# Nested branches

.section my_code

my_start1:
ld $0xF0000000, %sp
ld $handler, %r1
csrwr %r1, %handler

int

halt

handler:
call func1_start
iret

func1_start:
jmp func2_start
func1_end:
ld $0x11111111, %r1
ret

func2_start:
ld $0x22222222, %r2
call func3
bne %r2, %r3, func1_end

func3:
ld $0x33333333, %r3
ret


.end
