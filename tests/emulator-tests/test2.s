# Code execution
.section my_code

my_start:
  ld $0xFFFFFEFE, %sp
  ld $handler, %r1
  csrwr %r1, %handler

  int # software interrupt

  call my_func
  ld my_func_addr, %r5
  halt

my_func:
  ld $0x22222222, %r2
  ret

handler:
  ld $0x33333333, %r3
  iret

.section my_data
my_func_addr:
  .word 0x55555555

#  r0=0x00000000  r1=0x????????  r2=0x22222222  r3=0x33333333
#  r4=0x00000000  r5=0x55555555  r6=0x00000000  r7=0x00000000
#  r8=0x00000000  r9=0x00000000 r10=0x00000000 r11=0x00000000
# r12=0x00000000 r13=0x00000000 r14=0xfffffefe r15=0x40000020