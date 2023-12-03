#ifndef _RELOCATION_HPP_
#define _RELOCATION_HPP_

struct Relocation
{
  unsigned offset;
  std::string symbolName;
  unsigned addend;
};

#endif