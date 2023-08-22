#include <iostream>

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
    Instruction instruction;
  };