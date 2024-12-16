//
// Created by bordeax on 10/30/24.
//

#include <iostream>
#include <iomanip>
#include "InstructionsHandlers.h"
#include "InstructionSimulatorHandler.h"


namespace
{
std::map<std::string, size_t> regNameToIndex{
        {"ax", 0},
        {"cx", 1},
        {"dx", 2},
        {"bx", 3},
        {"sp", 4},
        {"bp", 5},
        {"si", 6},
        {"di", 7},
        {"al", 0},
        {"cl", 1},
        {"dl", 2},
        {"bl", 3},
        {"ah", 0},
        {"ch", 1},
        {"dh", 2},
        {"bh", 3}
};

}

uint8_t* calculateMemoryAddress(uint8_t regMemValue, uint8_t displacement)
{
    auto memoryAddress = &memory[0];
    if(regMemValue == 0b000)
    {
      auto address{registers[regNameToIndex["bx"]] + registers[regNameToIndex["si"]]};
      memoryAddress = &(memoryAddress[address + displacement]);
    }
    else if(regMemValue == 0b001)
    {
      auto address{registers[regNameToIndex["bx"]] + registers[regNameToIndex["di"]]};
        memoryAddress = &(memoryAddress[address + displacement]);
    }
    else if(regMemValue == 0b010)
    {
      auto address{registers[regNameToIndex["bp"]] + registers[regNameToIndex["si"]]};
        memoryAddress = &(memoryAddress[address + displacement]);
    }
    else if(regMemValue == 0b011)
    {
      auto address{registers[regNameToIndex["bp"]] + registers[regNameToIndex["di"]]};
        memoryAddress = &(memoryAddress[address + displacement]);
    }
    else if(regMemValue == 0b100)
    {
      auto address{registers[regNameToIndex["si"]]};
        memoryAddress = &(memoryAddress[address + displacement]);
    }
    else if(regMemValue == 0b101)
    {
      auto address{registers[regNameToIndex["di"]]};
        memoryAddress = &(memoryAddress[address + displacement]);
    }
    else if(regMemValue == 0b110)
    {
      auto address{registers[regNameToIndex["bp"]]};
        memoryAddress = &(memoryAddress[address + displacement]);
    }
    else if(regMemValue == 0b111)
    {
      auto address{registers[regNameToIndex["bx"]]};
      memoryAddress = &(memoryAddress[address + displacement]);
    }
  return memoryAddress;
}

uint8_t fetchingFunc(INSTRUCTION_MASKS dataMask,uint8_t buffer8Bit)
{
    uint8_t fetchedData = buffer8Bit & static_cast<uint8_t>(dataMask);
    fetchedData >>= shiftRight[dataMask];
    return fetchedData;
};


std::array<uint16_t,5> fetchingRegMemData(std::array<uint8_t, 6>& buffer)
{
    // Fetching W
    uint16_t wValue = fetchingFunc(INSTRUCTION_MASKS::W_6BITES,buffer[0]);
    
    // Fetching D
    uint16_t dValue = fetchingFunc(INSTRUCTION_MASKS::D, buffer[0]);
    
    //Fetching Mode
    uint16_t mod = fetchingFunc(INSTRUCTION_MASKS::MOD, buffer[1]);
    
    // Fetching Reg
    uint16_t regValue = fetchingFunc(INSTRUCTION_MASKS::REG_6BITES_OP_CODE, buffer[1]);
    
    // Fetching REG_MEM
    uint16_t regMemValue = fetchingFunc(INSTRUCTION_MASKS::REG_MEM, buffer[1]);
    return {wValue,dValue,mod,regValue,regMemValue};
}

std::string getIPCountString(std::ifstream& bytestream)
{
    std::ostringstream ipCountString;
    ipCountString << " IP: " << "0x" << std::hex << prevIPCount << "->" << "0x" << std::hex << bytestream.tellg();
    return ipCountString.str();
}

