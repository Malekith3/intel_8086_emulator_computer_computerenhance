//
// Created by bordeax on 10/30/24.
//

#ifndef COMPUTER_ENHANCE_FOLLOW_UP_LUT_H
#define COMPUTER_ENHANCE_FOLLOW_UP_LUT_H
#include <string>
#include <map>
#include <set>

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
    OPCODE_5BITES = 0b11111000,
    DATA = 0b11111111,
    OPCODE_7BITES = 0b11111110,
    OPCODE_8BITES = 0b11111111
    
};

enum class OP_CODE_VALUES : uint8_t
{
    MOV_REG_MEM = 0b00100010,
    MOV_IMMEDIATE = 0b00001011,
    ADD_REG_MEM = 0b00000000,
    ADD_IMMEDIATE_ACCUMULATOR = 0b0000010,
    OP_LOGIC_IMMEDIATE_REG_MEM = 0b100000,
    SUB_REG_MEM = 0b001010,
    SUB_IMMEDIATE_ACCUMULATOR = 0b0010110,
    CMP_REG_MEM = 0b001110,
    CMP_IMMEDIATE_ACCUMULATOR = 0b0011110
};

static std::set<uint8_t > opcodes4Bit {
        static_cast<uint8_t>(OP_CODE_VALUES::MOV_IMMEDIATE)
};

static std::set<uint8_t> opcodes5Bit {
};

static std::set<uint8_t> opcodes6Bit {
        static_cast<uint8_t>(OP_CODE_VALUES::MOV_REG_MEM),
        static_cast<uint8_t>(OP_CODE_VALUES::ADD_REG_MEM),
        static_cast<uint8_t>(OP_CODE_VALUES::OP_LOGIC_IMMEDIATE_REG_MEM),
        static_cast<uint8_t>(OP_CODE_VALUES::SUB_REG_MEM),
        static_cast<uint8_t>(OP_CODE_VALUES::CMP_REG_MEM),
};

static std::set<uint8_t> opcodes7Bit {
        static_cast<uint8_t>(OP_CODE_VALUES::ADD_IMMEDIATE_ACCUMULATOR),
        static_cast<uint8_t>(OP_CODE_VALUES::SUB_IMMEDIATE_ACCUMULATOR),
        static_cast<uint8_t>(OP_CODE_VALUES::CMP_IMMEDIATE_ACCUMULATOR)
};


static std::map<INSTRUCTION_MASKS,int> shiftRight{
        {INSTRUCTION_MASKS::OPCODE_6BITES,     2},
        {INSTRUCTION_MASKS::OPCODE_7BITES,     1},
        {INSTRUCTION_MASKS::W_6BITES,          0},
        {INSTRUCTION_MASKS::W_4BITES,          3},
        {INSTRUCTION_MASKS::D,                 1},
        {INSTRUCTION_MASKS::MOD,               6},
        {INSTRUCTION_MASKS::REG_6BITES_OP_CODE,3},
        {INSTRUCTION_MASKS::REG_4BITES_OP_CODE,0},
        {INSTRUCTION_MASKS::REG_MEM,           0},
        {INSTRUCTION_MASKS::OPCODE_4BITES,     4},
        {INSTRUCTION_MASKS::OPCODE_5BITES,     5},
        {INSTRUCTION_MASKS::DATA,              0}
    
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

inline std::array<std::string, 8> regNamesToStr{
        "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"
};

inline std::array<std::string, 8> regNamesExtendedToStr{
        "ax", "cx", "dx", "bx", "sp", "bp", "si", "di"
};

inline std::array<std::string, 8> regNamesPlus{
        "bx + si", "bx + di", "bp + si", "bp + di", "si", "di", "bp", "bx"
};

inline std::string OpCodeToString(OP_CODE_VALUES opCode)
{
    static const std::map<OP_CODE_VALUES, std::string> opCodesToString{
            {OP_CODE_VALUES::MOV_REG_MEM,   "mov"},
            {OP_CODE_VALUES::MOV_IMMEDIATE, "mov"},
            {OP_CODE_VALUES::ADD_REG_MEM,   "add"},
            {OP_CODE_VALUES::ADD_IMMEDIATE_ACCUMULATOR, "add"},
            {OP_CODE_VALUES::OP_LOGIC_IMMEDIATE_REG_MEM, "add"},
            {OP_CODE_VALUES::SUB_REG_MEM,"sub"},
            {OP_CODE_VALUES::SUB_IMMEDIATE_ACCUMULATOR, "sub"},
            {OP_CODE_VALUES::CMP_REG_MEM, "cmp"},
            {OP_CODE_VALUES::CMP_IMMEDIATE_ACCUMULATOR, "cmp"}
    };
    
    auto it = opCodesToString.find(opCode);
    return (it != opCodesToString.end()) ? it->second : "";
}

static std::unordered_map<uint8_t, std::string> jumpOpCodeToStrMap = {

    {0b11100011, "jcxz"},       // Jump if CX register is zero
    // Conditional Jumps
    {0b01110000, "jo"},         // Jump if overflow
    {0b01110001, "jno"},        // Jump if not overflow
    {0b01110010, "jb"},         // Jump if below (unsigned)
    {0b01110011, "jae"},        // Jump if above or equal (unsigned)
    {0b01110100, "je"},         // Jump if equal (zero)
    {0b01110101, "jne"},        // Jump if not equal (non-zero)
    {0b01110110, "jbe"},        // Jump if below or equal (unsigned)
    {0b01110111, "ja"},         // Jump if above (unsigned)
    {0b01111000, "js"},         // Jump if sign (negative)
    {0b01111001, "jns"},        // Jump if not sign (positive)
    {0b01111010, "jp"},         // Jump if parity (even)
    {0b01111011, "jpo"},        // Jump if parity odd
    {0b01111100, "jl"},         // Jump if less (signed)
    {0b01111101, "jge"},        // Jump if greater or equal (signed)
    {0b01111110, "jle"},        // Jump if less or equal (signed)
    {0b01111111, "jg"}          // Jump if greater (signed)
};

#endif //COMPUTER_ENHANCE_FOLLOW_UP_LUT_H