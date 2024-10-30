//
// Created by bordeax on 10/30/24.
//

#ifndef COMPUTER_ENHANCE_FOLLOW_UP_INSTRUCTIONSHANDLERS_H
#define COMPUTER_ENHANCE_FOLLOW_UP_INSTRUCTIONSHANDLERS_H
#include <string>
#include <array>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <functional>
#include "LUT.h"

uint8_t fetchingFunc(INSTRUCTION_MASKS dataMask,uint8_t buffer8Bit);

std::string handleMovRegMemInstruction(std::array<uint8_t, 6>& buffer, std::ifstream& bytesStream);

std::string handleMovImmediateToRegister(std::array<uint8_t, 6>& buffer, std::ifstream& bytesStream);

std::string handleAddRegMemInstruction(std::array<uint8_t, 6>& buffer, std::ifstream& bytesStream);

std::string handleAddImmediateToRegister(std::array<uint8_t, 6>& buffer, std::ifstream& bytesStream);


static std::unordered_map<OP_CODE_VALUES,std::function<std::string(std::array<uint8_t, 6>&,std::ifstream&)>> opCodesToFunc = {
        {OP_CODE_VALUES::MOV_REG_MEM,   handleMovRegMemInstruction},
        {OP_CODE_VALUES::MOV_IMMEDIATE, handleMovImmediateToRegister},
        {OP_CODE_VALUES::ADD_REG_MEM,   handleAddRegMemInstruction},
        {OP_CODE_VALUES::ADD_IMMEDIATE, handleAddImmediateToRegister}
};



#endif //COMPUTER_ENHANCE_FOLLOW_UP_INSTRUCTIONSHANDLERS_H