void printInstructionAndChange(const std::string &instructionStr, const std::string& regName, uint16_t oldValue,
                               uint16_t newValue, std::ifstream& bytestream)
{
    std::ostringstream oss, ossNew, flagsOSS,ipCountString;
    
    oss << "0x" << std::hex << std::setw(4) << std::setfill('0') << oldValue;
    ossNew << "0x" << std::hex  << std::setw(4) << std::setfill('0') << newValue;
    
    if(flags.any())
    {
        flagsOSS << " flags -> ";
        if(flags.test(ZERO))
        {
            flagsOSS <<  "Z";
        }
        
        if(flags.test(SIGN))
        {
            flagsOSS << "S";
        }
        
    }
    
    std::cout << instructionStr + "; " + regName + ":" + oss.str() + "->" + ossNew.str() + flagsOSS.str() + getIPCountString(bytestream) + "\n";
}

std::string handleRegMemModValues(std::array<uint16_t,5> prefetchedValues, const std::string& instructionString,
                                  std::ifstream& bytesStream, std::array<uint8_t, 6>& buffer,
                                  std::function<void(uint16_t&, uint16_t, uint16_t)> execFunc = nullptr)
{
    auto [wValue,dValue,mod,regValue,regMemValue] = prefetchedValues;
    std::stringstream ss;
    
    if(mod == 0b11)
    {
        auto regString = (wValue) ? regNamesExtendedToStr[regValue] : regNamesToStr[regValue];
        
        auto regMemValueString = (wValue) ? regNamesExtendedToStr[regMemValue] : regNamesToStr[regMemValue];
        ss << instructionString << " " << regMemValueString  << "," << " " << regString;
        
        if(execFunc)
        {
            uint16_t oldRegValue{registers[regNameToIndex[regMemValueString]]};
            
            if(wValue)
            {
                execFunc(registers[regMemValue], registers[regValue], 0x0000);
            }
            else
            {
                auto mask = (regMemValue > 3) ? 0xFF00 : 0x00FF;
                
                auto  regData = registers[regValue % 4];
                if(regValue > 3)
                {
                    regData >>=8;
                }
                else
                {
                    regData &= 0x00FF;
                }
                
                execFunc(registers[regMemValue % 4], regData, mask);
            }
            printInstructionAndChange(ss.str(), regMemValueString, oldRegValue,
                                      registers[regNameToIndex[regMemValueString]], bytesStream);
        }
    }
    else if (mod == 0b01)
    {
        auto regMemValueString = (wValue) ? regNamesExtendedToStr[regValue] : regNamesToStr[regValue];
        auto calculationAddress = regNamesPlus[regMemValue];
        
        bytesStream.read(reinterpret_cast<char*>(&buffer[2]), sizeof(buffer[2]));
        uint8_t data = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[2]);
        
        std::stringstream displacementValue;
        if(data != 0u)
        {
            displacementValue << "[" << calculationAddress << " + " << std::to_string(static_cast<int16_t>(data)) << "]";
        }
        else
        {
            displacementValue << "[" << calculationAddress << "]";
        }
        
        ss << instructionString << " "
           << ((dValue == 1u) ? regMemValueString : displacementValue.str())  << ", "
           << ((dValue == 1u) ? displacementValue.str() : regMemValueString);
        
    }
    else if(mod == 0b00)
    {
      uint16_t directAddress{};
      if(regMemValue == 0b110)
      {
        bytesStream.read(reinterpret_cast<char*>(&buffer[2]), sizeof(buffer[2]));
        bytesStream.read(reinterpret_cast<char*>(&buffer[3]), sizeof(buffer[3]));
        directAddress = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[2]);
        uint16_t highBitDirectAddress = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[3]);
        highBitDirectAddress <<= 8;
        directAddress ^= highBitDirectAddress;
      }

      auto regMemValueString = (wValue) ? regNamesExtendedToStr[regValue] : regNamesToStr[regValue];
      auto calculationAddress = (regMemValue != 0b110) ? regNamesPlus[regMemValue] : std::to_string(directAddress);
      std::string displacementValue = "[" + calculationAddress + "]";
      ss << instructionString << " "
         << ((dValue == 1u) ? regMemValueString: displacementValue) << ", "
         << ((dValue == 1u) ? displacementValue: regMemValueString);


      if(execFunc)
      {
        uint16_t oldRegValue{registers[regNameToIndex[regMemValueString]]};

        uint16_t dataFromMemory{};
        if(wValue && regMemValue == 0b110)
        {
          dataFromMemory = memory[directAddress];
          dataFromMemory |= (memory[directAddress + 1] << 8);
        }
        else if(wValue && regMemValue != 0b110)
        {
          auto* memoryPointer = calculateMemoryAddress(regMemValue, 0);
          dataFromMemory = memoryPointer[0];
          dataFromMemory |= (memoryPointer[1] << 8);
        }
        else if(!wValue && regMemValue == 0b110)
        {
          dataFromMemory = memory[directAddress];
        }
        else if(!wValue && regMemValue != 0b110)
        {
          auto* memoryPointer = calculateMemoryAddress(regMemValue, 0);
          dataFromMemory = memoryPointer[0];
        }
        
        uint16_t dataToSave{};
        execFunc((dValue == 1u) ? registers[regValue] : dataToSave , (dValue == 1u) ? dataFromMemory : registers[regValue]  , 0xFFFF);
        
        if(dValue == 0u)
        {
            auto* memoryPointer = calculateMemoryAddress(regMemValue, 0);
            memoryPointer[0] = dataToSave;
            memoryPointer[1] = dataToSave >> 8;
            std::cout << ss.str() + getIPCountString(bytesStream) + "\n";
            
        }
        else
        {
        printInstructionAndChange(ss.str(), regMemValueString, oldRegValue,
                                  registers[regNameToIndex[regMemValueString]], bytesStream);
        }
      }
    }
    else if(mod == 0b10)
    {
        auto regMemValueString = (wValue) ? regNamesExtendedToStr[regMemValue] : regNamesToStr[regMemValue];
        auto calculationAddress = regNamesPlus[regMemValue];
        
        bytesStream.read(reinterpret_cast<char*>(&buffer[2]), sizeof(buffer[2]));
        bytesStream.read(reinterpret_cast<char*>(&buffer[3]), sizeof(buffer[3]));
        uint16_t data = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[2]);
        uint16_t dataHigh = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[3]);
        dataHigh <<= 8;
        data ^= dataHigh;
        
        
        std::stringstream displacementValue;
        if(data != 0u)
        {
            displacementValue << "[" << calculationAddress << " + " << std::to_string(static_cast<int16_t>(data)) << "]";
        }
        else
        {
            displacementValue << "[" << calculationAddress << "]";
        }
        
        ss << instructionString << " "
           << ((dValue == 1u)? regMemValueString : displacementValue.str())  << ","
           << ((dValue == 1u) ? displacementValue.str() : regMemValueString);

      if(execFunc)
      {
        uint16_t oldRegValue{registers[regNameToIndex[regMemValueString]]};

        uint16_t dataFromMemory{};
        if(wValue)
        {
            auto* memoryPointer = calculateMemoryAddress(regMemValue, data);
            dataFromMemory = memoryPointer[0];
            dataFromMemory |= (memoryPointer[1] << 8);
        }
        else
        {
          auto* memoryPointer = calculateMemoryAddress(regMemValue, data);
          dataFromMemory = memoryPointer[0];
        }

        execFunc(registers[regValue], dataFromMemory, 0xFFFF);

        printInstructionAndChange(ss.str(), regMemValueString, oldRegValue,
                                  registers[regNameToIndex[regMemValueString]], bytesStream);
      }
    }
    
    ss << "\n";
    return ss.str();
}

