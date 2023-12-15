ASSEMBLER=assembler
LINKER=linker
EMULATOR=emulator

${ASSEMBLER} -o test1.o my-tests/test1.s
${ASSEMBLER} -o test2.o my-tests/test2.s
${ASSEMBLER} -o test3.o my-tests/test3.s
${ASSEMBLER} -o test4.o my-tests/test4.s
${LINKER} -hex \
  -place=code1@0x40000000 \
  -o program.hex \
  test1.o test2.o test3.o test4.o
${EMULATOR} program.hex