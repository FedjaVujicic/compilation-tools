.section my_code

my_start:
  int #

  iret #
  
  call 0xABCD
  call my_func

  ret #

  jmp 0x1111
  jmp my_func

  beq %r1, %r2, 0x2222
  beq %r2, %r3, my_func

  bne %r3, %r4, 0x3333
  bne %r4, %r5, my_func

  bgt %r5, %r6, 0x4444
  bgt %r6, %r7, my_func

  push %r8

  pop %r9

  xchg %r10, %r11

  add %r11, %r12

  sub %r12, %r13

  mul %r13, %r1

  div %r1, %r2

  not %r3

  and %r4, %r5

  or %r5, %r6

  xor %r6, %r7

  shl %r7, %r8

  shl %r5, %r2

  ld $0x12345678, %r1
  ld $my_func, %r2
  ld 0x12345678, %r3
  ld my_func, %r4
  ld [%r4], %r5
  ld [%r5 + 0x123], %r6

  st %r1, 0x12345678
  st %r2, my_func
  st %r3, [%r4]
  st %r5, [%r6 + 0x456]  

  csrrd %cause, %r1
  csrwr %r2, %handler


halt

my_func:
  ret

.end