std::string handleImmediateToRegister(std::array<uint8_t, 6> &buffer, std::ifstream &bytesStream,
                                      const std::string &instructionType,
                                      std::function<void(uint16_t&, uint16_t, uint16_t)> execFunc = nullptr)
{
    uint8_t wValue = fetchingFunc(INSTRUCTION_MASKS::W_4BITES,buffer[0]);
    if(wValue == 1u)
    {
        bytesStream.read(reinterpret_cast<char*>(&buffer[2]), sizeof(buffer[2]));
    }
    uint8_t regValue = fetchingFunc(INSTRUCTION_MASKS::REG_4BITES_OP_CODE, buffer[0]);
    uint16_t  data = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[1]);
    if(wValue == 1u)
    {
        uint16_t highBitData = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[2]);
        highBitData <<=8;
        data ^= highBitData;
    }
    
    auto regString = (wValue) ? regNamesExtendedToStr[regValue] : regNamesToStr[regValue];
    auto valueString = std::to_string(static_cast<int16_t>(data));
    std::stringstream ss;
    ss << instructionType << " " << regString  << "," << " " << valueString;
    
    if(execFunc)
    {
        uint16_t oldRegValue{registers[regNameToIndex[regString]]};
        
        if(wValue)
        {
            
            execFunc(registers[regValue], data, 0x0000);
        }
        else
        {
            auto mask = (regValue > 3) ? 0xFF00 : 0x00FF;
            execFunc(registers[regValue % 4], data, mask);
        }
        
        printInstructionAndChange(ss.str(), regString, oldRegValue, registers[regNameToIndex[regString]],
                                  bytesStream);
    }
    ss << std::endl;
    return ss.str();
}

