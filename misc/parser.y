%{
  #include <cstdio>
  #include <iostream>
  #include <vector>
  #include "../inc/assembler.hpp"
  #include "parser_data.hpp"

  int yylex();
  void yyerror(const char *s);

  Directive currentDirective;
  Instruction currentInstruction;
  std::vector<Line> parsedLines;
  Line currentLine;

  void resetValues()
  {
    currentDirective.mnemonic = "";
    currentDirective.argList.clear();
    currentInstruction.mnemonic = "";
    currentInstruction.reg1 = "";
    currentInstruction.reg2 = "";
    currentInstruction.operand = "";
    currentInstruction.operand_type = "";
    currentInstruction.offset = "";
    currentLine.type = "";
    currentLine.label = "";
  }

  void addArgument(std::string argument, std::string type)
  {
    currentDirective.argList.push_back({type, argument});
  }

  void createDirective()
  {
    currentLine.type = "directive";
    currentLine.directive = currentDirective;
    parsedLines.push_back(currentLine);
    resetValues();
  }

  void createInstruction() 
  {
    currentLine.type = "instruction"; 
    currentLine.instruction = currentInstruction; 
    parsedLines.push_back(currentLine);
    resetValues();
  }

  int i = 0;
%}

%union {
  char* symbol;
}

%token GLOBAL EXTERN SECTION WORD SKIP END

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
| directive                     { createDirective(); }
| label ':' directive           { createDirective(); }
;

label:
  SYMBOL      { currentLine.label = $1; }
;

directive:
  global
| extern
| section
| word
| skip
| end
;

global:
  GLOBAL SYMBOL       { currentDirective.mnemonic = "global"; addArgument($2, "symbol"); }
| global ',' SYMBOL   { currentDirective.mnemonic = "global"; addArgument($3, "symbol"); }
;

extern:
  EXTERN SYMBOL       { currentDirective.mnemonic = "extern"; addArgument($2, "symbol"); }
| extern ',' SYMBOL   { currentDirective.mnemonic = "extern"; addArgument($3, "symbol"); }
;

section:
  SECTION SYMBOL       { currentDirective.mnemonic = "section"; addArgument($2, "symbol"); }
;

word:
  WORD NUMBER       { currentDirective.mnemonic = "word"; addArgument($2, "number"); }
| WORD SYMBOL       { currentDirective.mnemonic = "word"; addArgument($2, "symbol"); }
| word ',' NUMBER   { currentDirective.mnemonic = "word"; addArgument($3, "number"); }
| word ',' SYMBOL   { currentDirective.mnemonic = "word"; addArgument($3, "symbol"); }
;

skip:
  SKIP NUMBER       { currentDirective.mnemonic = "skip"; addArgument($2, "number"); }
;

end:
  END       { currentDirective.mnemonic = "end"; }
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

// Print parsing status message
void printParsingStatus(int parseStatus)
{
  if (parseStatus == 0)
  {
    std::cout << "Parsing successful" << std::endl;
  }
  else if (parseStatus == 1)
  {
    std::cout << "Parsing failed" << std::endl;
  }
  else if (parseStatus == 2)
  {
    std::cout << "Parsing memory exhausted" << std::endl;
  }
}

// Print parsed instructions and directives, with all of their data
void printParsingData()
{
  for (const auto &line : parsedLines)
  {
    std::cout << "label: " << line.label << std::endl
              << "type: " << line.type << std::endl;

    if (line.type == "instruction")
    {
      std::cout << "mnemonic: " << line.instruction.mnemonic << std::endl
                << "reg1: " << line.instruction.reg1 << std::endl
                << "reg2: " << line.instruction.reg2 << std::endl
                << "operand: " << line.instruction.operand << std::endl
                << "offset: " << line.instruction.offset << std::endl
                << "operand_type: " << line.instruction.operand_type << std::endl
                << std::endl;
    }
    else if (line.type == "directive")
    {
      std::cout << "mnemonic: " << line.directive.mnemonic << std::endl;
      int i = 0;
      for (const auto& arg : line.directive.argList)
      {
        std::cout << "arg " << i << ": " << arg.value << " - " << arg.type << std::endl;
        ++i;
      }
      std::cout << std::endl;
    }
  }
}