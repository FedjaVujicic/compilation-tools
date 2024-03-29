#include "../inc/assembler.hpp"
#include <map>
#include <iomanip>
#include <vector>

#define SP r[14]
#define PC r[15]

#define STATUS csr[0]
#define HANDLER csr[1]
#define CAUSE csr[2]

namespace emulator
{
  std::ifstream inputFile;
  std::map<uint32_t, uint16_t> mem;
  std::vector<uint32_t> r(15);
  std::vector<uint32_t> csr(3);
  bool stopEmulation = false;
  bool printInstructions = false;

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
      if (currentWord[0] == '#')
      {
        return;
      }
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

  uint32_t fetchInstruction()
  {
    uint16_t byte1 = mem[PC];
    uint16_t byte2 = mem[PC + 1];
    uint16_t byte3 = mem[PC + 2];
    uint16_t byte4 = mem[PC + 3];
    uint32_t instruction = (byte4 << 24) | (byte3 << 16) | (byte2 << 8) | (byte1 << 0);
    PC += 4;
    return instruction;
  }

  // Returns a 4 byte word from memory for the specified address
  uint32_t readWord(uint32_t addr)
  {
    uint16_t byte1 = mem[addr];
    uint16_t byte2 = mem[addr + 1];
    uint16_t byte3 = mem[addr + 2];
    uint16_t byte4 = mem[addr + 3];
    uint32_t word = (byte4 << 24) | (byte3 << 16) | (byte2 << 8) | (byte1 << 0);
    return word;
  }

  // Writes a 4 byte word to memory at the specified address
  void writeWord(uint32_t addr, uint32_t word)
  {
    uint16_t byte4 = (word & 0xFF000000) >> 24;
    uint16_t byte3 = (word & 0x00FF00FF) >> 16;
    uint16_t byte2 = (word & 0x0000FF00) >> 8;
    uint16_t byte1 = (word & 0x000000FF) >> 0;
    mem[addr] = byte1;
    mem[addr + 1] = byte2;
    mem[addr + 2] = byte3;
    mem[addr + 3] = byte4;
  }

  void printInt()
  {
    std::cout << "int" << std::endl;
  }

  void printCall(uint32_t instruction)
  {
    uint16_t mod = ((instruction & 0x0F000000) >> 24);
    uint16_t regA = ((instruction & 0x00F00000) >> 20);
    uint16_t regB = ((instruction & 0x000F0000) >> 16);
    uint16_t disp = (instruction & 0x00000FFF);

    switch (mod)
    {
    case 0b0001:
      std::cout << "call ";
      std::cout << "0x" << std::hex << readWord(r[regA] + disp);
      std::cout << std::endl;
      break;
    default:
      std::cout << "Emulator error. Invalid instruction modifier." << std::endl;
      exit(1);
    }
  }

  void printBranch(uint32_t instruction)
  {
    uint16_t mod = ((instruction & 0x0F000000) >> 24);
    uint16_t regA = ((instruction & 0x00F00000) >> 20);
    uint16_t regB = ((instruction & 0x000F0000) >> 16);
    uint16_t regC = ((instruction & 0x0000F000) >> 12);
    uint16_t disp = (instruction & 0x00000FFF);

    switch (mod)
    {
    case 0b1000:
      std::cout << "jmp ";
      std::cout << "0x" << std::hex << readWord(r[regA] + disp);
      std::cout << std::endl;
      break;
    case 0b1001:
      std::cout << "beq ";
      std::cout << "%r" << std::dec << regB << ", "
                << "%r" << std::dec << regC << ", ";
      std::cout << "0x" << std::hex << readWord(r[regA] + disp);
      std::cout << std::endl;
      break;
    case 0b1010:
      std::cout << "bne ";
      std::cout << "%r" << std::dec << regB << ", "
                << "%r" << std::dec << regC << ", ";
      std::cout << "0x" << std::hex << readWord(r[regA] + disp);
      std::cout << std::endl;
      break;
    case 0b1011:
      std::cout << "bgt ";
      std::cout << "%r" << std::dec << regB << ", "
                << "%r" << std::dec << regC << ", ";
      std::cout << "0x" << std::hex << readWord(r[regA] + disp);
      std::cout << std::endl;
      break;
    default:
      std::cout << "Emulator error. Invalid instruction modifier." << std::endl;
      exit(1);
    }
  }

  void printXchg(uint32_t instruction)
  {
    uint16_t regB = ((instruction & 0x000F0000) >> 16);
    uint16_t regC = ((instruction & 0x0000F000) >> 12);
    std::cout << "xchg %r" << std::dec << regB << ", %r" << std::dec << regC << std::endl;
  }

