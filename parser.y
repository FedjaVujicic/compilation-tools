%{
  #include <cstdio>
  #include <iostream>
  #include <vector>
  #include "global.h"

  int yylex();
  void yyerror(const char *s);


  std::vector<Line> allLines;
  Line currentLine;
  Instruction currentInstruction;

  void resetValues()
  {
    currentInstruction.mnemonic = "";
    currentInstruction.reg1 = "";
    currentInstruction.reg2 = "";
    currentInstruction.operand = "";
    currentInstruction.operand_type = "";
    currentInstruction.offset = "";
    currentLine.type = "";
    currentLine.label = "";
    currentLine.instruction = currentInstruction;
  }

  void createInstruction() 
  {
    currentLine.type = "instruction"; 
    currentLine.instruction = currentInstruction; 
    allLines.push_back(currentLine);
    resetValues();
  }
%}

%union {
  char* symbol;
}

%token HALT INT IRET CALL RET JMP BEQ BNE BGT PUSH POP XCHG ADD
%token SUB MUL DIV NOT AND OR XOR SHL SHR LD ST CSRRD CSRWR

%token<symbol> SYMBOL
%token<symbol> GPR
%token<symbol> CSR
%token<symbol> NUMBER

%%

input:
  %empty
| input line
;

line:
  instr                         { createInstruction(); }
| label ':' instr               { createInstruction(); }
;

label:
  SYMBOL      { currentLine.label = $1; }
;


instr:
  halt
| int
| iret
| call
| ret
| jmp
| beq
| bne
| bgt
| push
| pop
| xchg
| add
| sub
| mul
| div
| not
| and
| or
| xor
| shl
| shr
| ld
| st
| csrrd
| csrwr
;

halt:
  HALT  { currentInstruction.mnemonic = "halt"; }
;

int:
  INT  { currentInstruction.mnemonic = "int"; }
;

iret:
  IRET  { currentInstruction.mnemonic = "iret"; }
;

call:
  CALL operand  { currentInstruction.mnemonic = "call"; }

ret:
  RET  { currentInstruction.mnemonic = "ret"; }
;

jmp:
  JMP operand  { currentInstruction.mnemonic = "jmp"; }
;

beq:
  BEQ instr_gpr1 ',' instr_gpr2 ',' operand { currentInstruction.mnemonic = "beq"; }
;
bne:
  BNE instr_gpr1 ',' instr_gpr2 ',' operand { currentInstruction.mnemonic = "bne"; }
;
bgt:
  BGT instr_gpr1 ',' instr_gpr2 ',' operand { currentInstruction.mnemonic = "bgt"; }
;

push:
  PUSH instr_gpr1  { currentInstruction.mnemonic = "push"; }
;

pop:
  POP instr_gpr1  { currentInstruction.mnemonic = "pop"; }
;

xchg:
  XCHG instr_gpr1 ',' instr_gpr2  { currentInstruction.mnemonic = "xchg"; }
;

add:
  ADD instr_gpr1 ',' instr_gpr2   { currentInstruction.mnemonic = "add"; }
;

sub:
  SUB instr_gpr1 ',' instr_gpr2  { currentInstruction.mnemonic = "sub"; }
;

mul:
  MUL instr_gpr1 ',' instr_gpr2   { currentInstruction.mnemonic = "mul"; }
;

div:
  DIV instr_gpr1 ',' instr_gpr2   { currentInstruction.mnemonic = "div"; }
;

not:
  NOT instr_gpr1  { currentInstruction.mnemonic = "not"; }
;

and:
  AND instr_gpr1 ',' instr_gpr2   { currentInstruction.mnemonic = "and"; }
;

or:
  OR instr_gpr1 ',' instr_gpr2   { currentInstruction.mnemonic = "or"; }
;

xor:
  XOR instr_gpr1 ',' instr_gpr2   { currentInstruction.mnemonic = "xor"; }
;

shl:
  SHL instr_gpr1 ',' instr_gpr2   { currentInstruction.mnemonic = "shl"; }
;

shr:
  SHR instr_gpr1 ',' instr_gpr2   { currentInstruction.mnemonic = "shr"; }
;

ld:
  LD operand ',' instr_gpr1  { currentInstruction.mnemonic = "ld"; }
;

st:
  ST instr_gpr1 ',' operand { currentInstruction.mnemonic = "st"; }
;

csrrd:
  CSRRD instr_csr1 ',' instr_gpr2  { currentInstruction.mnemonic = "csrrd"; }
;

csrwr:
  CSRWR instr_gpr1 ',' instr_csr2 { currentInstruction.mnemonic = "csrwr"; }
;



instr_gpr1:
  GPR         { currentInstruction.reg1 = $1; }
;

instr_gpr2:
  GPR         { currentInstruction.reg2 = $1; }
;

instr_csr1:
  CSR         { currentInstruction.reg1 = $1; }
;

instr_csr2:
  CSR         { currentInstruction.reg2 = $1; }
;

operand_reg:
  GPR         { currentInstruction.operand = $1; }
;

operand:
  '$' SYMBOL          { currentInstruction.operand = $2; currentInstruction.operand_type = "sym"; }
| '$' NUMBER          { currentInstruction.operand = $2; currentInstruction.operand_type = "num"; }
| SYMBOL              { currentInstruction.operand = $1; currentInstruction.operand_type = "mem[sym]"; }
| NUMBER              { currentInstruction.operand = $1; currentInstruction.operand_type = "mem[num]"; }
| operand_reg         { currentInstruction.operand_type = "reg"; }
| '[' operand_reg ']' { currentInstruction.operand_type = "mem[reg]"; }
| '[' operand_reg '+' offset ']'
;

offset:
  NUMBER      { currentInstruction.offset = $1; currentInstruction.operand_type = "mem[reg+num]"; }
| SYMBOL      { currentInstruction.offset = $1; currentInstruction.operand_type = "mem[reg+sym]"; }

%%

 void yyerror(const char *s) 
 {
    fprintf (stderr, "%s\n", s);
 }
