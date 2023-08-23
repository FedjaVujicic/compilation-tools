%{
  #include "parser.hpp"
  #include <iostream>
%}

%option noyywrap

%%

.global                     { return GLOBAL;}
.extern                     { return EXTERN;}
.section                    { return SECTION;}
.word                       { return WORD;}
.skip                       { return SKIP;}
.end                        { return END;}
halt                        { return HALT; }
int                         { return INT; }
iret                        { return IRET; }
call                        { return CALL; }
ret                         { return RET; }
jmp                         { return JMP; }
beq                         { return BEQ; }
bne                         { return BNE; }
bgt                         { return BGT; }
push                        { return PUSH; }
pop                         { return POP; }
xchg                        { return XCHG; }
add                         { return ADD; }
sub                         { return SUB; }
mul                         { return MUL; }
div                         { return DIV; }
not                         { return NOT; }
and                         { return AND; }
or                          { return OR; }
xor                         { return XOR; }
shl                         { return SHL; }
shr                         { return SHR; }
ld                          { return LD; }
st                          { return ST; }
csrrd                       { return CSRRD; }
csrwr                       { return CSRWR; }
(%r([0-9]|1[0-5]))|%sp|%pc  { yylval.symbol = yytext; return GPR; }
%status|%handler|%cause     { yylval.symbol = yytext; return CSR; }
[a-zA-Z_][a-zA-Z0-9_]*      { yylval.symbol = yytext; return SYMBOL; }
0[xX][0-9a-fA-F]{1,8}       { yylval.symbol = yytext; return NUMBER; }
[0-9]{1,10}                 { yylval.symbol = yytext; return NUMBER; }
,                           { return ','; }
:                           { return ':'; }
\%                          { return '%'; }
\$                          { return '$'; }
\[                          { return '['; }
\]                          { return ']'; }
\+                          { return '+'; }
[ \r\t\n]
#.*\n
.                           { std::cout << "LEXING ERROR" << std::endl; }

%%
