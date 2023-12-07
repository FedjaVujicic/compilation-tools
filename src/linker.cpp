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
  std::unordered_map<std::string, uint32_t> placeSections;
  bool isHex = false;
  bool isRelocatable = false;

  std::unordered_map<std::string, Symbol> symbolTable;
  std::unordered_map<std::string, std::vector<uint16_t>> sectionContent;
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
    for (const auto &inputFileName : inputFileNames)
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

  void addPlaceSection(std::string sectionName, uint32_t sectionAddress)
  {
    placeSections[sectionName] = sectionAddress;
  }

  void outputSymbolTable()
  {
    for (const auto &sym : symbolTable)
    {
      std::cout << "Value(" << sym.second.value << ") "
                << "Size(" << sym.second.size << ") "
                << "Type(" << SymbolTypeToString(sym.second.type) << ") "
                << "Scope(" << ScopeTypeToString(sym.second.scope) << ") "
                << "Section(" << sym.second.section << ") "
                << "Name(" << sym.first << ")" << std::endl;
    }
  }

  void outputSections()
  {
    for (const auto &sec : sectionContent)
    {
      std::cout << sec.first << std::endl;
      uint16_t i = 0;
      for (const auto &mem : sec.second)
      {
        std::cout << mem << " ";
        ++i;
        if (!(i % 8))
          std::cout << std::endl;
      }
      std::cout << std::endl;
    }
  }

  void outputRelocationTables()
  {
    for (const auto &relT : relocationTable)
    {
      std::cout << "rela." << relT.first << std::endl;
      for (const auto &rel : relT.second)
      {
        std::cout << "Offset(" << rel.offset << ") "
                  << "Symbol(" << rel.symbolName << ") "
                  << "Addend(" << rel.addend << ")" << std::endl;
      }
    }
  }

  void parseInputFiles()
  {
    for (auto &inputFile : inputFiles)
    {
      uint32_t i = 0;
      std::string currentWord = "";
      while (currentWord != "Name")
      {
        inputFile >> currentWord;
      }
      // Parse symbol table
      while (true)
      {
        uint32_t value;
        uint16_t size;
        SymbolType type;
        ScopeType scope;
        std::string section;
        std::string name;

        // value
        inputFile >> currentWord;
        if (currentWord.substr(0, 2) == "#.")
        {
          break;
        }
        value = std::stoul(currentWord, nullptr, 16);

        // size
        inputFile >> currentWord;
        size = stoi(currentWord);

        // type
        inputFile >> currentWord;
        if (currentWord == "NOTYPE")
        {
          type = SymbolType::NOTYPE;
        }
        else if (currentWord == "SECTION")
        {
          type = SymbolType::SECTION;
        }

        // scope
        inputFile >> currentWord;
        if (currentWord == "GLOBAL")
        {
          scope = ScopeType::GLOBAL;
        }
        else if (currentWord == "LOCAL")
        {
          scope = ScopeType::LOCAL;
        }

        // section
        inputFile >> currentWord;
        section = currentWord;

        // name
        inputFile >> currentWord;
        name = currentWord;

        symbolTable[name] = {value, size, type, scope, section};
      }

      // Parse memory content
      while (true)
      {
        if (currentWord.substr(0, 7) == "#.rela.")
        {
          break;
        }
        std::string sectionName = currentWord.substr(2, std::string::npos);
        std::vector<uint16_t> sectionMem;
        inputFile >> currentWord;
        while (currentWord[0] != '#')
        {
          sectionMem.push_back(stoul(currentWord, nullptr, 16));
          inputFile >> currentWord;
        }
        sectionContent[sectionName] = sectionMem;
      }

      // Parse relocation tables
      while (!inputFile.eof())
      {
        std::string sectionName = currentWord.substr(7, std::string::npos);
        inputFile >> currentWord; // Offset
        inputFile >> currentWord; // Symbol
        inputFile >> currentWord; // Addend

        while (inputFile >> currentWord)
        {
          // Read section relocation data
          if (currentWord.substr(0, 2) == "#.")
          {
            break;
          }

          uint32_t offset;
          std::string symbolName;
          uint32_t addend;

          offset = stoul(currentWord, nullptr, 16);

          inputFile >> currentWord;
          symbolName = currentWord;

          inputFile >> currentWord;
          addend = stoi(currentWord);

          relocationTable[sectionName].push_back({offset, symbolName, addend});
        }
      }
      
    }
  }

  void link()
  {
    parseInputFiles();
  }

}