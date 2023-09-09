#ifndef _PARSER_DATA_HPP_
#define _PARSER_DATA_HPP_

#include <iostream>
#include <vector>

struct Argument
{
  std::string type; //
  std::string value;
};

struct Directive
{
  std::string mnemonic; //
  std::vector<Argument> argList;
};

struct Instruction
{
  std::string mnemonic; //
  std::string reg1; //
  std::string reg2; //
  std::string operand;
  std::string operand_type; //
  std::string offset;
};

struct Line
{
  std::string type; //
  std::string label;
  Directive directive;
  Instruction instruction;
};


#endif