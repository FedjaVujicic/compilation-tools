#include <vector>
#include <iomanip>
#include "../inc/assembler.hpp"
#include "../misc/parser.hpp"

extern std::vector<Line> parsedLines;
extern FILE *yyin;
extern void printParsingStatus(int parseStatus);
extern void printParsingData();

namespace assembler
{
  FILE *inputFile;
  std::ofstream outputFile;
  std::map<std::string, Symbol> symbolTable;
  std::map<std::string, Section> sectionTable;
  std::string currentSection = "ABS";
  unsigned locationCounter = 0;

  bool isContentOutOfSection(Line line)
  {
    if (currentSection != "ABS")
    {
      return false;
    }
    if (line.label != "")
    {
      return true;
    }
    if (line.type == "directive")
    {
      if (line.directive.mnemonic != "word" && line.directive.mnemonic != "skip" && line.directive.mnemonic != "ascii")
      {
        return false;
      }
    }

    return true;
  }

  void setIOFiles(std::string inputFileName, std::string outputFileName)
  {
    inputFile = fopen(inputFileName.c_str(), "r");
    outputFile.open(outputFileName);

    if (!outputFile.is_open())
    {
      std::cout << "Error opening output file." << std::endl;
      return;
    }
  }

  void addLabelSymbol(std::string symbolName)
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

  void addSectionSymbol(std::string symbolName)
  {
    symbolTable[symbolName] = {0, 0, SymbolType::SECTION, ScopeType::LOCAL, symbolName};
  }

  void outputByte(int byteHigh, int byteLow)
  {
    if (!(locationCounter % 8))
    {
      outputFile << std::dec << std::setw(4) << std::right << std::setfill('0') << locationCounter << ": ";
    }
    outputFile << std::setw(1) << std::left << std::setfill(' ') << std::hex << byteHigh;
    outputFile << std::setw(2) << std::left << std::setfill(' ') << std::hex << byteLow;
    locationCounter += 1;
    if (!(locationCounter % 8))
    {
      outputFile << std::endl;
    }
  }

  void outputWord(int byte4High, int byte4Low, int byte3High, int byte3Low, int byte2High, int byte2Low, int byte1High, int byte1Low)
  {
    outputByte(byte1High, byte1Low);
    outputByte(byte2High, byte2Low);
    outputByte(byte3High, byte3Low);
    outputByte(byte4High, byte4Low);
  }

  void outputInteger(int value)
  {
    int byte4High = (value & 0xF0000000) >> 28;
    int byte4Low = (value & 0x0F000000) >> 24;
    int byte3High = (value & 0x00F00000) >> 20;
    int byte3Low = (value & 0x000F0000) >> 16;
    int byte2High = (value & 0x0000F000) >> 12;
    int byte2Low = (value & 0x00000F00) >> 8;
    int byte1High = (value & 0x000000F0) >> 4;
    int byte1Low = (value & 0x0000000F);
    outputWord(byte4High, byte4Low, byte3High, byte3Low,
               byte2High, byte2Low, byte1High, byte1Low);
  }

  void outputString(std::string value)
  {
    std::vector<int> bytes;
    for (const auto& c : value)
    {
      bytes.push_back(c);
    }
    for (const auto& byte : bytes)
    {
      int byteHigh = (byte & 0x000000F0) >> 4;
      int byteLow = (byte & 0x0000000F);
      outputByte(byteHigh, byteLow);
    }
  }

  void handleDirectiveFirstPass(Directive directive)
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

  void handleLineFirstPass(Line line)
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

  int stringToInt(std::string value)
  {
    if (value.length() > 2)
    {
      if (value[0] == '0' && value[1] == 'x')
      {
        return std::stoul(value, nullptr, 16);
      }
    }
    return stoi(value);
  }

  void handleDirectiveSecondPass(Directive directive)
  {
    if (directive.mnemonic == "word")
    {
      for (const auto &arg : directive.argList)
      {
        if (arg.type == "symbol")
        {
          // ??
          continue;
        }
        if (arg.type == "number")
        {
          int value = stringToInt(arg.value);
          outputInteger(value);
        }
      }
    }
    if (directive.mnemonic == "skip")
    {
      int size = stoi(directive.argList[0].value);
      for (int i = 0; i < size; ++i)
      {
        outputByte(0, 0);
      }
    }
    if (directive.mnemonic == "ascii")
    {
      outputString(directive.argList[0].value);
    }
  }

