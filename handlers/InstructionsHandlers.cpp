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

void printInstructionAndChange(const std::string& instructionStr,const std::string& regName, uint16_t oldValue, uint16_t newValue)
{
    std::ostringstream oss, ossNew;
    oss << "0x" << std::hex << std::setw(4) << std::setfill('0') << oldValue;
    ossNew << "0x" << std::hex  << std::setw(4) << std::setfill('0') << newValue;
    
    std::cout << instructionStr + "; " + regName + ":" + oss.str() + " ->" + ossNew.str() + "\n";
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
            
            printInstructionAndChange(ss.str(),regMemValueString,oldRegValue,registers[regNameToIndex[regMemValueString]]);
        }
        
        ss << std::endl;
        return ss.str();
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
           << ((dValue == 1u) ? displacementValue.str() : regMemValueString)  << std::endl;;
        
        return ss.str();
    }
    else if(mod == 0b00)
    {
        
        auto regMemValueString = (wValue) ? regNamesExtendedToStr[regValue] : regNamesToStr[regValue];
        auto calculationAddress = regNamesPlus[regMemValue];
        std::string displacementValue = "[" + calculationAddress + "]";
        ss << instructionString << " "
           << ((dValue == 1u) ? regMemValueString: displacementValue) << ", "
           << ((dValue == 1u) ? displacementValue: regMemValueString) << std::endl;
        
        return ss.str();
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
           << ((dValue == 1u) ? displacementValue.str() : regMemValueString) << std::endl;;
        
        
        return ss.str();
    }
    
    return "";
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
        
        printInstructionAndChange(ss.str(),regString,oldRegValue,registers[regNameToIndex[regString]]);
    }
    ss << std::endl;
    return ss.str();
}

std::string handleImmediateToRegisterLogicOp(std::array<uint8_t, 6>& buffer, std::ifstream& bytesStream, const std::string& instructionType)
{
    auto [wValue,sValue,mod,regValue,regMemValue] = fetchingRegMemData(buffer);
    
    std::stringstream ss;
    
    if(mod == 0b11)
    {
        auto regMemValueString = (wValue) ? regNamesExtendedToStr[regMemValue] : regNamesToStr[regMemValue];
        bytesStream.read(reinterpret_cast<char*>(&buffer[2]), sizeof(buffer[2]));
        uint8_t data = fetchingFunc(INSTRUCTION_MASKS::DATA, buffer[2]);
        
        ss << instructionType << " " << regMemValueString  << "," << " " << std::to_string(static_cast<int8_t>(data)) << std::endl;
        return ss.str();
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
        
        return ss.str();
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
           << ((sValue == 0 && wValue == 1) ? std::to_string(static_cast<int16_t>(data)) : std::to_string(static_cast<int8_t>(data))) << std::endl;
        
        return ss.str();
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
           << ((sValue == 0 && wValue == 1) ? std::to_string(static_cast<int16_t>(data)) : std::to_string(static_cast<int8_t>(data))) << std::endl;
        
        return ss.str();
    }
    
    return "";
}

std::string handleImmediateToAccOpLogic(std::array<uint8_t, 6>& buffer, std::ifstream& bytesStream, const std::string& instructionType)
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
        instructionString << "ax, " << std::to_string(static_cast<int16_t>(data)) << std::endl;
    }
    else
    {
        instructionString << "al, " << std::to_string(static_cast<int8_t>(data)) << std::endl;
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
    return handleRegMemModValues(fetchedValues,OpCodeToString(OP_CODE_VALUES::ADD_REG_MEM),bytesStream, buffer);
}

std::string handleAddImmediateToAccumulator(std::array<uint8_t, 6>& buffer, std::ifstream& bytesStream)
{
    bytesStream.read(reinterpret_cast<char*>(&buffer[1]), sizeof(buffer[1]));
    return handleImmediateToAccOpLogic(buffer,bytesStream, OpCodeToString(OP_CODE_VALUES::ADD_IMMEDIATE_ACCUMULATOR));
}

std::string handleOpLogicImmediateToRegMem(std::array<uint8_t, 6> &buffer, std::ifstream &bytesStream)
{
    static std::map<uint8_t, std::string> opTypesToStr = {
            {0b000, "add"},
            {0b101, "sub"},
            {0b111, "cmp"}
    };
    bytesStream.read(reinterpret_cast<char*>(&buffer[1]), sizeof(buffer[1]));
    auto opType = fetchingFunc(INSTRUCTION_MASKS::REG_6BITES_OP_CODE, buffer[1]);
    
    return handleImmediateToRegisterLogicOp(buffer, bytesStream, opTypesToStr[opType]);
}

std::string handleSubImmediateToAccumulator(std::array<uint8_t, 6> &buffer, std::ifstream &bytesStream)
{
    bytesStream.read(reinterpret_cast<char*>(&buffer[1]), sizeof(buffer[1]));
    return handleImmediateToAccOpLogic(buffer,bytesStream, OpCodeToString(OP_CODE_VALUES::SUB_IMMEDIATE_ACCUMULATOR));
}

std::string handleSubRegMemInstruction(std::array<uint8_t, 6> &buffer, std::ifstream &bytesStream)
{
    bytesStream.read(reinterpret_cast<char*>(&buffer[1]), sizeof(buffer[1]));
    auto fetchedValues = fetchingRegMemData(buffer);
    return handleRegMemModValues(fetchedValues,OpCodeToString(OP_CODE_VALUES::SUB_REG_MEM),bytesStream, buffer);
}

std::string handleCmpImmediateToAccumulator(std::array<uint8_t, 6> &buffer, std::ifstream &bytesStream)
{
    bytesStream.read(reinterpret_cast<char*>(&buffer[1]), sizeof(buffer[1]));
    return handleImmediateToAccOpLogic(buffer,bytesStream, OpCodeToString(OP_CODE_VALUES::CMP_IMMEDIATE_ACCUMULATOR));
}

std::string handleCmpRegMemInstruction(std::array<uint8_t, 6> &buffer, std::ifstream &bytesStream)
{
    bytesStream.read(reinterpret_cast<char*>(&buffer[1]), sizeof(buffer[1]));
    auto fetchedValues = fetchingRegMemData(buffer);
    return handleRegMemModValues(fetchedValues,OpCodeToString(OP_CODE_VALUES::CMP_REG_MEM),bytesStream, buffer);
}

std::string handleJump(std::array<uint8_t, 6>& buffer, std::ifstream& bytesStream, std::string& jumpString) {
  bytesStream.read(reinterpret_cast<char*>(&buffer[1]), sizeof(buffer[1]));
  std::stringstream instructionString;
  
  instructionString << jumpString << " " << std::to_string(static_cast<int8_t>(buffer[1])) << std::endl;
  return instructionString.str();
}

std::string handleMovRegMemInstruction(std::array<uint8_t, 6>& buffer, std::ifstream& bytesStream)
{
    bytesStream.read(reinterpret_cast<char*>(&buffer[1]), sizeof(buffer[1]));
    auto fetchedValues = fetchingRegMemData(buffer);
    return handleRegMemModValues(fetchedValues,OpCodeToString(OP_CODE_VALUES::MOV_REG_MEM),bytesStream, buffer, MoveExecute);
}