std::string handleImmediateToMemoryBitOp(std::array<uint8_t, 6> &buffer, std::ifstream &bytesStream,
                                         const std::string &instructionType,
                                         std::function<void(uint16_t &, uint16_t, uint16_t)> execFunc = nullptr)
{
  // Fetching W
    auto wValue = fetchingFunc(INSTRUCTION_MASKS::W_6BITES, buffer[0]);

  //Fetching Mode
  uint16_t mod = fetchingFunc(INSTRUCTION_MASKS::MOD, buffer[1]);

  // Fetching REG_MEM
  uint16_t regMemValue = fetchingFunc(INSTRUCTION_MASKS::REG_MEM, buffer[1]);

  std::stringstream ss;
  if(mod==0b11)
  {
    auto regMemValueString = (wValue) ? regNamesExtendedToStr[regMemValue] : regNamesToStr[regMemValue];
    bytesStream.read(reinterpret_cast<char*>(&buffer[2]), sizeof(buffer[2]));
    uint16_t data = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[2]);

    if(wValue == 1)
    {
      bytesStream.read(reinterpret_cast<char*>(&buffer[3]), sizeof(buffer[3]));
      uint16_t highBitData = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[3]);
      highBitData <<= 8;
      data ^= highBitData;
    }

    ss << instructionType << " " << "[" + regMemValueString +"]" << "," << " " << std::to_string(static_cast<int16_t>(data));

    if(execFunc)
    {
      uint16_t oldRegValue{registers[regNameToIndex[regMemValueString]]};

      if(wValue)
      {
        execFunc(registers[regMemValue], data, 0xFFFF);
      }
      else
      {
        auto mask = (regMemValue > 3) ? 0xFF00 : 0x00FF;

        auto  regData = data & mask;
        if(regMemValue > 3)
        {
          regData >>= 8;
        }
        else
        {
          regData &= 0x00FF;
        }

        execFunc(registers[regMemValue % 4], regData, mask);
      }

      printInstructionAndChange(ss.str(), regMemValueString, oldRegValue,
                                registers[regNameToIndex[regMemValueString]], bytesStream);
    }
  }
  else if(mod==0b00 & regMemValue != 0b110)
  {
    bytesStream.read(reinterpret_cast<char*>(&buffer[2]), sizeof(buffer[2]));
    uint16_t data = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[2]);

    if(wValue == 1)
    {
      bytesStream.read(reinterpret_cast<char*>(&buffer[3]), sizeof(buffer[3]));
      uint16_t highBitData = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[3]);
      highBitData <<= 8;
      data ^= highBitData;
    }
    ss << instructionType << " " << "[" + regNamesPlus[regMemValue] + "]" << ", " << std::to_string(static_cast<int16_t>(data));
    if(execFunc)
    {
      uint16_t bufferValue{};
      if (wValue)
      {
        execFunc(bufferValue, data, 0xFFFF);
      } else {
        auto mask = (regMemValue > 3) ? 0xFF00 : 0x00FF;

        auto regData = data & mask;
        if (regMemValue > 3) {
          regData >>= 8;
        } else {
          regData &= 0x00FF;
        }

        execFunc(bufferValue, regData, mask);
      }
        auto* memAddr = calculateMemoryAddress(regMemValue, 0);
        memAddr[0] = static_cast<uint8_t>(bufferValue & 0xFF);
        memAddr[0] = static_cast<uint8_t>((bufferValue >> 8) & 0xFF);
        std::cout << ss.str() + getIPCountString(bytesStream) + "\n";
    }
  }
  else if(mod==0b01)
  {

    bytesStream.read(reinterpret_cast<char*>(&buffer[2]), sizeof(buffer[2]));
    uint16_t dataDisp = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[2]);
    bytesStream.read(reinterpret_cast<char*>(&buffer[3]), sizeof(buffer[3]));
    uint16_t data = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[3]);
    if(wValue == 1)
    {
      bytesStream.read(reinterpret_cast<char*>(&buffer[4]), sizeof(buffer[4]));
      uint16_t highBitData = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[4]);
      highBitData <<= 8;
      data ^= highBitData;
    }

    auto displacement = (dataDisp != 0u) ? std::to_string(static_cast<int16_t>(dataDisp)) : "";
    ss << instructionType << " " << regNamesPlus[regMemValue] + "+" + displacement
       << ", " << std::to_string(static_cast<int16_t>(data));

    if(execFunc)
    {
      uint16_t bufferValue{};
      if(wValue)
      {
        execFunc(bufferValue, data, 0xFFFF);
      }
      else
      {
        auto mask = (regMemValue > 3) ? 0xFF00 : 0x00FF;

        auto regData = data & mask;
        if(regMemValue > 3)
        {
          regData >>= 8;
        }
        else
        {
          regData &= 0x00FF;
        }

        execFunc(bufferValue, regData, mask);
      }

      auto* memAddr = calculateMemoryAddress(regMemValue, dataDisp);
      memAddr[0] = static_cast<uint8_t>(bufferValue & 0xFF);
      if(wValue)
      {
        memAddr[1] = static_cast<uint8_t>((bufferValue >> 8) & 0xFF);
      }
      std::cout << ss.str() + getIPCountString(bytesStream) + "\n";
    }

  }
  else if (mod==0b00 && regMemValue == 0b110)
  {
    bytesStream.read(reinterpret_cast<char*>(&buffer[2]), sizeof(buffer[2]));
    bytesStream.read(reinterpret_cast<char*>(&buffer[3]), sizeof(buffer[3]));

    uint16_t dataDisp = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[2]);
    uint16_t highBitDataDisp = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[3]);
    highBitDataDisp <<= 8;
    dataDisp ^= highBitDataDisp;

    bytesStream.read(reinterpret_cast<char*>(&buffer[4]), sizeof(buffer[4]));
    uint16_t data = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[4]);

    if(wValue == 1)
    {
      bytesStream.read(reinterpret_cast<char*>(&buffer[5]), sizeof(buffer[5]));
      uint16_t highBitData = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[5]);
      highBitData <<= 8;
      data ^= highBitData;
    }
    ss << instructionType << " " << std::to_string(static_cast<int16_t>(dataDisp)) << "," << " " << std::to_string(static_cast<int16_t>(data));
    if(execFunc)
    {
      uint16_t bufferValue{};
      if(wValue)
      {

        execFunc(bufferValue, data, 0xFFFF);
      }
      else
      {
        auto mask = (regMemValue > 3) ? 0xFF00 : 0x00FF;

        auto  regData = data & mask;
        if(regMemValue > 3)
        {
          regData >>= 8;
        }
        else
        {
          regData &= 0x00FF;
        }

        execFunc(registers[regMemValue % 4], regData, mask);
      }

      memory[dataDisp] = static_cast<uint8_t>(bufferValue & 0xFF);
      memory[dataDisp + 1] = static_cast<uint8_t>((bufferValue >> 8) & 0xFF);

      std::cout << ss.str() + getIPCountString(bytesStream) + "\n";
    }
  } else if(mod==0b10)
  {
    bytesStream.read(reinterpret_cast<char*>(&buffer[2]), sizeof(buffer[2]));
    bytesStream.read(reinterpret_cast<char*>(&buffer[3]), sizeof(buffer[3]));
    uint16_t dataDisp = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[2]);
    uint16_t dataHigh = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[3]);
    dataHigh <<= 8;
    dataDisp ^= dataHigh;

    bytesStream.read(reinterpret_cast<char*>(&buffer[4]), sizeof(buffer[4]));
    uint16_t data = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[4]);
    if(wValue == 1)
    {
      bytesStream.read(reinterpret_cast<char*>(&buffer[5]), sizeof(buffer[5]));
      uint16_t highBitData = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[5]);
      highBitData <<= 8;
      data ^= highBitData;
    }
    auto displacement = (dataDisp != 0u) ? std::to_string(static_cast<int16_t>(dataDisp)) : "";
    ss << instructionType << " " << regNamesPlus[regMemValue] << "+"<< displacement << "," << " " << std::to_string(static_cast<int16_t>(data));
    std::cout << ss.str() + getIPCountString(bytesStream) + "\n";
  }

  ss << "\n";
  return ss.str();

}