  int getGprIndex(std::string regCode)
  {
    if (regCode == "%sp")
    {
      return 14;
    }
    if (regCode == "%pc")
    {
      return 15;
    }
    return stoi(regCode.substr(2, std::string::npos));
  }

  void handleInstructionSecondPass(Instruction instruction)
  {
    if (instruction.mnemonic == "halt")
    {
      outputWord(0, 0, 0, 0, 0, 0, 0, 0);
    }
    if (instruction.mnemonic == "int")
    {
      outputWord(0, 0, 0, 0, 0, 0, 1, 0);
    }
    // iret
    // call
    // ret
    if (instruction.mnemonic == "jmp")
    {
      
    }
    // beq
    // bne
    // bgt
    // push
    // pop
    if (instruction.mnemonic == "xchg")
    {
      int regB = getGprIndex(instruction.reg1);
      int regC = getGprIndex(instruction.reg2);
      outputWord(4, 0, 0, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "add")
    {
      int regA = getGprIndex(instruction.reg2);
      int regB = getGprIndex(instruction.reg1);
      int regC = getGprIndex(instruction.reg2);
      outputWord(5, 0, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "sub")
    {
      int regA = getGprIndex(instruction.reg2);
      int regB = getGprIndex(instruction.reg1);
      int regC = getGprIndex(instruction.reg2);
      outputWord(5, 1, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "mul")
    {
      int regA = getGprIndex(instruction.reg2);
      int regB = getGprIndex(instruction.reg1);
      int regC = getGprIndex(instruction.reg2);
      outputWord(5, 2, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "div")
    {
      int regA = getGprIndex(instruction.reg2);
      int regB = getGprIndex(instruction.reg1);
      int regC = getGprIndex(instruction.reg2);
      outputWord(5, 3, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "not")
    {
      int regA = getGprIndex(instruction.reg1);
      int regB = getGprIndex(instruction.reg1);
      int regC = 0;
      outputWord(6, 0, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "and")
    {
      int regA = getGprIndex(instruction.reg2);
      int regB = getGprIndex(instruction.reg1);
      int regC = getGprIndex(instruction.reg2);
      outputWord(6, 1, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "or")
    {
      int regA = getGprIndex(instruction.reg2);
      int regB = getGprIndex(instruction.reg1);
      int regC = getGprIndex(instruction.reg2);
      outputWord(6, 2, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "xor")
    {
      int regA = getGprIndex(instruction.reg2);
      int regB = getGprIndex(instruction.reg1);
      int regC = getGprIndex(instruction.reg2);
      outputWord(6, 3, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "shl")
    {
      int regA = getGprIndex(instruction.reg2);
      int regB = getGprIndex(instruction.reg1);
      int regC = getGprIndex(instruction.reg2);
      outputWord(7, 0, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "shr")
    {
      int regA = getGprIndex(instruction.reg2);
      int regB = getGprIndex(instruction.reg1);
      int regC = getGprIndex(instruction.reg2);
      outputWord(7, 1, regA, regB, regC, 0, 0, 0);
    }
    // ld
    // st
    // csrrd
    // csrwr
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

  void firstPass()
  {
    yyin = inputFile;
    int parseStatus = yyparse();
    printParsingStatus(parseStatus);
    locationCounter = 0;
    currentSection = "ABS";
  }

  void secondPass()
  {
    outputSymbolTable();
    for (const auto &line : parsedLines)
    {
      if (line.type == "directive")
      {
        if (line.directive.mnemonic == "section")
        {
          currentSection = line.directive.argList[0].value;
          outputFile << "#." << currentSection << std::endl;
        }
      }
      if (isContentOutOfSection(line))
      {
        std::cout << "Line " << line.number << ": Error. Content defined outside of section." << std::endl;
        exit(1);
      }
      if (line.type == "directive")
      {
        handleDirectiveSecondPass(line.directive);
      }
      else if (line.type == "instruction")
      {
        handleInstructionSecondPass(line.instruction);
      }
    }
    locationCounter = 0;
  }

  void assemble()
  {
    firstPass();
    secondPass();
  }

}