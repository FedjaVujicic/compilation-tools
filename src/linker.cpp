#include <iomanip>
#include <fstream>
#include <vector>
#include <unordered_map>
#include "../inc/linker.hpp"
#include "../inc/symbol.hpp"
#include "../inc/relocation.hpp"
#include "../inc/section_info.hpp"

namespace linker
{
  std::vector<std::ifstream> inputFiles;
  std::ofstream outputFile;
  std::unordered_map<std::string, uint32_t> placeSections;
  bool isHex = false;
  bool isRelocatable = false;

  std::unordered_map<std::string, Symbol> symbolTable;
  std::unordered_map<std::string, std::vector<Relocation>> relocationTables;

  std::unordered_map<std::string, SectionInfo> sections;
  // For preserving the order in which the sections were parsed
  std::vector<std::string> parsedSections;

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
    outputFile << "#.symtab" << std::endl;
    outputFile << std::setw(10) << std::left << std::setfill(' ') << "Value";
    outputFile << std::setw(10) << std::left << std::setfill(' ') << "Size";
    outputFile << std::setw(10) << std::left << std::setfill(' ') << "Type";
    outputFile << std::setw(10) << std::left << std::setfill(' ') << "Scope";
    outputFile << std::setw(20) << std::left << std::setfill(' ') << "Section";
    outputFile << std::setw(20) << std::left << std::setfill(' ') << "Name";
    outputFile << std::endl;
    for (const auto &symbol : symbolTable)
    {
      outputFile << std::setw(8) << std::right << std::setfill('0') << std::hex << symbol.second.value << "  ";
      outputFile << std::setw(10) << std::left << std::setfill(' ') << symbol.second.size;
      outputFile << std::setw(10) << std::left << std::setfill(' ') << SymbolTypeToString(symbol.second.type);
      outputFile << std::setw(10) << std::left << std::setfill(' ') << ScopeTypeToString(symbol.second.scope);
      outputFile << std::setw(20) << std::left << std::setfill(' ') << symbol.second.section;
      outputFile << std::setw(20) << std::left << std::setfill(' ') << symbol.first;
      outputFile << std::endl;
    }
  }

  void printSections()
  {
    for (const auto &sec : sections)
    {
      std::cout << "section(" << sec.first << ") ";
      std::cout << "address(" << std::hex << sec.second.address << ") ";
      std::cout << "size(" << std::dec << sec.second.size << ")" << std::endl;
      uint16_t i = 0;
      for (const auto &mem : sec.second.data)
      {
        std::cout << mem << " ";
        ++i;
        if (!(i % 8))
          std::cout << std::endl;
      }
      std::cout << std::endl;
    }
  }

  void printRelocationTables()
  {
    for (const auto &relT : relocationTables)
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

  void addSymbol(uint32_t value, uint16_t size, SymbolType type, ScopeType scope, std::string section, std::string name)
  {
    if (symbolTable.count(name) == 0)
    {
      symbolTable[name] = {value, size, type, scope, section};
      return;
    }
    if (type == SymbolType::SECTION)
    {
      return;
    }
    if (symbolTable[name].section != "UND" && section != "UND")
    {
      std::cout << "Linker error. Symbol " << name << " redefinition." << std::endl;
      exit(1);
    }
    if (scope == ScopeType::LOCAL || symbolTable[name].scope == ScopeType::LOCAL)
    {
      std::cout << "Linker error. Unresolved reference to " << name << std::endl;
      exit(1);
    }
    if (symbolTable[name].section == "UND" && section != "UND")
    {
      symbolTable[name] = {value, size, type, scope, section};
    }
  }

  void parseInputFiles()
  {
    uint16_t sectionNum = 0;

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

        // Section merging update
        if (sections.count(section) > 0 && type != SymbolType::SECTION)
        {
          if (section == "UND")
          {
            std::cout << "Sections entry created for UND" << std::endl;
            exit(1);
          }
          value += sections[section].size;
        }

        // name
        inputFile >> currentWord;
        name = currentWord;

        addSymbol(value, size, type, scope, section, name);
      }

      // Parse memory content
      while (true)
      {
        if (currentWord.substr(0, 7) == "#.rela.")
        {
          break;
        }
        std::string sectionName = currentWord.substr(2, std::string::npos);

        if (!sections.count(sectionName))
        {
          parsedSections.push_back(sectionName);
        }

        inputFile >> currentWord;
        while (currentWord[0] != '#')
        {
          sections[sectionName].data.push_back(stoul(currentWord, nullptr, 16));
          inputFile >> currentWord;
        }
      }

      // Parse relocation tables
      while (!inputFile.eof())
      {
        std::string sectionName = currentWord.substr(7, std::string::npos);
        inputFile >> currentWord; // Offset
        inputFile >> currentWord; // Symbol
        inputFile >> currentWord; // Addend

        uint32_t base = 0;

        // Section merging update
        if (relocationTables.count(sectionName))
        {
          base += sections[sectionName].size;
        }
        sections[sectionName].size = sections[sectionName].data.size();

        // Create a relocation tables entry in case it's empty
        relocationTables[sectionName];

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
          offset += base;

          inputFile >> currentWord;
          symbolName = currentWord;

          inputFile >> currentWord;
          addend = stoi(currentWord);
          if (symbolTable[symbolName].scope == ScopeType::LOCAL)
          {
            addend += base;
          }

          relocationTables[sectionName].push_back({offset, symbolName, addend});
        }
      }
    }
  }

  void mapSections()
  {
    uint32_t defaultAddress = 0;

    // Placed sections
    for (const auto &section : placeSections)
    {
      for (const auto& refSection : placeSections)
      {
        if (section.first == refSection.first)
        {
          continue;
        }
        if ((section.second <= refSection.second) && ((section.second + sections[section.first].size) > refSection.second))
        {
          std::cout << "Linker Error. Section collision." << std::endl;
          exit(1);
        }
      }
      sections[section.first].address = section.second;
      
      if (section.second + sections[section.first].size > defaultAddress)
      {
        defaultAddress = section.second + sections[section.first].size;
      }
    }

    // Unplaced sections
    for (const auto& sectionName : parsedSections)
    {
      if (placeSections.count(sectionName))
      {
        continue;
      }
      sections[sectionName].address = defaultAddress;
      defaultAddress += sections[sectionName].size;
    }

    if (defaultAddress > 0xFFFFFF00)
    {
      std::cout << "Linker Error. Section collision with memory mapped registers." << std::endl;
      exit(1);
    }
  }

  void link()
  {
    parseInputFiles();
    mapSections();
    outputSymbolTable();
    // printSections();
    // printRelocationTables();
  }

}