std::string handleImmediateToRegisterLogicOp(std::array<uint8_t, 6> &buffer, std::ifstream &bytesStream,
                                             const std::string &instructionType,
                                             std::function<void(uint16_t &, uint16_t, uint16_t)> execFunc = nullptr)
{
    auto [wValue,sValue,mod,regValue,regMemValue] = fetchingRegMemData(buffer);
    
    std::stringstream ss;
    
    if(mod == 0b11)
    {
        auto regMemValueString = (wValue) ? regNamesExtendedToStr[regMemValue] : regNamesToStr[regMemValue];
        bytesStream.read(reinterpret_cast<char*>(&buffer[2]), sizeof(buffer[2]));
        uint16_t data = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[2]);
        
        if(sValue == 0 && wValue == 1)
        {
            bytesStream.read(reinterpret_cast<char*>(&buffer[3]), sizeof(buffer[3]));
            uint16_t highBitData = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[3]);
            highBitData <<= 8;
            data ^= highBitData;
        }
        
        ss << instructionType << " " << regMemValueString  << "," << " " << std::to_string(static_cast<int16_t>(data));
        
        if(execFunc)
        {
            uint16_t oldRegValue{registers[regNameToIndex[regMemValueString]]};
            
            if(wValue)
            {
                execFunc(registers[regMemValue], data, 0xFFFF);
            }
            else
            {
                auto mask = (regMemValue > 3) ? 0xFF00 : 0x00FF;
                
                auto  regData = data & mask;
                if(regValue > 3)
                {
                    regData >>= 8;
                }
                else
                {
                    regData &= 0x00FF;
                }
                
                execFunc(registers[regMemValue % 4], regData, mask);
            }
            
            printInstructionAndChange(ss.str(), regMemValueString, oldRegValue,
                                      registers[regNameToIndex[regMemValueString]], bytesStream);
        }
    }
    else if (mod == 0b01)
    {
        auto regMemValueString = (wValue) ? regNamesExtendedToStr[regValue] : regNamesToStr[regValue];
        auto calculationAddress = regNamesPlus[regMemValue];
        
        bytesStream.read(reinterpret_cast<char*>(&buffer[2]), sizeof(buffer[2]));
        uint16_t data = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[2]);
        
        if(sValue == 0 && wValue == 1)
        {
            bytesStream.read(reinterpret_cast<char*>(&buffer[3]), sizeof(buffer[3]));
            uint8_t lowBitData = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[3]);
            data <<= 8;
            data ^= lowBitData;
        }
        
        std::stringstream displacementValue;
        if(data != 0u)
        {
            displacementValue << "[" << calculationAddress << " + " << std::to_string(static_cast<int16_t>(data)) << "]";
        }
        else
        {
            displacementValue << "[" << calculationAddress << "]";
        }
        
        ss << instructionType << " " << ((sValue == 1u) ? regMemValueString : displacementValue.str()) << ", "
           << std::to_string(static_cast<int16_t>(data));
    }
    else if(mod == 0b00)
    {
        
        auto calculationAddress = regNamesPlus[regMemValue];
        
        bytesStream.read(reinterpret_cast<char*>(&buffer[2]), sizeof(buffer[2]));
        uint16_t data = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[2]);
        if(sValue == 0 && wValue == 1)
        {
            bytesStream.read(reinterpret_cast<char*>(&buffer[3]), sizeof(buffer[3]));
            uint16_t lowBitData = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[3]);
            lowBitData <<= 8;
            data ^= lowBitData;
        }
        
        
        ss << instructionType << " " << calculationAddress << ", "
           << ((sValue == 0 && wValue == 1) ? std::to_string(static_cast<int16_t>(data)) : std::to_string(static_cast<int8_t>(data)));
        
    }
    else if(mod == 0b10)
    {
        auto calculationAddress = regNamesPlus[regMemValue];
        
        bytesStream.read(reinterpret_cast<char*>(&buffer[2]), sizeof(buffer[2]));
        bytesStream.read(reinterpret_cast<char*>(&buffer[3]), sizeof(buffer[3]));
        uint16_t dataDisp = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[2]);
        uint16_t dataHigh = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[3]);
        dataHigh <<= 8;
        dataDisp ^= dataHigh;
        
        bytesStream.read(reinterpret_cast<char*>(&buffer[4]), sizeof(buffer[4]));
        uint16_t data = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[4]);
        if(sValue == 0 && wValue == 1)
        {
            bytesStream.read(reinterpret_cast<char*>(&buffer[5]), sizeof(buffer[5]));
            uint8_t lowBitData = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[5]);
            data <<= 8;
            data ^= lowBitData;
        }
        
        std::stringstream displacementValue;
        if(dataDisp != 0u)
        {
            displacementValue << "[" << calculationAddress << " + " << std::to_string(static_cast<int16_t>(dataDisp)) << "]";
        }
        else
        {
            displacementValue << "[" << calculationAddress << "]";
        }
        
        ss << instructionType << " "
           << displacementValue.str() << ","
           << ((sValue == 0 && wValue == 1) ? std::to_string(static_cast<int16_t>(data)) : std::to_string(static_cast<int8_t>(data)));
    }
    
    if(!ss.str().empty())
    {
        ss << "\n";
    }
    
    return ss.str();
}

