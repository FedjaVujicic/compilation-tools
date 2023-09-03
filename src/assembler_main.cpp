#include <iostream>
#include <vector>
#include <fstream>
#include "../misc/parser.hpp"
#include "../inc/assembler.hpp"

extern FILE *yyin;
extern void printParsingStatus(int parseStatus);
extern void printParsingData();


int main(int argc, char **argv)
{
  FILE *fp;
  if ((argc != 4) || (std::string(argv[1]) != "-o"))
  {
    std::cout << "Invalid command." << std::endl;
    return 1;
  }

  std::string inputFileName = std::string("tests/") + argv[3];
  fp = fopen(inputFileName.c_str(), "r");
  yyin = fp;

  std::string outputFileName = std::string(argv[2]);

  int parseStatus = yyparse();
  printParsingStatus(parseStatus);

  Assembler assembler(outputFileName);  
  assembler.assemble();

  return 0;
}