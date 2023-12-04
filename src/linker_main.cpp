#include <vector>
#include <fstream>
#include <algorithm>
#include <string>
#include <unordered_map>
#include "../inc/linker.hpp"

void printParsedArguments(std::vector<std::string> inputFileNames, std::string outputFileName, std::unordered_map<std::string, unsigned> placeSections)
{

  std::cout << "Output File: " << outputFileName << std::endl;
  std::cout << "Input Files: ";
  for (const auto &file : inputFileNames)
  {
    std::cout << file << " ";
  }
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "Place Sections: " << std::endl;
  for (const auto &section : placeSections)
  {
    std::cout << "Section(" << section.first << ") "
              << "Address(" << section.second << ")" << std::endl;
  }
}

int main(int argc, char **argv)
{

  std::vector<std::string> inputFileNames;
  std::string outputFileName;
  std::unordered_map<std::string, unsigned> placeSections;
  bool isHex = false;
  bool isRelocatable = false;

  if (argc < 4)
  {
    std::cerr << "Error: Incorrect number of arguments." << std::endl;
    return 1;
  }

  for (int i = 1; i < argc; ++i)
  {
    std::string arg = argv[i];
    if (arg == "-o")
    {
      if (i == argc - 1)
      {
        std::cout << "Error. No output file specified." << std::endl;
        exit(1);
      }
      outputFileName = argv[i + 1];
      ++i;
    }
    else if (arg.substr(0, 7) == "-place=")
    {
      std::string::size_type pos = arg.find('@');
      // not checking for errors
      std::string sectionName = arg.substr(7, pos - 7);
      unsigned sectionAddress = std::stoul(arg.substr(pos + 1, std::string::npos), nullptr, 16);
      placeSections[sectionName] = sectionAddress;
    }
    else if (arg == "-hex")
    {
      isHex = true;
    }
    else if (arg == "-relocatable")
    {
      isRelocatable = true;
    }
    else
    {
      if (outputFileName.empty())
      {
        std::cout << "Error. No output file specified." << std::endl;
        exit(1);
      }
      if (i == argc - 1)
      {
        std::cout << "Error. No input files specified." << std::endl;
        exit(1);
      }
      for (; i < argc; ++i)
      {
        std::string arg = argv[i];
        if (*(arg.end() - 1) != '0' && *(arg.end() - 2) != '.')
        {
          std::cout << "Error. Invalid file name " + arg << std::endl;
          exit(1);
        }
        inputFileNames.push_back(arg);
      }
    }
  }
  if (outputFileName.empty())
  {
    std::cout << "Error. No output file specified." << std::endl;
    exit(1);
  }
  if (inputFileNames.size() == 0)
  {
    std::cout << "Error. No input files specified." << std::endl;
    exit(1);
  }

  return 0;
}