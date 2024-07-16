#include <iostream>
#include <fstream>
#include <map>
#include <array>
#include <fstream>
#include <sstream>

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

enum class register_names: uint8_t
{
  al = 0x00,
  cl = 0x01,
  dl = 0x02,
  bl = 0x03,
  ah = 0x04,
  ch = 0x05,
  dh = 0x06,
  bh = 0x07
};

enum class register_names_extended: uint8_t
{
  ax = 0x00,
  cx = 0x01,
  dx = 0x02,
  bx = 0x03,
  sp = 0x04,
  bp = 0x05,
  si = 0x06,
  di = 0x07
};

std::array<std::string, 8> regNamesToStr{
    "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"
};

std::array<std::string, 8> regNamesExtendedToStr{
    "ax", "cx", "dx", "bx", "sp", "bp", "si", "di"
};

std::string OpCodeToString(OP_CODE_VALUES opCode)
{
  static const std::map<OP_CODE_VALUES, std::string> opCodesToString{
      {OP_CODE_VALUES::MOV, "mov"}
  };

  auto it = opCodesToString.find(opCode);
  return (it != opCodesToString.end()) ? it->second : "";

}

std::string processInstruction(uint16_t instruction)
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

  if(mod != 0b11)
  {
    std::cerr << "Dont support mod that is not 0b11";
  }
  std::stringstream ss;
  auto regString = (wValue) ? regNamesExtendedToStr[regValue] : regNamesToStr[regValue];

  auto regMemValueString = (wValue) ? regNamesExtendedToStr[regMemValue] : regNamesToStr[regMemValue];

  ss << OpCodeToString(static_cast<OP_CODE_VALUES>(opCode)) << " " << regMemValueString  << "," << " " << regString << std::endl;
  return ss.str();
}

bool isFileEmpty(const std::string& filePath) {
  std::ifstream file(filePath, std::ios::binary | std::ios::ate); // Open the file in binary mode and move to the end
  if (!file.is_open()) {
    return true; // If the file cannot be opened, assume it's empty
  }
  return file.tellg() == 0; // Check if the file size is 0
}

void WriteInstructionToAnFile(std::string_view instruction)
{
  bool fileEmpty = isFileEmpty("../asm_files/output.asm");
  std::ofstream outFile("../asm_files/output.asm", std::ios::app);

  if (!outFile.is_open())
  {
    std::cerr << "Failed to open file for writing." << std::endl;
    return;
  }

  if(fileEmpty)
  {
    outFile << "bits 16" << std::endl;
    outFile << std::endl;
  }

  outFile << instruction;
}

int main()
{
  std::ifstream  instructions("../asm_files/listing_0038_many_register_mov", std::ios::binary);

  if(!instructions)
  {
    std::cerr << "Got error on opening a file!" << std::endl;
    return -1;
  }

  // Buffer to hold the two bytes read from the file
  uint16_t buffer;

  // Read two bytes at a time
  while (instructions.read(reinterpret_cast<char*>(&buffer), sizeof(buffer))) {
    std::string instructionString = processInstruction(buffer);
    WriteInstructionToAnFile(instructionString);
  }

  return 0;

}