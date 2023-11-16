.section code
ld $0x12345, %r2
jmp asdf
beq %r2, %r1, 0xFFF6
st %r1, 0x7234
st %sp, label

label:
halt
