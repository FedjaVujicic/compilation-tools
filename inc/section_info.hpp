#ifndef _SECTION_INFO_HPP_
#define _SECTION_INFO_HPP_

#include <iostream>
#include <cstdint>
#include <vector>

struct SectionInfo
{
  uint16_t num;
  uint32_t address;
  uint32_t size;
  std::vector<uint16_t> data;
};

#endif