  void printArithmOp(uint32_t instruction)
  {
    uint16_t mod = ((instruction & 0x0F000000) >> 24);
    uint16_t regA = ((instruction & 0x00F00000) >> 20);
    uint16_t regB = ((instruction & 0x000F0000) >> 16);
    uint16_t regC = ((instruction & 0x0000F000) >> 12);

    switch (mod)
    {
    case 0b0000:
      std::cout << "add ";
      break;
    case 0b0001:
      std::cout << "sub ";
      break;
    case 0b0010:
      std::cout << "mul ";
      break;
    case 0b0011:
      std::cout << "div ";
      break;
    default:
      std::cout << "Invalid instruction modifier." << std::endl;
      exit(1);
      break;
    }
    std::cout << "%r" << std::dec << regB << ", %r" << std::dec << regC << std::endl;
  }

  void printLogOp(uint32_t instruction)
  {
    uint16_t mod = ((instruction & 0x0F000000) >> 24);
    uint16_t regA = ((instruction & 0x00F00000) >> 20);
    uint16_t regB = ((instruction & 0x000F0000) >> 16);
    uint16_t regC = ((instruction & 0x0000F000) >> 12);

    switch (mod)
    {
    case 0b0000:
      std::cout << "not %r" << std::dec << regB << std::endl;
      break;
    case 0b0001:
      std::cout << "and %r" << std::dec << regB << ", %r" << std::dec << regC << std::endl;
      break;
    case 0b0010:
      std::cout << "or %r" << std::dec << regB << ", %r" << std::dec << regC << std::endl;
      break;
    case 0b0011:
      std::cout << "xor %r" << std::dec << regB << ", %r" << std::dec << regC << std::endl;
      break;
    default:
      std::cout << "Invalid instruction modifier." << std::endl;
      exit(1);
      break;
    }
  }

  void printShift(uint32_t instruction)
  {
    uint16_t mod = ((instruction & 0x0F000000) >> 24);
    uint16_t regA = ((instruction & 0x00F00000) >> 20);
    uint16_t regB = ((instruction & 0x000F0000) >> 16);
    uint16_t regC = ((instruction & 0x0000F000) >> 12);

    switch (mod)
    {
    case 0b0000:
      std::cout << "shl ";
      break;
    case 0b0001:
      std::cout << "shr ";
      break;
    default:
      std::cout << "Invalid instruction modifier." << std::endl;
      exit(1);
      break;
    }
    std::cout << "%r" << std::dec << regB << ", %r" << std::dec << regC << std::endl;
  }

  void printStore(uint32_t instruction)
  {
    uint16_t mod = ((instruction & 0x0F000000) >> 24);
    uint16_t regA = ((instruction & 0x00F00000) >> 20);
    uint16_t regB = ((instruction & 0x000F0000) >> 16);
    uint16_t regC = ((instruction & 0x0000F000) >> 12);
    uint16_t disp = (instruction & 0x00000FFF);

    switch (mod)
    {
    case 0b0000:
      std::cout << "st ";
      std::cout << "%r" << std::dec << regC << ", ";
      std::cout << "[%r" << std::dec << regA;
      std::cout << " + 0x" << std::hex << disp << "]";
      std::cout << std::endl;
      break;
    case 0b0010:
      std::cout << "st ";
      std::cout << "%r" << std::dec << regC << ", ";
      std::cout << "0x" << std::hex << readWord(r[regA] + disp);
      std::cout << std::endl;
      break;
    case 0b0001:
      std::cout << "push ";
      std::cout << "%r" << std::dec << regC << std::endl;
      break;
    default:
      std::cout << "Emulator error. Invalid instruction modifier." << std::endl;
      exit(1);
    }
  }

