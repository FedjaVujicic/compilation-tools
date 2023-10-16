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
      int value = stoi(directive.argList[0].value);
      while (value % 4)
      {
        ++value;
      }
      locationCounter += value;
    }
    if (directive.mnemonic == "ascii")
    {
      int value = directive.argList[0].value.length();
      while (value % 4)
      {
        ++value;
      }
      locationCounter += value;
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

  void handleDirectiveSecondPass(Directive directive)
  {
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

  void outputInstruction(int byte4High, int byte4Low, int byte3High, int byte3Low, int byte2High, int byte2Low, int byte1High, int byte1Low)
  {
    outputFile << std::setw(1) << std::left << std::setfill(' ') << std::hex << byte1High;
    outputFile << std::setw(2) << std::left << std::setfill(' ') << std::hex << byte1Low;
    outputFile << std::setw(1) << std::left << std::setfill(' ') << std::hex << byte2High;
    outputFile << std::setw(2) << std::left << std::setfill(' ') << std::hex << byte2Low;
    outputFile << std::setw(1) << std::left << std::setfill(' ') << std::hex << byte3High;
    outputFile << std::setw(2) << std::left << std::setfill(' ') << std::hex << byte3Low;
    outputFile << std::setw(1) << std::left << std::setfill(' ') << std::hex << byte4High;
    outputFile << std::setw(2) << std::left << std::setfill(' ') << std::hex << byte4Low;
    outputFile << std::endl;
  }

  void handleInstructionSecondPass(Instruction instruction)
  {
    if (instruction.mnemonic == "halt")
    {
      outputInstruction(0, 0, 0, 0, 0, 0, 0, 0);
    }
    if (instruction.mnemonic == "int")
    {
      outputInstruction(0, 0, 0, 0, 0, 0, 1, 0);
    }
    //iret - 3
    //call - 1
    //ret - 3
    //jmp - 1
    //beq - 1
    //bne - 1
    //bgt - 1
    //push - 2
    //pop - 2
    if (instruction.mnemonic == "xchg")
    {
      int regB = getGprIndex(instruction.reg1);
      int regC = getGprIndex(instruction.reg2);
      outputInstruction(4, 0, 0, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "add")
    {
      int regA = getGprIndex(instruction.reg2);
      int regB = getGprIndex(instruction.reg1);
      int regC = getGprIndex(instruction.reg2);
      outputInstruction(5, 0, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "sub")
    {
      int regA = getGprIndex(instruction.reg2);
      int regB = getGprIndex(instruction.reg1);
      int regC = getGprIndex(instruction.reg2);
      outputInstruction(5, 1, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "mul")
    {
      int regA = getGprIndex(instruction.reg2);
      int regB = getGprIndex(instruction.reg1);
      int regC = getGprIndex(instruction.reg2);
      outputInstruction(5, 2, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "div")
    {
      int regA = getGprIndex(instruction.reg2);
      int regB = getGprIndex(instruction.reg1);
      int regC = getGprIndex(instruction.reg2);
      outputInstruction(5, 3, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "not")
    {
      int regA = getGprIndex(instruction.reg1);
      int regB = getGprIndex(instruction.reg1);
      int regC = 0;
      outputInstruction(6, 0, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "and")
    {
      int regA = getGprIndex(instruction.reg2);
      int regB = getGprIndex(instruction.reg1);
      int regC = getGprIndex(instruction.reg2);
      outputInstruction(6, 1, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "or")
    {
      int regA = getGprIndex(instruction.reg2);
      int regB = getGprIndex(instruction.reg1);
      int regC = getGprIndex(instruction.reg2);
      outputInstruction(6, 2, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "xor")
    {
      int regA = getGprIndex(instruction.reg2);
      int regB = getGprIndex(instruction.reg1);
      int regC = getGprIndex(instruction.reg2);
      outputInstruction(6, 3, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "shl")
    {
      int regA = getGprIndex(instruction.reg2);
      int regB = getGprIndex(instruction.reg1);
      int regC = getGprIndex(instruction.reg2);
      outputInstruction(7, 0, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "shr")
    {
      int regA = getGprIndex(instruction.reg2);
      int regB = getGprIndex(instruction.reg1);
      int regC = getGprIndex(instruction.reg2);
      outputInstruction(7, 1, regA, regB, regC, 0, 0, 0);
    }
    //ld - 2
    //st - 2
    //csrrd - 2
    //csrwr - 2
    locationCounter += 4;
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
    std::cout << locationCounter << std::endl;
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
  }

  void assemble()
  {
    firstPass();
    secondPass();
  }

}