std::string handleImmediateToAccOpLogic(std::array<uint8_t, 6> &buffer, std::ifstream &bytesStream,
                                        const std::string &instructionType,
                                        std::function<void(uint16_t &, uint16_t, uint16_t)> execFunc = nullptr)
{
    auto wValue = fetchingFunc(INSTRUCTION_MASKS::W_6BITES, buffer[0]);
    uint16_t data = buffer[1];
    
    std::stringstream instructionString;
    instructionString << instructionType << " ";
    if(wValue == 1)
    {
        bytesStream.read(reinterpret_cast<char*>(&buffer[2]), sizeof(buffer[2]));
        uint16_t lowBitData = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[2]);
        lowBitData <<= 8;
        data ^= lowBitData;
        instructionString << "ax, ";
    }
    else
    {
        instructionString << "al, ";
    }
    
    instructionString << std::to_string(static_cast<int8_t>(data));
    
    if(execFunc)
    {
        uint16_t oldRegValue{registers[0]};
        auto mask = (wValue)? 0xFFFF : 0x00FF;
        execFunc(registers[0], data & mask, mask);
        printInstructionAndChange(instructionString.str(), (wValue) ? "ax" : "al", oldRegValue, registers[0],
                                  bytesStream);
    }
    
    return instructionString.str();
}

