#include "../inc/emulator.hpp"

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    std::cout << "Invalid command." << std::endl;
    return 1;
  }

  std::string inputFileName = argv[1];

  emulator::setInputFile(inputFileName);  
  emulator::run();

  return 0;
}