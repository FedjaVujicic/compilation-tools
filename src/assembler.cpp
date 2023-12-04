#include <vector>
#include <iomanip>
#include <unordered_map>
#include <unordered_set>
#include "../misc/parser.hpp"
#include "../inc/assembler.hpp"
#include "../inc/relocation.hpp"
#include "../inc/symbol.hpp"
#include "../inc/section.hpp"

extern std::vector<Line> parsedLines;
extern FILE *yyin;
extern int yylineno;
extern void printParsingStatus(int parseStatus);
extern void printParsingData();

namespace assembler
{
  FILE *inputFile;
  std::ofstream outputFile;
  std::unordered_set<std::string> globalSymbols;
  std::unordered_map<std::string, Symbol> symbolTable;
  std::unordered_map<std::string, Section> sectionTable;
  std::map<std::pair<std::string, unsigned>, unsigned> literalNumTable;
  std::map<std::pair<std::string, std::string>, unsigned> literalSymTable;
  std::unordered_map<std::string, std::vector<Relocation>> relocationTable;
  std::string currentSection = "ABS";
  unsigned locationCounter = 0;

  unsigned stringToUnsignedInt(std::string value)
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

  int stringToSignedInt(std::string value)
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
    if (symbolTable.count(symbolName) > 0 && symbolTable[symbolName].section != "UND")
    {
      std::cout << "Assembler error, symbol " << symbolName << " redefinition." << std::endl;
      exit(1);
    }
    if (globalSymbols.count(symbolName) == 0)
    {
      symbolTable[symbolName].scope = ScopeType::LOCAL;
    }
    symbolTable[symbolName].value = locationCounter - sectionTable[currentSection].base;
    symbolTable[symbolName].size = 0;
    symbolTable[symbolName].type = SymbolType::NOTYPE;
    symbolTable[symbolName].section = currentSection;
  }

  void addSectionSymbol(std::string symbolName)
  {
    if (symbolTable.count(symbolName) > 0)
    {
      std::cout << "Assembler error, symbol " << symbolName << " redefinition." << std::endl;
      exit(1);
    }
    symbolTable[symbolName] = {0, 0, SymbolType::SECTION, ScopeType::LOCAL, symbolName};
  }

  void addInstructionSymbol(std::string symbolName)
  {
    if (symbolTable.count(symbolName) > 0)
    {
      return;
    }
    symbolTable[symbolName] = {0, 0, SymbolType::NOTYPE, ScopeType::GLOBAL, "UND"};
  }

  void addRelocationInstruction(std::string symbolName)
  {
    unsigned relOffset = literalSymTable[std::make_pair(currentSection, symbolName)];
    unsigned relAddend;
    std::string relSymbolName;
    if (symbolTable[symbolName].scope == ScopeType::LOCAL)
    {
      relAddend = symbolTable[symbolName].value;
      relSymbolName = symbolTable[symbolName].section;
    }
    else if (symbolTable[symbolName].scope == ScopeType::GLOBAL)
    {
      relAddend = 0;
      relSymbolName = symbolName;
    }
    relocationTable[currentSection].push_back({relOffset, relSymbolName, relAddend});
  }

  void addRelocationWordDirective(std::string symbolName)
  {
    unsigned relOffset = locationCounter - sectionTable[currentSection].base;
    unsigned relAddend = 0;
    std::string relSymbolName;
    if (symbolTable[symbolName].scope == ScopeType::LOCAL)
    {
      relAddend = symbolTable[symbolName].value;
      relSymbolName = symbolTable[symbolName].section;
    }
    else if (symbolTable[symbolName].scope == ScopeType::GLOBAL)
    {
      relAddend = 0;
      relSymbolName = symbolName;
    }
    relocationTable[currentSection].push_back({relOffset, relSymbolName, relAddend});
    
  }

  void outputByte(unsigned byteHigh, unsigned byteLow)
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

  void outputWord(unsigned byte4High, unsigned byte4Low, unsigned byte3High, unsigned byte3Low, unsigned byte2High, unsigned byte2Low, unsigned byte1High, unsigned byte1Low)
  {
    outputByte(byte1High, byte1Low);
    outputByte(byte2High, byte2Low);
    outputByte(byte3High, byte3Low);
    outputByte(byte4High, byte4Low);
  }

  void outputWordDisp(unsigned byte4High, unsigned byte4Low, unsigned byte3High, unsigned byte3Low, unsigned byte2High, unsigned disp)
  {
    unsigned byte2Low = (disp & 0x00000F00) >> 8;
    unsigned byte1High = (disp & 0x000000F0) >> 4;
    unsigned byte1Low = (disp & 0x0000000F);
    outputByte(byte1High, byte1Low);
    outputByte(byte2High, byte2Low);
    outputByte(byte3High, byte3Low);
    outputByte(byte4High, byte4Low);
  }

  void outputInteger(unsigned value)
  {
    unsigned byte4High = (value & 0xF0000000) >> 28;
    unsigned byte4Low = (value & 0x0F000000) >> 24;
    unsigned byte3High = (value & 0x00F00000) >> 20;
    unsigned byte3Low = (value & 0x000F0000) >> 16;
    unsigned byte2High = (value & 0x0000F000) >> 12;
    unsigned byte2Low = (value & 0x00000F00) >> 8;
    unsigned byte1High = (value & 0x000000F0) >> 4;
    unsigned byte1Low = (value & 0x0000000F);
    outputWord(byte4High, byte4Low, byte3High, byte3Low,
               byte2High, byte2Low, byte1High, byte1Low);
  }

  void outputString(std::string value)
  {
    std::vector<unsigned> bytes;
    for (const auto &c : value)
    {
      bytes.push_back(c);
    }
    for (const auto &byte : bytes)
    {
      unsigned byteHigh = (byte & 0x000000F0) >> 4;
      unsigned byteLow = (byte & 0x0000000F);
      outputByte(byteHigh, byteLow);
    }
  }

  void literalPoolFirstPass()
  {
    for (auto &num : literalNumTable)
    {
      if (num.first.first != currentSection)
      {
        continue;
      }
      num.second = locationCounter - sectionTable[currentSection].base;
      locationCounter += 4;
    }
    for (auto &sym : literalSymTable)
    {
      if (sym.first.first != currentSection)
      {
        continue;
      }
      sym.second = locationCounter - sectionTable[currentSection].base;
      locationCounter += 4;
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
          if (symbolTable.count(arg.value) > 0)
          {
            std::cout << "Assembler error, symbol " << arg.value << " redefinition." << std::endl;
            exit(1);
          }
          symbolTable[arg.value] = {0, 0, SymbolType::NOTYPE, ScopeType::GLOBAL, "UND"};
          globalSymbols.insert(arg.value);
        }
      }
    }
    if (directive.mnemonic == "global")
    {
      for (const auto &arg : directive.argList)
      {
        if (arg.type == "symbol")
        {
          if (symbolTable.count(arg.value) > 0)
          {
            std::cout << "Assembler error, symbol " << arg.value << " redefinition." << std::endl;
            exit(1);
          }
          symbolTable[arg.value] = {0, 0, SymbolType::NOTYPE, ScopeType::GLOBAL, "UND"};
          globalSymbols.insert(arg.value);
        }
      }
    }
    if (directive.mnemonic == "section")
    {
      if (currentSection != "ABS")
      {
        literalPoolFirstPass();
      }
      while (locationCounter % 8)
      {
        ++locationCounter;
      }
      addSectionSymbol(directive.argList[0].value);
      sectionTable[currentSection].length = locationCounter - sectionTable[currentSection].base;
      currentSection = directive.argList[0].value;
      sectionTable[currentSection].base = locationCounter;
    }
    if (directive.mnemonic == "word")
    {
      for (const auto &arg : directive.argList)
      {
        if (arg.type == "symbol")
        {
          if (symbolTable.count(arg.value) > 0)
          {
            continue;
          }
          symbolTable[arg.value] = {0, 0, SymbolType::NOTYPE, ScopeType::GLOBAL, "UND"};
        }
      }
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

  void handleInstructionFirstPass(Instruction instruction)
  {
    if (instruction.mnemonic == "iret" ||
        (instruction.mnemonic == "ld" && (instruction.operand_type == "mem[num]" || instruction.operand_type == "mem[sym]")))
    {
      locationCounter += 4;
    }
    locationCounter += 4;

    if (instruction.mnemonic == "call" || instruction.mnemonic == "jmp" || instruction.mnemonic == "beq" ||
        instruction.mnemonic == "bne" || instruction.mnemonic == "bgt")
    {
      if (instruction.operand_type == "num")
      {
        literalNumTable[std::make_pair(currentSection, stringToUnsignedInt(instruction.operand))];
      }
      if (instruction.operand_type == "sym")
      {
        addInstructionSymbol(instruction.operand);
        literalSymTable[std::make_pair(currentSection, instruction.operand)];
      }
    }

    if (instruction.mnemonic == "st")
    {
      if (instruction.operand_type == "mem[num]")
      {
        literalNumTable[std::make_pair(currentSection, stringToUnsignedInt(instruction.operand))];
      }
      if (instruction.operand_type == "mem[sym]")
      {
        addInstructionSymbol(instruction.operand);
        literalSymTable[std::make_pair(currentSection, instruction.operand)];
      }
    }

    if (instruction.mnemonic == "ld")
    {
      if (instruction.operand_type == "num" || instruction.operand_type == "mem[num]")
      {
        literalNumTable[std::make_pair(currentSection, stringToUnsignedInt(instruction.operand))];
      }
      if (instruction.operand_type == "sym" || instruction.operand_type == "mem[sym]")
      {
        addInstructionSymbol(instruction.operand);
        literalSymTable[std::make_pair(currentSection, instruction.operand)];
      }
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
      handleInstructionFirstPass(line.instruction);
    }
  }

  void handleDirectiveSecondPass(Directive directive)
  {
    if (directive.mnemonic == "word")
    {
      for (const auto &arg : directive.argList)
      {
        if (arg.type == "symbol")
        {
          addRelocationWordDirective(arg.value);
          outputInteger(0);
        }
        if (arg.type == "number")
        {
          unsigned value = stringToUnsignedInt(arg.value);
          outputInteger(value);
        }
      }
    }
    if (directive.mnemonic == "skip")
    {
      unsigned size = stoi(directive.argList[0].value);
      for (unsigned i = 0; i < size; ++i)
      {
        outputByte(0, 0);
      }
    }
    if (directive.mnemonic == "ascii")
    {
      outputString(directive.argList[0].value);
    }
  }

  unsigned getGprIndex(std::string regCode)
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

  unsigned getCsrIndex(std::string regCode)
  {
    if (regCode == "%status")
    {
      return 0;
    }
    if (regCode == "%handler")
    {
      return 1;
    }
    if (regCode == "%cause")
    {
      return 2;
    }
    std::cout << "Invalid CSR index" << std::endl;
    exit(1);
  }

  unsigned getDisplacement(std::string operand, std::string type)
  {
    if (type == "num" || type == "mem[num]")
    {
      return literalNumTable[std::make_pair(currentSection, stringToUnsignedInt(operand))] - locationCounter;
    }
    return literalSymTable[std::make_pair(currentSection, operand)] - locationCounter;
  }

  void handleInstructionSecondPass(Instruction instruction)
  {
    if (instruction.mnemonic == "halt")
    {
      outputWord(0, 0, 0, 0, 0, 0, 0, 0);
    }
    if (instruction.mnemonic == "int")
    {
      outputWord(1, 0, 0, 0, 0, 0, 0, 0);
    }
    if (instruction.mnemonic == "iret")
    {
      outputWord(9, 3, 15, 14, 0, 0, 0, 4);
      outputWord(9, 7, 0, 14, 0, 0, 0, 4);
    }
    if (instruction.mnemonic == "call")
    {
      unsigned disp = getDisplacement(instruction.operand, instruction.operand_type);
      outputWordDisp(2, 1, 15, 0, 0, disp);
    }
    if (instruction.mnemonic == "ret")
    {
      outputWord(9, 3, 15, 14, 0, 0, 0, 4);
    }
    if (instruction.mnemonic == "jmp")
    {
      unsigned disp = getDisplacement(instruction.operand, instruction.operand_type);
      outputWordDisp(3, 8, 15, 0, 0, disp);
    }
    if (instruction.mnemonic == "beq")
    {
      unsigned disp = getDisplacement(instruction.operand, instruction.operand_type);
      unsigned regB = getGprIndex(instruction.reg1);
      unsigned regC = getGprIndex(instruction.reg2);
      outputWordDisp(3, 9, 15, regB, regC, disp);
    }
    if (instruction.mnemonic == "bne")
    {
      unsigned disp = getDisplacement(instruction.operand, instruction.operand_type);
      unsigned regB = getGprIndex(instruction.reg1);
      unsigned regC = getGprIndex(instruction.reg2);
      outputWordDisp(3, 10, 15, regB, regC, disp);
    }
    if (instruction.mnemonic == "bgt")
    {
      unsigned disp = getDisplacement(instruction.operand, instruction.operand_type);
      unsigned regB = getGprIndex(instruction.reg1);
      unsigned regC = getGprIndex(instruction.reg2);
      outputWordDisp(3, 11, 15, regB, regC, disp);
    }
    if (instruction.mnemonic == "push")
    {
      unsigned regC = getGprIndex(instruction.reg1);
      outputWord(8, 1, 14, 0, regC, 0, 0, 4);
    }
    if (instruction.mnemonic == "pop")
    {
      unsigned regA = getGprIndex(instruction.reg1);
      outputWord(9, 3, regA, 14, 0, 0, 0, 4);
    }
    if (instruction.mnemonic == "xchg")
    {
      unsigned regB = getGprIndex(instruction.reg1);
      unsigned regC = getGprIndex(instruction.reg2);
      outputWord(4, 0, 0, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "add")
    {
      unsigned regA = getGprIndex(instruction.reg2);
      unsigned regB = getGprIndex(instruction.reg1);
      unsigned regC = getGprIndex(instruction.reg2);
      outputWord(5, 0, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "sub")
    {
      unsigned regA = getGprIndex(instruction.reg2);
      unsigned regB = getGprIndex(instruction.reg1);
      unsigned regC = getGprIndex(instruction.reg2);
      outputWord(5, 1, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "mul")
    {
      unsigned regA = getGprIndex(instruction.reg2);
      unsigned regB = getGprIndex(instruction.reg1);
      unsigned regC = getGprIndex(instruction.reg2);
      outputWord(5, 2, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "div")
    {
      unsigned regA = getGprIndex(instruction.reg2);
      unsigned regB = getGprIndex(instruction.reg1);
      unsigned regC = getGprIndex(instruction.reg2);
      outputWord(5, 3, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "not")
    {
      unsigned regA = getGprIndex(instruction.reg1);
      unsigned regB = getGprIndex(instruction.reg1);
      unsigned regC = 0;
      outputWord(6, 0, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "and")
    {
      unsigned regA = getGprIndex(instruction.reg2);
      unsigned regB = getGprIndex(instruction.reg1);
      unsigned regC = getGprIndex(instruction.reg2);
      outputWord(6, 1, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "or")
    {
      unsigned regA = getGprIndex(instruction.reg2);
      unsigned regB = getGprIndex(instruction.reg1);
      unsigned regC = getGprIndex(instruction.reg2);
      outputWord(6, 2, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "xor")
    {
      unsigned regA = getGprIndex(instruction.reg2);
      unsigned regB = getGprIndex(instruction.reg1);
      unsigned regC = getGprIndex(instruction.reg2);
      outputWord(6, 3, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "shl")
    {
      unsigned regA = getGprIndex(instruction.reg2);
      unsigned regB = getGprIndex(instruction.reg1);
      unsigned regC = getGprIndex(instruction.reg2);
      outputWord(7, 0, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "shr")
    {
      unsigned regA = getGprIndex(instruction.reg2);
      unsigned regB = getGprIndex(instruction.reg1);
      unsigned regC = getGprIndex(instruction.reg2);
      outputWord(7, 1, regA, regB, regC, 0, 0, 0);
    }
    if (instruction.mnemonic == "ld")
    {
      if (instruction.operand_type == "num" || instruction.operand_type == "sym" ||
          instruction.operand_type == "mem[num]" || instruction.operand_type == "mem[sym]")
      {
        unsigned disp = getDisplacement(instruction.operand, instruction.operand_type);
        unsigned regA = getGprIndex(instruction.reg1);
        outputWordDisp(9, 2, regA, 15, 0, disp);
      }
      if (instruction.operand_type == "mem[num]" || instruction.operand_type == "mem[sym]")
      {
        unsigned regA = getGprIndex(instruction.reg1);
        outputWord(9, 2, regA, regA, 0, 0, 0, 0);
      }
      if (instruction.operand_type == "mem[reg]")
      {
        unsigned regA = getGprIndex(instruction.reg1);
        unsigned regB = getGprIndex(instruction.operand);
        outputWord(9, 2, regA, regB, 0, 0, 0, 0);
      }
      if (instruction.operand_type == "mem[reg+num]")
      {
        if (stringToSignedInt(instruction.offset) > 2047 || stringToSignedInt(instruction.offset) < -2048)
        {
          std::cout << "Error. Signed offset out of range" << std::endl;
          exit(1);
        }
        unsigned regA = getGprIndex(instruction.reg1);
        unsigned regB = getGprIndex(instruction.operand);
        int disp = stringToSignedInt(instruction.offset);
        outputWordDisp(9, 2, regA, regB, 0, disp);
      }
    }
    if (instruction.mnemonic == "st")
    {
      if (instruction.operand_type == "mem[num]" || instruction.operand_type == "mem[sym]")
      {
        unsigned disp = getDisplacement(instruction.operand, instruction.operand_type);
        unsigned regC = getGprIndex(instruction.reg1);
        outputWordDisp(8, 2, 15, 0, regC, disp);
      }
      if (instruction.operand_type == "mem[reg]")
      {
        unsigned regC = getGprIndex(instruction.reg1);
        unsigned regA = getGprIndex(instruction.operand);
        outputWord(8, 0, regA, 0, regC, 0, 0, 0);
      }
      if (instruction.operand_type == "mem[reg+num]")
      {
        if (stringToSignedInt(instruction.offset) > 2047 || stringToSignedInt(instruction.offset) < -2048)
        {
          std::cout << "Error. Signed offset out of range" << std::endl;
          exit(1);
        }
        unsigned regC = getGprIndex(instruction.reg1);
        unsigned regA = getGprIndex(instruction.operand);
        int disp = stringToSignedInt(instruction.offset);
        outputWordDisp(8, 0, regA, 0, regC, disp);
      }
    }
    if (instruction.mnemonic == "csrrd")
    {
      unsigned regA = getCsrIndex(instruction.reg1);
      unsigned regB = getGprIndex(instruction.reg2);
      outputWord(9, 4, regA, regB, 0, 0, 0, 0);
    }
    if (instruction.mnemonic == "csrwr")
    {
      unsigned regA = getGprIndex(instruction.reg1);
      unsigned regB = getCsrIndex(instruction.reg2);
      outputWord(9, 0, regA, regB, 0, 0, 0, 0);
    }
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

  void outputRelocationTables()
  {
    for (const auto& section : sectionTable)
    {
      if (section.first == "ABS")
      {
        continue;
      }
      outputFile << "#.rela." << section.first << std::endl;
      outputFile << std::setw(10) << std::left << std::setfill(' ') << "Offset";
      outputFile << std::setw(20) << std::left << std::setfill(' ') << "Symbol";
      outputFile << std::setw(10) << std::left << std::setfill(' ') << "Addend";
      outputFile << std::endl;
      for (const auto &rel : relocationTable[section.first])
      {
        outputFile << std::setw(8) << std::right << std::setfill('0') << std::hex << rel.offset << "  ";
        outputFile << std::setw(20) << std::left << std::setfill(' ') << rel.symbolName;
        outputFile << std::setw(10) << std::left << std::setfill(' ') << std::dec << rel.addend;
        outputFile << std::endl;
      }
    }
  }

  void literalPoolSecondPass()
  {
    for (const auto &num : literalNumTable)
    {
      if (num.first.first != currentSection)
      {
        continue;
      }
      outputInteger((num.first).second);
    }
    for (const auto &sym : literalSymTable)
    {
      if (sym.first.first != currentSection)
      {
        continue;
      }
      outputInteger(0);
      addRelocationInstruction(sym.first.second);
    }
  }

  void outputLiteralPool()
  {
    for (const auto& num : literalNumTable)
    {
      std::cout << "Section(" << num.first.first << ") ";
      std::cout << "Literal(" << num.first.second << ") ";
      std::cout << "Address(" << num.second << ") " << std::endl;
    }
    for (const auto& sym : literalSymTable)
    {
      std::cout << "Section(" << sym.first.first << ") ";
      std::cout << "Literal(" << sym.first.second << ") ";
      std::cout << "Address(" << sym.second << ") " << std::endl;
    }
  }

  void firstPass()
  {
    yyin = inputFile;
    int parseStatus = yyparse();
    literalPoolFirstPass();
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
          if (currentSection != "ABS")
          {
            literalPoolSecondPass();
          }
          if (locationCounter % 8)
          {
            outputFile << std::endl;
          }
          while (locationCounter % 8)
          {
            ++locationCounter;
          }
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
    literalPoolSecondPass();
    outputRelocationTables();
    locationCounter = 0;
  }

  void assemble()
  {
    firstPass();
    secondPass();
  }

}