/// Handlers functions

std::string handleMovImmediateToRegister(std::array<uint8_t, 6>& buffer, std::ifstream& bytesStream)
{
    bytesStream.read(reinterpret_cast<char*>(&buffer[1]), sizeof(buffer[1]));
    return handleImmediateToRegister(buffer, bytesStream, OpCodeToString(OP_CODE_VALUES::MOV_IMMEDIATE), MoveExecute);
}

std::string handleAddRegMemInstruction(std::array<uint8_t, 6>& buffer, std::ifstream& bytesStream)
{
    bytesStream.read(reinterpret_cast<char*>(&buffer[1]), sizeof(buffer[1]));
    auto fetchedValues = fetchingRegMemData(buffer);
    return handleRegMemModValues(fetchedValues,OpCodeToString(OP_CODE_VALUES::ADD_REG_MEM),bytesStream, buffer, AddExecute);
}

std::string handleAddImmediateToAccumulator(std::array<uint8_t, 6>& buffer, std::ifstream& bytesStream)
{
    bytesStream.read(reinterpret_cast<char*>(&buffer[1]), sizeof(buffer[1]));
    return handleImmediateToAccOpLogic(buffer, bytesStream, OpCodeToString(OP_CODE_VALUES::ADD_IMMEDIATE_ACCUMULATOR),
                                       AddExecute);
}

