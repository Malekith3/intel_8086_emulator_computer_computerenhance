#include <iostream>
#include <fstream>
#include <map>
#include <array>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <functional>

enum class INSTRUCTION_MASKS : uint8_t
{
  OPCODE_6BITES = 0b11111100,
  W_6BITES = 0x01,
  W_4BITES = 0b00001000,
  D = 0x02,
  MOD = 0b11000000,
  REG_6BITES_OP_CODE = 0b00111000,
  REG_4BITES_OP_CODE = 0b00000111,
  REG_MEM = 0b000000111,
  OPCODE_4BITES = 0b11110000,
  OPCODE_5BITES = 0b11111000
};

enum class OP_CODE_VALUES : uint8_t
{
  MOV_REG_MEM = 0b00100010,
  MOV_IMMEDIATE = 0b00001011
};

std::map<INSTRUCTION_MASKS,int> shiftRight{
    {INSTRUCTION_MASKS::OPCODE_6BITES,     2},
    {INSTRUCTION_MASKS::W_6BITES,          0},
    {INSTRUCTION_MASKS::W_4BITES,          3},
    {INSTRUCTION_MASKS::D,                 1},
    {INSTRUCTION_MASKS::MOD,               6},
    {INSTRUCTION_MASKS::REG_6BITES_OP_CODE,3},
    {INSTRUCTION_MASKS::REG_4BITES_OP_CODE,0},
    {INSTRUCTION_MASKS::REG_MEM,           0},
    {INSTRUCTION_MASKS::OPCODE_4BITES,     4},
    {INSTRUCTION_MASKS::OPCODE_5BITES,     5}

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
      {OP_CODE_VALUES::MOV_REG_MEM, "mov"},
      {OP_CODE_VALUES::MOV_IMMEDIATE, "mov"}
  };

  auto it = opCodesToString.find(opCode);
  return (it != opCodesToString.end()) ? it->second : "";

}


auto fetchingFunc = [](auto dataMask,auto buffer8Bit)
{
    uint8_t fetchedData = buffer8Bit & static_cast<uint8_t>(dataMask);
    fetchedData >>= shiftRight[dataMask];
    return fetchedData;
};


std::string handleMovRegMemInstruction(std::array<uint8_t, 6>& buffer, std::ifstream& bytesStream)
{
    bytesStream.read(reinterpret_cast<char*>(&buffer[1]), sizeof(buffer[1]));
    
    // Fetching W
    uint16_t wValue = fetchingFunc(INSTRUCTION_MASKS::W_6BITES,buffer[0]);
    
//    // Fetching D
//    uint16_t dValue = fetchingFunc(INSTRUCTION_MASKS::D, buffer[0]);
    
    //Fetching Mode
    uint16_t mod = fetchingFunc(INSTRUCTION_MASKS::MOD, buffer[1]);
    
    // Fetching Reg
    uint16_t regValue = fetchingFunc(INSTRUCTION_MASKS::REG_6BITES_OP_CODE, buffer[1]);
    
    // Fetching REG_MEM
    uint16_t regMemValue = fetchingFunc(INSTRUCTION_MASKS::REG_MEM, buffer[1]);
    
    if(mod != 0b11)
    {
        std::cerr << "Dont support mod that is not 0b11";
    }
    std::stringstream ss;
    auto regString = (wValue) ? regNamesExtendedToStr[regValue] : regNamesToStr[regValue];
    
    auto regMemValueString = (wValue) ? regNamesExtendedToStr[regMemValue] : regNamesToStr[regMemValue];
    
    ss << OpCodeToString(OP_CODE_VALUES::MOV_REG_MEM) << " " << regMemValueString  << "," << " " << regString << std::endl;
    return ss.str();
    
}

std::unordered_map<OP_CODE_VALUES,std::function<std::string(std::array<uint8_t, 6>&,std::ifstream&)>> opCodesToFunc = {
        {OP_CODE_VALUES::MOV_REG_MEM, handleMovRegMemInstruction}
};

std::string processInstruction(std::ifstream& bytesStream)
{
    
    std::array<uint8_t,6> fullInstructionBuffer{};
    // Buffer to hold the two bytes read from the file
    bytesStream.read(reinterpret_cast<char*>(&fullInstructionBuffer[0]), sizeof(fullInstructionBuffer[0]));
    
    auto getStringFromInstruction = [&fullInstructionBuffer,&bytesStream](auto opcodeMask){
        
        uint8_t opcode = fetchingFunc(opcodeMask,fullInstructionBuffer[0]);
        auto opcodeEnum = static_cast<OP_CODE_VALUES>(opcode);
        if(auto function = opCodesToFunc.find(opcodeEnum); function != opCodesToFunc.end())
        {
            return function->second(fullInstructionBuffer,bytesStream);
        }
        return std::string();
    };
    
    
    // check 4bit opcodes
    std::string result = getStringFromInstruction(INSTRUCTION_MASKS::OPCODE_4BITES);
    // check 5bit opcodes
    if(result.empty())
    {
        result = getStringFromInstruction(INSTRUCTION_MASKS::OPCODE_5BITES);
    }
    else
    {
        return result;
    }
    
    // check 6bit opcodes
    if(result.empty())
    {
        result = getStringFromInstruction(INSTRUCTION_MASKS::OPCODE_6BITES);
    }
    
    return result;
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
  
  // Read two bytes at a time
  while (!instructions.eof()) {
    std::string instructionStr = processInstruction(instructions);
      ///Decide if you need to fetch more.
    WriteInstructionToAnFile(instructionStr);
  }

  return 0;

}