//
// Created by bordeax on 12/1/24.
//

#ifndef COMPUTER_ENHANCE_FOLLOW_UP_INSTRUCTIONSIMULATORHANDLER_H
#define COMPUTER_ENHANCE_FOLLOW_UP_INSTRUCTIONSIMULATORHANDLER_H

#include <array>
#include <cstdint>
#include <bitset>

extern std::bitset<2> flags;

void MoveExecute(uint16_t &dest, uint16_t data, uint16_t lowHighBitmask);

void AddExecute(uint16_t &dest, uint16_t data, uint16_t lowHighBitmask);

void SubExecute(uint16_t &dest, uint16_t data, uint16_t lowHighBitmask);

void CompExecute(uint16_t &dest, uint16_t data, uint16_t lowHighBitmask);

#endif //COMPUTER_ENHANCE_FOLLOW_UP_INSTRUCTIONSIMULATORHANDLER_H