  void printLoad(uint32_t instruction)
  {
    uint16_t mod = ((instruction & 0x0F000000) >> 24);
    uint16_t regA = ((instruction & 0x00F00000) >> 20);
    uint16_t regB = ((instruction & 0x000F0000) >> 16);
    uint16_t regC = ((instruction & 0x0000F000) >> 12);
    int16_t disp = (instruction & 0x00000FFF);
    if (disp & 0x0800)
    {
      disp |= 0xF000; // Set sign bits for negative numbers
    }
    std::string csr;

    switch (mod)
    {
    case 0b0010:
      std::cout << "ld ";
      if (regB == 15)
      {
        std::cout << "$0x" << std::hex << readWord(r[regB] + disp);
      }
      else
      {
        std::cout << "[%r" << std::dec << regB << " + 0x" << std::hex << disp << "]";
      }
      std::cout << ", %r" << std::dec << regA << std::endl;
      break;
    case 0b0011:
      std::cout << "pop ";
      std::cout << "%r" << std::dec << regA << "(" << std::dec << disp << ")" << std::endl;
      break;
    case 0b0111:
      std::cout << "pop ";
      switch (regA)
      {
      case 0:
        csr = "status";
        break;
      case 1:
        csr = "handler";
        break;
      case 2:
        csr = "cause";
        break;
      default:
        csr = "ERROR_REG";
        break;
      }
      std::cout << csr << "(" << std::dec << disp << ")" << std::endl;
      break;
    case 0b0100:
      std::cout << "csrwr ";
      std::cout << "%r" << std::dec << regB << ", ";
      switch (regA)
      {
      case 0:
        csr = "status";
        break;
      case 1:
        csr = "handler";
        break;
      case 2:
        csr = "cause";
        break;
      default:
        csr = "ERROR_REG";
        break;
      }
      std::cout << "%" << csr << std::endl;
      break;
    case 0b0000:
      std::cout << "csrrd ";
      switch (regB)
      {
      case 0:
        csr = "status";
        break;
      case 1:
        csr = "handler";
        break;
      case 2:
        csr = "cause";
        break;
      default:
        csr = "ERROR_REG";
        break;
      }
      std::cout << "%" << csr << ", ";
      std::cout << "%r" << std::dec << regA << std::endl;
      break;
    case 0b0001:
      std::cout << "%r" << regA << " = %r" << regB;
      if (disp >= 0)
      {
        std::cout << " + ";
      }
      std::cout << disp << std::endl;
      break;
    default:
      std::cout << "Emulator error. Invalid instruction modifier." << std::endl;
      exit(1);
    }
  }

  void pushReg(uint32_t value)
  {
    SP -= 4;
    writeWord(SP, value);
  }

  void popReg(uint32_t &reg)
  {
    reg = readWord(SP);
    SP += 4;
  }

  void executeHalt()
  {
    stopEmulation = true;
    std::cout << "Emulated processor executed halt instruction" << std::endl;
    std::cout << "Emulated processor state:" << std::endl;
    std::cout << std::setw(6) << std::setfill(' ') << "r0=0x" << std::hex << std::setw(8) << std::setfill('0') << r[0] << " ";
    std::cout << std::setw(6) << std::setfill(' ') << "r1=0x" << std::hex << std::setw(8) << std::setfill('0') << r[1] << " ";
    std::cout << std::setw(6) << std::setfill(' ') << "r2=0x" << std::hex << std::setw(8) << std::setfill('0') << r[2] << " ";
    std::cout << std::setw(6) << std::setfill(' ') << "r3=0x" << std::hex << std::setw(8) << std::setfill('0') << r[3] << std::endl;
    std::cout << std::setw(6) << std::setfill(' ') << "r4=0x" << std::hex << std::setw(8) << std::setfill('0') << r[4] << " ";
    std::cout << std::setw(6) << std::setfill(' ') << "r5=0x" << std::hex << std::setw(8) << std::setfill('0') << r[5] << " ";
    std::cout << std::setw(6) << std::setfill(' ') << "r6=0x" << std::hex << std::setw(8) << std::setfill('0') << r[6] << " ";
    std::cout << std::setw(6) << std::setfill(' ') << "r7=0x" << std::hex << std::setw(8) << std::setfill('0') << r[7] << std::endl;
    std::cout << std::setw(6) << std::setfill(' ') << "r8=0x" << std::hex << std::setw(8) << std::setfill('0') << r[8] << " ";
    std::cout << std::setw(6) << std::setfill(' ') << "r9=0x" << std::hex << std::setw(8) << std::setfill('0') << r[9] << " ";
    std::cout << std::setw(6) << std::setfill(' ') << "r10=0x" << std::hex << std::setw(8) << std::setfill('0') << r[10] << " ";
    std::cout << std::setw(6) << std::setfill(' ') << "r11=0x" << std::hex << std::setw(8) << std::setfill('0') << r[11] << std::endl;
    std::cout << std::setw(6) << std::setfill(' ') << "r12=0x" << std::hex << std::setw(8) << std::setfill('0') << r[12] << " ";
    std::cout << std::setw(6) << std::setfill(' ') << "r13=0x" << std::hex << std::setw(8) << std::setfill('0') << r[13] << " ";
    std::cout << std::setw(6) << std::setfill(' ') << "r14=0x" << std::hex << std::setw(8) << std::setfill('0') << r[14] << " ";
    std::cout << std::setw(6) << std::setfill(' ') << "r15=0x" << std::hex << std::setw(8) << std::setfill('0') << r[15] << std::endl;
  }

