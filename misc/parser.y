%{
  #include <cstdio>
  #include <iostream>
  #include <vector>
  #include "../inc/assembler.hpp"
  #include "parser_data.hpp"

  int yylex();
  void yyerror(const char *s);
  extern int yylineno;

  int currentLineNumber = -1;
  Directive currentDirective;
  Instruction currentInstruction;
  std::vector<Line> parsedLines;
  Line currentLine;
  bool stopParsing = false;

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
    currentLine.number = currentLineNumber;
    if (!stopParsing)
    {
      parsedLines.push_back(currentLine);
      assembler::handleLineFirstPass(currentLine);
    }
    resetValues();
  }

  void createInstruction() 
  {
    currentLine.type = "instruction"; 
    currentLine.instruction = currentInstruction; 
    currentLine.number = currentLineNumber;
    if (!stopParsing)
    {
      parsedLines.push_back(currentLine);
      assembler::handleLineFirstPass(currentLine);
    }
    resetValues();
  }

  int i = 0;
%}

%union {
  char* symbol;
}

%token GLOBAL EXTERN SECTION WORD SKIP END ASCII

%token HALT INT IRET CALL RET JMP BEQ BNE BGT PUSH POP XCHG ADD
%token SUB MUL DIV NOT AND OR XOR SHL SHR LD ST CSRRD CSRWR


%token<symbol> SYMBOL
%token<symbol> GPR
%token<symbol> CSR
%token<symbol> NUMBER
%token<symbol> STRING

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
| ascii
;

global:
  GLOBAL SYMBOL       { currentDirective.mnemonic = "global"; addArgument($2, "symbol"); currentLineNumber = yylineno; }
| global ',' SYMBOL   { currentDirective.mnemonic = "global"; addArgument($3, "symbol"); }
;

extern:
  EXTERN SYMBOL       { currentDirective.mnemonic = "extern"; addArgument($2, "symbol"); currentLineNumber = yylineno; }
| extern ',' SYMBOL   { currentDirective.mnemonic = "extern"; addArgument($3, "symbol"); }
;

section:
  SECTION SYMBOL       { currentDirective.mnemonic = "section"; addArgument($2, "symbol"); currentLineNumber = yylineno; }
;

word:
  WORD NUMBER       { currentDirective.mnemonic = "word"; addArgument($2, "number"); currentLineNumber = yylineno; }
| WORD SYMBOL       { currentDirective.mnemonic = "word"; addArgument($2, "symbol"); currentLineNumber = yylineno; }
| word ',' NUMBER   { currentDirective.mnemonic = "word"; addArgument($3, "number"); }
| word ',' SYMBOL   { currentDirective.mnemonic = "word"; addArgument($3, "symbol"); }
;

skip:
  SKIP NUMBER       { currentDirective.mnemonic = "skip"; addArgument($2, "number"); currentLineNumber = yylineno; }
;

end:
  END       { currentDirective.mnemonic = "end"; currentLineNumber = yylineno; }
;

ascii:
  ASCII STRING      { currentDirective.mnemonic = "ascii"; addArgument($2, "string"); currentLineNumber = yylineno; }
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
  HALT  { currentInstruction.mnemonic = "halt"; currentLineNumber = yylineno; }
;

int:
  INT  { currentInstruction.mnemonic = "int"; currentLineNumber = yylineno; }
;

iret:
  IRET  { currentInstruction.mnemonic = "iret"; currentLineNumber = yylineno; }
;

call:
  CALL operand_branch  { currentInstruction.mnemonic = "call"; currentLineNumber = yylineno; }

ret:
  RET  { currentInstruction.mnemonic = "ret"; currentLineNumber = yylineno; }
;

jmp:
  JMP operand_branch  { currentInstruction.mnemonic = "jmp"; currentLineNumber = yylineno; }
;

beq:
  BEQ instr_gpr1 ',' instr_gpr2 ',' operand_branch { currentInstruction.mnemonic = "beq"; currentLineNumber = yylineno; }
;
bne:
  BNE instr_gpr1 ',' instr_gpr2 ',' operand_branch { currentInstruction.mnemonic = "bne"; currentLineNumber = yylineno; }
;
bgt:
  BGT instr_gpr1 ',' instr_gpr2 ',' operand_branch { currentInstruction.mnemonic = "bgt"; currentLineNumber = yylineno; }
;

push:
  PUSH instr_gpr1  { currentInstruction.mnemonic = "push"; currentLineNumber = yylineno; }
;

pop:
  POP instr_gpr1  { currentInstruction.mnemonic = "pop"; currentLineNumber = yylineno; }
;

xchg:
  XCHG instr_gpr1 ',' instr_gpr2  { currentInstruction.mnemonic = "xchg"; currentLineNumber = yylineno; }
