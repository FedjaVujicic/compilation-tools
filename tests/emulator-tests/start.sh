ASSEMBLER=assembler
LINKER=linker
EMULATOR=emulator

${ASSEMBLER} -o test3.o emulator-tests/test3.s
${LINKER} -hex \
  -place=my_code@0x40000000 \
  -o program.hex \
  test3.o
${EMULATOR} program.hex