  void executeInt()
  {
    if (printInstructions)
    {
      printInt();
    }
    pushReg(STATUS);
    pushReg(PC);
    CAUSE = 4;
    STATUS &= ~(0x1);
    PC = HANDLER;
  }

  void executeCall(uint32_t instruction)
  {
    if (printInstructions)
    {
      printCall(instruction);
    }
    uint16_t mod = ((instruction & 0x0F000000) >> 24);
    uint16_t regA = ((instruction & 0x00F00000) >> 20);
    uint16_t regB = ((instruction & 0x000F0000) >> 16);
    uint16_t disp = (instruction & 0x00000FFF);

    switch (mod)
    {
    case 0b0001:
      pushReg(PC);
      PC = readWord(r[regA] + r[regB] + disp);
      break;
    default:
      std::cout << "Emulator error. Invalid instruction modifier." << std::endl;
      exit(1);
    }
  }

  void executeBranch(uint32_t instruction)
  {
    if (printInstructions)
    {
      printBranch(instruction);
    }
    uint16_t mod = ((instruction & 0x0F000000) >> 24);
    uint16_t regA = ((instruction & 0x00F00000) >> 20);
    uint16_t regB = ((instruction & 0x000F0000) >> 16);
    uint16_t regC = ((instruction & 0x0000F000) >> 12);
    uint16_t disp = (instruction & 0x00000FFF);

    switch (mod)
    {
    case 0b1000:
      PC = readWord(r[regA] + disp);
      break;
    case 0b1001:
      if (r[regB] == r[regC])
      {
        PC = readWord(r[regA] + disp);
      }
      break;
    case 0b1010:
      if (r[regB] != r[regC])
      {
        PC = readWord(r[regA] + disp);
      }
      break;
    case 0b1011:
      if (r[regB] > r[regC])
      {
        PC = readWord(r[regA] + disp);
      }
      break;
    default:
      std::cout << "Emulator error. Invalid instruction modifier." << std::endl;
      exit(1);
    }
  }

  void executeXchg(uint32_t instruction)
  {
    if (printInstructions)
    {
      printXchg(instruction);
    }
    uint16_t regB = ((instruction & 0x000F0000) >> 16);
    uint16_t regC = ((instruction & 0x0000F000) >> 12);
    uint32_t temp = r[regB];
    r[regB] = r[regC];
    r[regC] = temp;
  }

  void executeArithmOp(uint32_t instruction)
  {
    if (printInstructions)
    {
      printArithmOp(instruction);
    }
    uint16_t mod = ((instruction & 0x0F000000) >> 24);
    uint16_t regA = ((instruction & 0x00F00000) >> 20);
    uint16_t regB = ((instruction & 0x000F0000) >> 16);
    uint16_t regC = ((instruction & 0x0000F000) >> 12);

    switch (mod)
    {
    case 0b0000:
      r[regA] = r[regB] + r[regC];
      break;
    case 0b0001:
      r[regA] = r[regB] - r[regC];
      break;
    case 0b0010:
      r[regA] = r[regB] * r[regC];
      break;
    case 0b0011:
      r[regA] = r[regB] / r[regC];
      break;
    default:
      std::cout << "Invalid instruction modifier." << std::endl;
      exit(1);
      break;
    }
  }

  void executeLogOp(uint32_t instruction)
  {
    if (printInstructions)
    {
      printLogOp(instruction);
    }
    uint16_t mod = ((instruction & 0x0F000000) >> 24);
    uint16_t regA = ((instruction & 0x00F00000) >> 20);
    uint16_t regB = ((instruction & 0x000F0000) >> 16);
    uint16_t regC = ((instruction & 0x0000F000) >> 12);

    switch (mod)
    {
    case 0b0000:
      r[regA] = ~r[regB];
      break;
    case 0b0001:
      r[regA] = r[regB] & r[regC];
      break;
    case 0b0010:
      r[regA] = r[regB] | r[regC];
      break;
    case 0b0011:
      r[regA] = r[regB] ^ r[regC];
      break;
    default:
      std::cout << "Invalid instruction modifier." << std::endl;
      exit(1);
      break;
    }
  }

