#include <vector>
#include <iomanip>
#include "../inc/assembler.hpp"


extern std::vector<Line> parsedLines;

Assembler::Assembler(std::string outputFileName)
{
  outputFile.open(outputFileName);

  if (!outputFile.is_open())
  {
    std::cout << "Error opening file." << std::endl;
    return;
  }
}

void Assembler::addLabelSymbol(std::string symbolName)
{
  symbolTable[symbolName].value = locationCounter - sectionTable[currentSection].base;
  symbolTable[symbolName].size = 0;
  symbolTable[symbolName].type = SymbolType::NOTYPE;
  if (symbolTable.find(symbolName) == symbolTable.end())
  {
    symbolTable[symbolName].scope = ScopeType::LOCAL;
  }
  symbolTable[symbolName].section = currentSection;
}

void Assembler::addSectionSymbol(std::string symbolName)
{
  symbolTable[symbolName] = {0, 0, SymbolType::SECTION, ScopeType::LOCAL, symbolName};
}

void Assembler::handleDirectiveFirstPass(Directive directive)
{
  if (directive.mnemonic == "extern")
  {
    for (const auto &arg : directive.argList)
    {
      if (arg.type == "symbol")
      {
        symbolTable[arg.value] = {0, 0, SymbolType::NOTYPE, ScopeType::GLOBAL, "UND"};
      }
    }
  }
  if (directive.mnemonic == "global")
  {
    for (const auto &arg : directive.argList)
    {
      if (arg.type == "symbol")
      {
        symbolTable[arg.value].scope = ScopeType::GLOBAL;
      }
    }
  }
  if (directive.mnemonic == "section")
  {
    addSectionSymbol(directive.argList[0].value);
    sectionTable[currentSection].length = locationCounter - sectionTable[currentSection].base;
    currentSection = directive.argList[0].value;
    sectionTable[currentSection].base = locationCounter;
  }
  if (directive.mnemonic == "word")
  {
    locationCounter += directive.argList.size() * 4;
  }
  if (directive.mnemonic == "skip")
  {
    locationCounter += stoi(directive.argList[0].value);
  }
  if (directive.mnemonic == "ascii")
  {
    locationCounter += directive.argList[0].value.length();
  }
}

void Assembler::firstPass()
{
  for (const auto &line : parsedLines)
  {
    if (line.label != "")
    {
      addLabelSymbol(line.label);
    }
    if (line.type == "directive")
    {
      handleDirectiveFirstPass(line.directive);
    }
    if (line.type == "instruction")
    {
      locationCounter += 4;
    }
  }
  locationCounter = 0;
}

void Assembler::secondPass()
{
  outputSymbolTable();
}

void Assembler::assemble()
{
  firstPass();
  secondPass();
}

void Assembler::outputSymbolTable()
{
  outputFile << "#.symtab" << std::endl;
  outputFile << std::setw(10) << std::left << std::setfill(' ') << "Value";
  outputFile << std::setw(10) << std::left << std::setfill(' ') << "Size";
  outputFile << std::setw(10) << std::left << std::setfill(' ') << "Type";
  outputFile << std::setw(10) << std::left << std::setfill(' ') << "Scope";
  outputFile << std::setw(20) << std::left << std::setfill(' ') << "Section";
  outputFile << std::setw(20) << std::left << std::setfill(' ') << "Name";
  outputFile << std::endl;
  for (const auto& symbol : symbolTable)
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