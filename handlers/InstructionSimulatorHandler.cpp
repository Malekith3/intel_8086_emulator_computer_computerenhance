//
// Created by bordeax on 12/1/24.
//
#include "InstructionSimulatorHandler.h"
#include "LUT.h"

void zeroFlagTest(const uint16_t dest)
{
    if(dest == 0)
    {
        flags.set(ZERO);
    }
    else
    {
        flags.reset(ZERO);
    }
    
}

void signFlagTest(const uint16_t dest)
{
    if(static_cast<bool>((dest & 0x8000) >> 15))
    {
        flags.set(SIGN);
    }
    else
    {
        flags.reset(SIGN);
    }
}

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
    
    zeroFlagTest(dest);
    signFlagTest(dest);
}

void AddExecute(uint16_t &dest, uint16_t data, uint16_t lowHighBitmask)
{
    dest += data;
    zeroFlagTest(dest);
    signFlagTest(dest);
}

void SubExecute(uint16_t &dest, uint16_t data, uint16_t lowHighBitmask)
{
    dest-=data;
    zeroFlagTest(dest);
    signFlagTest(dest);
}

void CompExecute(uint16_t &dest, uint16_t data, uint16_t lowHighBitmask)
{
    uint16_t comprResult{static_cast<uint16_t>(dest - data)};
    zeroFlagTest(comprResult);
    if(static_cast<uint16_t>(dest - data) != 0)
    {
        signFlagTest(0x8000);
    }
}