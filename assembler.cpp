#include <iostream>
#include <vector>
#include "parser.hpp"
#include "global.h"

extern FILE *yyin;
extern std::vector<Line> allLines;
extern void printParsingData();
extern void printParsingStatus(int parseStatus);

int main(int argc, char **argv)
{
  FILE *fp;
  std::string filename = argv[1];
  fp = fopen(filename.c_str(), "r");
  yyin = fp;

  int parseStatus = yyparse();
  printParsingStatus(parseStatus);

  return 0;
}