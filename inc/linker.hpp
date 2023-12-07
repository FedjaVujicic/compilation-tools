#ifndef _LINKER_HPP_
#define _LINKER_HPP_
#include <iostream>
#include <vector>
#include <cstdint>

namespace linker
{
  void setHex();
  void setRelocatable();
  void setIOFiles(std::string outputFileName, std::vector<std::string> inputFileNames);
  void addPlaceSection(std::string sectionName, uint32_t sectionAddress);
  void link();
};

#endif