ASSEMBLER=assembler
LINKER=linker
EMULATOR=emulator

${ASSEMBLER} -o test1.o emulator-tests/test1.s
${LINKER} -hex \
  -place=my_code@0x40000000 \
  -o program.hex \
  test1.o
${EMULATOR} program.hex