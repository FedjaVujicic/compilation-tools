#include "../inc/assembler.hpp"
#include <map>
#include <iomanip>

namespace emulator
{
  std::ifstream inputFile;
  std::map<uint32_t, uint16_t> mem;

  void setInputFile(std::string inputFileName)
  {
    inputFile.open(inputFileName);

    if (!inputFile.is_open())
    {
      std::cout << "Error opening input file." << std::endl;
      exit(1);
    }
  }

  void parseInput()
  {
    std::string currentWord;
    uint32_t addr;
    while (inputFile >> currentWord)
    {
      if (currentWord.back() == ':')
      {
        currentWord.pop_back();
        addr = std::stoul(currentWord, nullptr, 16);
        continue;
      }
      uint16_t data = std::stoul(currentWord, nullptr, 16);
      mem[addr++] = data;
    }
  }

  void printMemoryContent()
  {
    uint32_t cnt = 0;
    for (const auto &memLoc : mem)
    {
      uint32_t addr = memLoc.first;
      uint16_t byte = memLoc.second;

      if (!(cnt % 8))
      {
        std::cout << std::endl
                  << std::hex << addr << ": ";
      }

      std::cout << std::hex << std::setw(2) << std::setfill('0') << byte << " ";

      if (!mem.count(addr + 1))
      {
        cnt = 0;
        continue;
      }
      cnt++;
    }
    std::cout << std::endl;
  }

  void emulate()
  {
    parseInput();
    // printMemoryContent();
  }
};