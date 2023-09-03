#include <vector>
#include "../inc/assembler.hpp"

extern std::vector<Line> parsedLines;

Assembler::Assembler(std::string outputFileName)
{
  outputFile.open(outputFileName);

  if (!outputFile.is_open()) {
    std::cout << "Error opening file." << std::endl;
    return;
  }

}

void Assembler::assemble()
{
}