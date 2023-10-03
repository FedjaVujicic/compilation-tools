#include <iostream>
#include <vector>
#include <fstream>
#include "../misc/parser.hpp"
#include "../inc/assembler.hpp"


int main(int argc, char **argv)
{
  if ((argc != 4) || (std::string(argv[1]) != "-o"))
  {
    std::cout << "Invalid command." << std::endl;
    return 1;
  }

  std::string inputFileName = std::string("tests/") + argv[3];
  std::string outputFileName = std::string(argv[2]);

  assembler::setIOFiles(inputFileName, outputFileName);  
  assembler::assemble();

  return 0;
}