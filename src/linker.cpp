#include <iomanip>
#include <fstream>
#include <vector>
#include <unordered_map>
#include "../inc/linker.hpp"
#include "../inc/symbol.hpp"
#include "../inc/relocation.hpp"

namespace linker
{
  std::vector<std::ifstream> inputFiles;
  std::ofstream outputFile;
  std::unordered_map<std::string, unsigned> placeSections;
  bool isHex = false;
  bool isRelocatable = false;

  // Za cuvanje podataka iz tabele simbola
  std::unordered_map<std::string, Symbol> symbolTable;
  // Za cuvanje podataka iz memorije, za svaku sekciju specificno
  std::unordered_map<std::string, std::vector<short>> sectionContent;
  // Za cuvanje podataka iz relokacionih tabela
  std::unordered_map<std::string, std::vector<Relocation>> relocationTable;

  void setHex()
  {
    isHex = true;
  }

  void setRelocatable()
  {
    isRelocatable = true;
  }

  void setIOFiles(std::string outputFileName, std::vector<std::string> inputFileNames)
  {
    for (const auto& inputFileName : inputFileNames)
    {
      std::ifstream inputFile;
      inputFile.open(inputFileName);
      if (!inputFile.is_open())
      {
        std::cout << "Error opening input file." << std::endl;
        exit(1);
      }
      inputFiles.push_back(std::move(inputFile));
    }
    outputFile.open(outputFileName);

    if (!outputFile.is_open())
    {
      std::cout << "Error opening output file." << std::endl;
      return;
    }

  }

  void addPlaceSection(std::string sectionName, unsigned sectionAddress)
  {
    placeSections[sectionName] = sectionAddress;
  }

  void parseObjectFile()
  {
    
  }

  void link()
  {
    parseObjectFile();
  }

}