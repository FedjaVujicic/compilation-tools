#ifndef _SYMBOL_HPP_
#define _SYMBOL_HPP_

#include <iostream>
#include <cstdint>

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
  uint32_t value;
  uint16_t size;
  SymbolType type;
  ScopeType scope;
  std::string section;
};

#endif