  void executeShift(uint32_t instruction)
  {
    if (printInstructions)
    {
      printShift(instruction);
    }
    uint16_t mod = ((instruction & 0x0F000000) >> 24);
    uint16_t regA = ((instruction & 0x00F00000) >> 20);
    uint16_t regB = ((instruction & 0x000F0000) >> 16);
    uint16_t regC = ((instruction & 0x0000F000) >> 12);

    switch (mod)
    {
    case 0b0000:
      r[regA] = r[regB] << r[regC];
      break;
    case 0b0001:
      r[regA] = r[regB] >> r[regC];
      break;
    default:
      std::cout << "Invalid instruction modifier." << std::endl;
      exit(1);
      break;
    }
  }

  void executeStore(uint32_t instruction)
  {
    if (printInstructions)
    {
      printStore(instruction);
    }
    uint16_t mod = ((instruction & 0x0F000000) >> 24);
    uint16_t regA = ((instruction & 0x00F00000) >> 20);
    uint16_t regB = ((instruction & 0x000F0000) >> 16);
    uint16_t regC = ((instruction & 0x0000F000) >> 12);
    uint16_t disp = (instruction & 0x00000FFF);

    switch (mod)
    {
    // st mem[reg], mem[reg + literal]
    case 0b0000:
      writeWord(r[regA] + r[regB] + disp, r[regC]);
      break;
    // push
    case 0b0010:
      writeWord(readWord(r[regA] + r[regB] + disp), r[regC]);
      break;
    // st mem[literal], mem[symbol]
    case 0b0001:
      r[regA] = r[regA] - disp;
      writeWord(r[regA], r[regC]);
      break;
    default:
      std::cout << "Emulator error. Invalid instruction modifier." << std::endl;
      exit(1);
    }
  }

  void executeLoad(uint32_t instruction)
  {
    if (printInstructions)
    {
      printLoad(instruction);
    }
    uint16_t mod = ((instruction & 0x0F000000) >> 24);
    uint16_t regA = ((instruction & 0x00F00000) >> 20);
    uint16_t regB = ((instruction & 0x000F0000) >> 16);
    uint16_t regC = ((instruction & 0x0000F000) >> 12);
    int16_t disp = (instruction & 0x00000FFF);
    if (disp & 0x0800)
    {
      disp |= 0xF000; // Set sign bits for negative numbers
    }

    switch (mod)
    {
    // ld literal, symbol, mem[literal], mem[symbol], mem[reg], mem[reg + literal]
    case 0b0010:
      r[regA] = readWord(r[regB] + r[regC] + disp);
      break;
    case 0b0011:
    // pop
      r[regA] = readWord(r[regB]);
      r[regB] = r[regB] + disp;
      break;
    case 0b0111:
    // pop csr
      csr[regA] = readWord(r[regB]);
      r[regB] = r[regB] + disp;
      break;
    case 0b0000:
    // csrwr
      r[regA] = csr[regB];
      break;
    case 0b0100:
    // csrrd
      csr[regA] = r[regB];
      break;
    case 0b0001:
    // for iret, sp = sp + 4
      r[regA] = r[regB] + disp;
      break;
    default:
      std::cout << "Emulator error. Invalid instruction modifier." << std::endl;
      exit(1);
    }
  }

  void executeInstruction(uint32_t instruction)
  {
    uint16_t opCode = instruction >> 28;
    switch (opCode)
    {
    case 0b0000:
      executeHalt();
      break;

    case 0b0001:
      executeInt();
      break;

    case 0b0010:
      executeCall(instruction);
      break;

    case 0b0011:
      executeBranch(instruction);
      break;

    case 0b0100:
      executeXchg(instruction);
      break;

    case 0b0101:
      executeArithmOp(instruction);
      break;

    case 0b0110:
      executeLogOp(instruction);
      break;

    case 0b0111:
      executeShift(instruction);
      break;

    case 0b1000:
      executeStore(instruction);
      break;

    case 0b1001:
      executeLoad(instruction);
      break;

    default:
      std::cout << "Emulator error. Invalid opcode." << std::endl;
      exit(1);
      break;
    }
  }

  void emulate()
  {
    r[0] = 0x00000000;
    PC = 0x40000000;
    while (!stopEmulation)
    {
      uint32_t currentInstruction = fetchInstruction();
      executeInstruction(currentInstruction);
      r[0] = 0x00000000;
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

  void run()
  {
    parseInput();

    emulate();

    // printMemoryContent();
  }
};