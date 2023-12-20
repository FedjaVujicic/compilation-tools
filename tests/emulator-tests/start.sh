ASSEMBLER=assembler
LINKER=linker
EMULATOR=emulator

${ASSEMBLER} -o test4.o emulator-tests/test4.s
${LINKER} -hex \
  -place=my_code@0x40000000 \
  -o program.hex \
  test4.o
${EMULATOR} program.hex