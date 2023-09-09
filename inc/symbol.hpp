#ifndef _SYMBOL_HPP_
#define _SYMBOL_HPP_

#include <iostream>

enum class SymbolType
{
  NOTYPE,
  SECTION
};
enum class ScopeType
{
  LOCAL,
  GLOBAL
};

std::string SymbolTypeToString(SymbolType st);
std::string ScopeTypeToString(ScopeType st);

struct Symbol
{
  unsigned value;
  short size;
  SymbolType type;
  ScopeType scope;
  std::string section;
};

#endif