;

add:
  ADD instr_gpr1 ',' instr_gpr2   { currentInstruction.mnemonic = "add"; currentLineNumber = yylineno; }
;

sub:
  SUB instr_gpr1 ',' instr_gpr2  { currentInstruction.mnemonic = "sub"; currentLineNumber = yylineno; }
;

mul:
  MUL instr_gpr1 ',' instr_gpr2   { currentInstruction.mnemonic = "mul"; currentLineNumber = yylineno; }
;

div:
  DIV instr_gpr1 ',' instr_gpr2   { currentInstruction.mnemonic = "div"; currentLineNumber = yylineno; }
;

not:
  NOT instr_gpr1  { currentInstruction.mnemonic = "not"; currentLineNumber = yylineno; }
;

and:
  AND instr_gpr1 ',' instr_gpr2   { currentInstruction.mnemonic = "and"; currentLineNumber = yylineno; }
;

or:
  OR instr_gpr1 ',' instr_gpr2   { currentInstruction.mnemonic = "or"; currentLineNumber = yylineno; }
;

xor:
  XOR instr_gpr1 ',' instr_gpr2   { currentInstruction.mnemonic = "xor"; currentLineNumber = yylineno; }
;

shl:
  SHL instr_gpr1 ',' instr_gpr2   { currentInstruction.mnemonic = "shl"; currentLineNumber = yylineno; }
;

shr:
  SHR instr_gpr1 ',' instr_gpr2   { currentInstruction.mnemonic = "shr"; currentLineNumber = yylineno; }
;

ld:
  LD operand_ld ',' instr_gpr1  { currentInstruction.mnemonic = "ld"; currentLineNumber = yylineno; }
;

st:
  ST instr_gpr1 ',' operand_st { currentInstruction.mnemonic = "st"; currentLineNumber = yylineno; }
;

csrrd:
  CSRRD instr_csr1 ',' instr_gpr2  { currentInstruction.mnemonic = "csrrd"; currentLineNumber = yylineno; }
;

csrwr:
  CSRWR instr_gpr1 ',' instr_csr2 { currentInstruction.mnemonic = "csrwr"; currentLineNumber = yylineno; }
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

operand_ld:
  '$' SYMBOL          { currentInstruction.operand = $2; currentInstruction.operand_type = "sym"; }
| '$' NUMBER          { currentInstruction.operand = $2; currentInstruction.operand_type = "num"; }
| SYMBOL              { currentInstruction.operand = $1; currentInstruction.operand_type = "mem[sym]"; }
| NUMBER              { currentInstruction.operand = $1; currentInstruction.operand_type = "mem[num]"; }
| '[' operand_reg ']' { currentInstruction.operand_type = "mem[reg]"; }
| '[' operand_reg '+' offset ']'
;

operand_st:
 SYMBOL               { currentInstruction.operand = $1; currentInstruction.operand_type = "mem[sym]"; }
| NUMBER              { currentInstruction.operand = $1; currentInstruction.operand_type = "mem[num]"; }
| '[' operand_reg ']' { currentInstruction.operand_type = "mem[reg]"; }
| '[' operand_reg '+' offset ']'
;

operand_branch:
 SYMBOL               { currentInstruction.operand = $1; currentInstruction.operand_type = "sym"; }
| NUMBER              { currentInstruction.operand = $1; currentInstruction.operand_type = "num"; }

offset:
  NUMBER      { currentInstruction.offset = $1; currentInstruction.operand_type = "mem[reg+num]"; }

%%

 void yyerror(const char *s) 
 {
    std::cout << "Syntax error, line: " << yylineno << "\n";
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
    std::cout << "line number: " << line.number << std::endl
              << "label: " << line.label << std::endl
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

/* operand_data:
  '$' SYMBOL          { currentInstruction.operand = $2; currentInstruction.operand_type = "sym"; }
| '$' NUMBER          { currentInstruction.operand = $2; currentInstruction.operand_type = "num"; }
| SYMBOL              { currentInstruction.operand = $1; currentInstruction.operand_type = "mem[sym]"; }
| NUMBER              { currentInstruction.operand = $1; currentInstruction.operand_type = "mem[num]"; }
| operand_reg         { currentInstruction.operand_type = "reg"; }
| '[' operand_reg ']' { currentInstruction.operand_type = "mem[reg]"; }
| '[' operand_reg '+' offset ']'
; */

/* offset:
  NUMBER      { currentInstruction.offset = $1; currentInstruction.operand_type = "mem[reg+num]"; }
| SYMBOL      { currentInstruction.offset = $1; currentInstruction.operand_type = "mem[reg+sym]"; } */