#include "../inc/symbol.hpp"

std::string SymbolTypeToString(SymbolType st)
{
  if (st == SymbolType::NOTYPE)
  {
    return "NOTYPE";
  }
  return "SECTION";
}

std::string ScopeTypeToString(ScopeType st)
{
  if (st == ScopeType::LOCAL)
  {
    return "LOCAL";
  }
  return "GLOBAL";
}