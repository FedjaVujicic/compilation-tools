# Compilation Tools

Tools for executing simple assembly code for a GNU. This project has been done as part of a course at ETF 2023.

## Features

* Two-Pass Assembler
* Linker
* Emulator

## Supports

* Load/Store Architecture
* Jumps
* Arithmethic and Logical instructions
* Software Interrupts
* Subroutines

## Installation

Setting up the project:

```git
  git clone --recursive https://github.com/FedjaVujicic/compilation-tools.git
```

In the root directory:

```
  make all

  ./assembler -o output.o input.s
  ./linker -o program.hex -place=<section>@<address> -hex input1.o input2.o ...
  ./emulator program.hex
```

