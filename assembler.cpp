#include <iostream>
#include <vector>
#include "parser.hpp"
#include "global.h"

extern FILE* yyin;
extern std::vector<Line> allLines;

int main(int argc, char** argv)
{
  FILE *fp;
  std::string filename = argv[1];
  fp = fopen(filename.c_str(),"r");
  yyin = fp;

  yyparse();
  for (const auto& line : allLines) 
  {
    std::cout << "label: " << line.label << std::endl
     << "type: " << line.type << std::endl
     << "mnemonic: " << line.instruction.mnemonic << std::endl
     << "reg1: " << line.instruction.reg1 << std::endl
     << "reg2: " << line.instruction.reg2 << std::endl
     << "operand: " << line.instruction.operand << std::endl
     << "offset: " << line.instruction.offset << std::endl
     << "operand_type: " << line.instruction.operand_type << std::endl << std::endl;
  }
  return 0;
}