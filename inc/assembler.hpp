#ifndef _ASSEMBLER_HPP_
#define _ASSEMBLER_HPP_

#include <iostream>
#include <fstream>
#include <map>
#include <cstdint>
#include "../misc/parser_data.hpp"


namespace assembler
{
  void setIOFiles(std::string inputFileName,std::string outputFileName);
  void assemble();
  void handleLineFirstPass(Line line);
};










#endif