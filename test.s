# file: main.s





my_start:
    ld $0xFFFFFEFE, %sp
    ld $handler, %r1
    csrwr %r1, %handler

    int  # software interrupt

    ld $1, %r1
    push %r1
    ld $1, %r1
    push %r1
    call 0xF0000000
    st %r1, value2

    ld $2, %r1
    push %r1
    ld $1, %r1
    push %r1
    call mathAdd
    st %r1, value3

    ld $7, %r1
    push %r1
    ld $11, %r1
    push %r1
    call mathSub
    st %r1, value4

    ld $5, %r1
    push %r1
    ld $25, %r1
    push %r1
    call mathDiv
    st %r1, value5

    ld $4, %r1
    push %r1
    ld $24, %r1
    push %r1
    call mathDiv
    st %r1, value6

    ld value1, %r1
    ld value2, %r2
    ld value3, %r3
    ld value4, %r4
    ld value5, %r5
    ld value6, %r6
    ld value7, %r7

    halt
