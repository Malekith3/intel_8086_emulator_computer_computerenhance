//
// Created by sanek on 15/07/2024.
//

#include <iostream>
#include <fstream>
#include <cstdint>
#include <map>

enum class INSTRUCTION_MASKS : uint16_t
{
  OP_CODE = 0xFC00,
  W = 0x0200,
  D = 0x0100,
  MOD = 0x0060,
  REG = 0x001C,
  REG_MEM = 0x0007
};

enum class OP_CODE_VALUES : uint8_t
{
  MOV = 0xD8
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
  //Fetching OP CODE
  uint16_t opCode = instruction & static_cast<uint16_t>(INSTRUCTION_MASKS::OP_CODE);
  opCode >>= 8;

  // Fetching W
  // Fetching D
  // Fetching Reg
  // Fetching REG_MEM


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