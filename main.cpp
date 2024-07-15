//
// Created by sanek on 15/07/2024.
//

#include <iostream>
#include <fstream>
#include <map>

enum class INSTRUCTION_MASKS : uint16_t
{
  OP_CODE = 0x00FC,
  W = 0x0001,
  D = 0x0002,
  MOD = 0xC000,
  REG = 0x3800,
  REG_MEM = 0x0700
};

enum class OP_CODE_VALUES : uint8_t
{
  MOV = 0x22
};

std::map<INSTRUCTION_MASKS,int> shiftRight{
    {INSTRUCTION_MASKS::OP_CODE,2},
    {INSTRUCTION_MASKS::W,0},
    {INSTRUCTION_MASKS::D,1},
    {INSTRUCTION_MASKS::MOD,14},
    {INSTRUCTION_MASKS::REG,11},
    {INSTRUCTION_MASKS::REG_MEM,8},

};

std::string OpCodeToString(OP_CODE_VALUES opCode)
{
  static const std::map<OP_CODE_VALUES, std::string> opCodesToString{
      {OP_CODE_VALUES::MOV, "mov"}
  };

  auto it = opCodesToString.find(opCode);
  return (it != opCodesToString.end()) ? it->second : "";

}

void processInstruction(uint16_t instruction)
{
  auto fetchingFunc = [&instruction](auto dataMask)
      {
        uint16_t fetchedData = instruction & static_cast<uint16_t>(dataMask);
        fetchedData >>= shiftRight[dataMask];
        return fetchedData;
      };

  //Fetching OP CODE
  uint16_t opCode = fetchingFunc(INSTRUCTION_MASKS::OP_CODE);

  // Fetching W
  uint16_t wValue = fetchingFunc(INSTRUCTION_MASKS::W);

  // Fetching D
  uint16_t dValue = fetchingFunc(INSTRUCTION_MASKS::D);

  //Fetching Mode
  uint16_t mod = fetchingFunc(INSTRUCTION_MASKS::MOD);

  // Fetching Reg
  uint16_t regValue = fetchingFunc(INSTRUCTION_MASKS::REG);

  // Fetching REG_MEM
  uint16_t regMemValue = fetchingFunc(INSTRUCTION_MASKS::REG_MEM);

  std::cout << OpCodeToString(static_cast<OP_CODE_VALUES>(opCode));
}

int main()
{
  std::ifstream  instructions("../asm_files/listing_0037_single_register_mov", std::ios::binary);

  if(!instructions)
  {
    std::cerr << "Got error on opening a file!" << std::endl;
    return -1;
  }

  // Buffer to hold the two bytes read from the file
  uint16_t buffer;

  // Read two bytes at a time
  while (instructions.read(reinterpret_cast<char*>(&buffer), sizeof(buffer))) {
    // Process the two bytes (buffer) as needed
    // For example, you can print the value in hexadecimal
    std::cout << std::hex << buffer << std::endl;
    processInstruction(buffer);
  }

  return 0;

}