//
// Created by bordeax on 12/1/24.
//

#include <cstdint>
#include "InstructionSimulatorHandler.h"

void MoveExecute(uint16_t &dest, uint16_t data, uint16_t lowHighBitmask)
{
    dest &= (~lowHighBitmask);
    if(lowHighBitmask == 0x00FF)
    {
        dest |= data;
    }
    else if(lowHighBitmask == 0xFF00)
    {
        data <<= 8;
        dest |= data;
    }
    
    dest = data;
}