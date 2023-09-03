#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <iostream>
#include <vector>
#include <fstream>

struct Argument {
  std::string type;
  std::string value;
};

struct Directive {
  std::string mnemonic;
  std::vector<Argument> argList;
};

struct Instruction {
  std::string mnemonic;
  std::string reg1;
  std::string reg2;
  std::string operand;
  std::string operand_type;
  std::string offset;
};

struct Line {
  std::string type;
  std::string label;
  Directive directive;
  Instruction instruction;
};

class Assembler
{
public:
  Assembler(std::string outputFileName);

  void assemble();

private:
  std::ofstream outputFile;
};










#endif