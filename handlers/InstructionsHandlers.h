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

std::string handleAddImmediateToAccumulator(std::array<uint8_t, 6>& buffer, std::ifstream& bytesStream);

std::string handleOpLogicImmediateToRegMem(std::array<uint8_t, 6>& buffer, std::ifstream& bytesStream);

std::string handleSubImmediateToAccumulator(std::array<uint8_t, 6>& buffer, std::ifstream& bytesStream);

std::string handleSubRegMemInstruction(std::array<uint8_t, 6>& buffer, std::ifstream& bytesStream);

std::string handleCmpImmediateToAccumulator(std::array<uint8_t, 6>& buffer, std::ifstream& bytesStream);

std::string handleCmpRegMemInstruction(std::array<uint8_t, 6>& buffer, std::ifstream& bytesStream);

std::string handleJump(std::array<uint8_t, 6>& buffer, std::ifstream& bytesStream, std::string& jumpString);


static std::unordered_map<OP_CODE_VALUES,std::function<std::string(std::array<uint8_t, 6>&,std::ifstream&)>> opCodesToFunc = {
        {OP_CODE_VALUES::MOV_REG_MEM,                handleMovRegMemInstruction},
        {OP_CODE_VALUES::MOV_IMMEDIATE,              handleMovImmediateToRegister},
        {OP_CODE_VALUES::ADD_REG_MEM,                handleAddRegMemInstruction},
        {OP_CODE_VALUES::ADD_IMMEDIATE_ACCUMULATOR,  handleAddImmediateToAccumulator},
        {OP_CODE_VALUES::OP_LOGIC_IMMEDIATE_REG_MEM, handleOpLogicImmediateToRegMem},
        {OP_CODE_VALUES::SUB_IMMEDIATE_ACCUMULATOR,  handleSubImmediateToAccumulator},
        {OP_CODE_VALUES::SUB_REG_MEM,                handleSubRegMemInstruction},
        {OP_CODE_VALUES::CMP_REG_MEM,                handleCmpRegMemInstruction},
        {OP_CODE_VALUES::CMP_IMMEDIATE_ACCUMULATOR,  handleCmpImmediateToAccumulator}
};

#endif //COMPUTER_ENHANCE_FOLLOW_UP_INSTRUCTIONSHANDLERS_H