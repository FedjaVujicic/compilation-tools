ASSEMBLER=assembler
LINKER=linker
EMULATOR=emulator

${ASSEMBLER} -o test6.o emulator-tests/test6.s
${LINKER} -hex \
  -place=my_code@0x40000000 \
  -o program.hex \
  test6.o
${EMULATOR} program.hex