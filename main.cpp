#include <iostream>
#include <iomanip>
#include <bitset>
#include "InstructionsHandlers.h"

std::array<uint16_t, 8> registers{0};
std::bitset<2> flags;
size_t prevIPCount{0U};
std::array<uint8_t,1024*1024> memory{0};

namespace
{
    const std::string OUTPUT_FILE_PATH = "../asm_files/output.asm";
    const std::string INPUT_FILE_PATH = "../asm_files/listing_0054_draw_rectangle";
    const std::string ASM_HEADER = "bits 16";
    const std::string DUMP_FILE_PATH = "../asm_files/memory.data";
}

std::string processInstruction(std::ifstream& bytesStream)
{
    std::array<uint8_t,6> fullInstructionBuffer{};
    // Buffer to hold the two bytes read from the file
    prevIPCount = static_cast<size_t>(bytesStream.tellg());
    if(!bytesStream.read(reinterpret_cast<char*>(&fullInstructionBuffer[0]), sizeof(fullInstructionBuffer[0])))
    {
        return "";
    };
    
    auto getStringFromInstruction = [&fullInstructionBuffer,&bytesStream](auto opcodeValue){
        
        auto opcodeEnum = static_cast<OP_CODE_VALUES>(opcodeValue);
        if(auto function = opCodesToFunc.find(opcodeEnum); function != opCodesToFunc.end())
        {
            return function->second(fullInstructionBuffer,bytesStream);
        }
        return std::string();
    };
    
    std::string result;
    // test for jumps
    auto opcode = fetchingFunc(INSTRUCTION_MASKS::OPCODE_8BITES,fullInstructionBuffer[0]);
    if(jumpOpCodeToStrMap.find(opcode) != jumpOpCodeToStrMap.end())
    {
       return handleJump(fullInstructionBuffer, bytesStream, opcode);
    }
    
    // check 4bit opcodes
    opcode = fetchingFunc(INSTRUCTION_MASKS::OPCODE_4BITES,fullInstructionBuffer[0]);
    if(opcodes4Bit.find(opcode) != opcodes4Bit.end())
    {
        result = getStringFromInstruction(opcode);
    }
    
    // check 5bit opcodes
    if(result.empty())
    {
        opcode = fetchingFunc(INSTRUCTION_MASKS::OPCODE_5BITES,fullInstructionBuffer[0]);
        if(opcodes5Bit.find(opcode) != opcodes5Bit.end())
        {
            result = getStringFromInstruction(opcode);
        }
    }
    else
    {
        return result;
    }
    
    // check 6bit opcodes
    if(result.empty())
    {
        opcode = fetchingFunc(INSTRUCTION_MASKS::OPCODE_6BITES,fullInstructionBuffer[0]);
        if(opcodes6Bit.find(opcode) != opcodes6Bit.end())
        {
            result = getStringFromInstruction(opcode);
        }
    }
    
    if(result.empty())
    {
        opcode = fetchingFunc(INSTRUCTION_MASKS::OPCODE_7BITES,fullInstructionBuffer[0]);
        if(opcodes7Bit.find(opcode) != opcodes7Bit.end())
        {
            result = getStringFromInstruction(opcode);
        }
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
  std::ofstream outFile(OUTPUT_FILE_PATH, std::ios::app);

  if (!outFile.is_open())
  {
    std::cerr << "Failed to open file for writing." << std::endl;
    return;
  }
  
  if(instruction.empty())
  {
      return;
  }
  
  outFile << instruction;
  
  outFile.close();
}

void printRegisters()
{
    std::ostringstream outputStream;
    auto index{0};
    outputStream << "Final registers:" << "\n";
    for(const auto& reg: registers)
    {
        if(reg != 0u)
        {
            std::ostringstream oss;
            oss << "0x" << std::hex << std::setw(4) << std::setfill('0') << reg;
            outputStream << "\t" << regNamesExtendedToStr[index] + ": " + oss.str() + "(" + std::to_string(reg) + ")" << "\n";
        }
        index++;
    }
    
    
    std::cout << outputStream.str();
}


void dumpMemoryToFile(const std::string& filePath)
{
  std::ofstream outFile(filePath, std::ios::binary);
  if (!outFile.is_open())
  {
    std::cerr << "Failed to open memory dump file for writing." << std::endl;
    return;
  }
  outFile.write(reinterpret_cast<const char*>(memory.data()), memory.size());
  outFile.close();
}

int main(int argc, char* argv[])
{
  bool dumpMemory = false;
  if (argc > 1 && std::strcmp(argv[1], "-dump") == 0)
  {
    dumpMemory = true;
  }

  std::ifstream  instructions(INPUT_FILE_PATH, std::ios::binary);

  if(!instructions)
  {
    std::cerr << "Got error on opening a file!" << std::endl;
    return -1;
  }
    
    std::ofstream outFile(OUTPUT_FILE_PATH, std::ios::out | std::ios::trunc);
    outFile << ASM_HEADER << std::endl;
    outFile << std::endl;
    outFile.close();
    
    std::cout << ASM_HEADER << "\n";
  
  while (!instructions.eof()) {
    std::string instructionStr = processInstruction(instructions);
    WriteInstructionToAnFile(instructionStr);
  }
  
  printRegisters();
  
  std::ostringstream hexVal;
  hexVal << "0x" << std::hex << std::setw(4) << std::setfill('0') << prevIPCount;
  std::cout << "\t" << "IP: " << hexVal.str() << "(" << prevIPCount <<")" << "\n";

  if (dumpMemory)
  {
    dumpMemoryToFile(DUMP_FILE_PATH);
  }

  return 0;

}