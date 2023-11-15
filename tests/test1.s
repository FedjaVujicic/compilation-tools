.section code
call 0x12345
call asdf
jmp 0x12345
jmp asdf
beq %r1, %r2, 0x12345
beq %r1, %r2, asdf
bne %r1, %r2, 0x12345
bne %r1, %r2, asdf
bgt %r1, %r2, 0x12345
bgt %r1, %r2, asdf