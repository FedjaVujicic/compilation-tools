#ifndef _ASSEMBLER_HPP_
#define _ASSEMBLER_HPP_

#include <iostream>
#include <fstream>
#include <map>
#include "symbol.hpp"
#include "section.hpp"
#include "../misc/parser_data.hpp"


namespace assembler
{
  extern FILE* inputFile;
  extern std::ofstream outputFile;
  extern std::map<std::string, Symbol> symbolTable;
  extern std::map<std::string, Section> sectionTable;
  extern std::string currentSection;
  extern unsigned locationCounter;

  void setIOFiles(std::string inputFileName,std::string outputFileName);
  void assemble();
  void handleLineFirstPass(Line line);
  void addLabelSymbol(std::string symbolName);
  void addSectionSymbol(std::string symbolName);
  void handleDirectiveFirstPass(Directive directive);  
  void firstPass();
  void secondPass();
  void outputSymbolTable();
};










#endif