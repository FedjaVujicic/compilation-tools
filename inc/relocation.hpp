#ifndef _RELOCATION_HPP_
#define _RELOCATION_HPP_

#include <iostream>
#include <cstdint>

struct Relocation
{
  uint32_t offset;
  std::string symbolName;
  uint32_t addend;
};

#endif