std::string handleOpLogicImmediateToRegMem(std::array<uint8_t, 6> &buffer, std::ifstream &bytesStream)
{
    static std::map<uint8_t, std::string> opTypesToStr = {
            {0b000, "add"},
            {0b101, "sub"},
            {0b111, "cmp"}
    };
    
    static std::map<uint8_t, std::function<void(uint16_t&, uint16_t, uint16_t)>> opTypesToExecution = {
            {0b000, AddExecute},
            {0b101, SubExecute},
            {0b111, CompExecute}
    };
    
    bytesStream.read(reinterpret_cast<char*>(&buffer[1]), sizeof(buffer[1]));
    auto opType = fetchingFunc(INSTRUCTION_MASKS::REG_6BITES_OP_CODE, buffer[1]);
    
    return handleImmediateToRegisterLogicOp(buffer, bytesStream, opTypesToStr[opType], opTypesToExecution[opType]);
}

std::string handleSubImmediateToAccumulator(std::array<uint8_t, 6> &buffer, std::ifstream &bytesStream)
{
    bytesStream.read(reinterpret_cast<char*>(&buffer[1]), sizeof(buffer[1]));
    return handleImmediateToAccOpLogic(buffer, bytesStream, OpCodeToString(OP_CODE_VALUES::SUB_IMMEDIATE_ACCUMULATOR),
                                       SubExecute);
}

std::string handleSubRegMemInstruction(std::array<uint8_t, 6> &buffer, std::ifstream &bytesStream)
{
    bytesStream.read(reinterpret_cast<char*>(&buffer[1]), sizeof(buffer[1]));
    auto fetchedValues = fetchingRegMemData(buffer);
    return handleRegMemModValues(fetchedValues,OpCodeToString(OP_CODE_VALUES::SUB_REG_MEM),bytesStream, buffer,SubExecute);
}

std::string handleCmpImmediateToAccumulator(std::array<uint8_t, 6> &buffer, std::ifstream &bytesStream)
{
    bytesStream.read(reinterpret_cast<char*>(&buffer[1]), sizeof(buffer[1]));
    return handleImmediateToAccOpLogic(buffer, bytesStream, OpCodeToString(OP_CODE_VALUES::CMP_IMMEDIATE_ACCUMULATOR),
                                       CompExecute);
}

std::string handleCmpRegMemInstruction(std::array<uint8_t, 6> &buffer, std::ifstream &bytesStream)
{
    bytesStream.read(reinterpret_cast<char*>(&buffer[1]), sizeof(buffer[1]));
    auto fetchedValues = fetchingRegMemData(buffer);
    return handleRegMemModValues(fetchedValues,OpCodeToString(OP_CODE_VALUES::CMP_REG_MEM),bytesStream, buffer, CompExecute);
}

std::string handleJump(std::array<uint8_t, 6>& buffer, std::ifstream& bytesStream, uint8_t jumpOpCode) {
  bytesStream.read(reinterpret_cast<char*>(&buffer[1]), sizeof(buffer[1]));
  
  std::stringstream instructionString;
  instructionString << jumpOpCodeToStrMap[jumpOpCode] << " " << std::to_string(static_cast<int8_t>(buffer[1]));
  
  JumpExecute(bytesStream,static_cast<int8_t>(buffer[1]), jumpOpCode);
  
  auto ipCountString{getIPCountString(bytesStream)};
  std::cout << instructionString.str() << ipCountString << "\n";
  
  instructionString << "\n";
  return instructionString.str();
}

std::string handleMovRegMemInstruction(std::array<uint8_t, 6>& buffer, std::ifstream& bytesStream)
{
    bytesStream.read(reinterpret_cast<char*>(&buffer[1]), sizeof(buffer[1]));
    auto fetchedValues = fetchingRegMemData(buffer);
    return handleRegMemModValues(fetchedValues,OpCodeToString(OP_CODE_VALUES::MOV_REG_MEM),bytesStream, buffer, MoveExecute);
}


std::string handleMovImmediateToMemory(std::array<uint8_t, 6> &buffer, std::ifstream &bytesStream) {
    bytesStream.read(reinterpret_cast<char*>(&buffer[1]), sizeof(buffer[1]));
  return handleImmediateToMemoryBitOp(buffer, bytesStream, OpCodeToString(OP_CODE_VALUES::MOV_IMMEDIATE_TO_MEMORY),
                                     MoveExecute);
}