#ifndef _ASSEMBLER_HPP_
#define _ASSEMBLER_HPP_

#include <iostream>
#include <fstream>
#include <map>
#include "symbol.hpp"
#include "section.hpp"
#include "../misc/parser_data.hpp"


class Assembler
{
public:
  Assembler(std::string outputFileName);

  void assemble();

private:
  std::ofstream outputFile;
  std::map<std::string, Symbol> symbolTable;
  std::map<std::string, Section> sectionTable;
  std::string currentSection = "ABS";
  unsigned locationCounter = 0;


  void addLabelSymbol(std::string symbolName);
  void addSectionSymbol(std::string symbolName);

  void handleDirectiveFirstPass(Directive directive);
  
  void firstPass();
  void secondPass();

  void outputSymbolTable();
};










#endif