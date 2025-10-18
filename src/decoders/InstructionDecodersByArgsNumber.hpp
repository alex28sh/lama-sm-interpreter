//
// Created by aleksandr on 12.10.25.
//

#pragma once
#include <cstddef>

#include "InstructionDecoder.hpp"

#pragma pack(push, 1)

template <std::size_t n_args>
class SimpleInstructionWithArgs {
public:
    char instruction;
    uint32_t args[n_args];

    static constexpr size_t length() {
        return 1 + n_args * sizeof(uint32_t);
    }
};

typedef SimpleInstructionWithArgs<0> NoArgsInstruction;

template <std::size_t n_args>
class InstructionWithArgsLowerBits {
public:
    char instruction;
    uint32_t args[n_args];

    static constexpr size_t length() {
        return 1 + n_args * sizeof(uint32_t);
    }

    constexpr char get_low_bits() {
        return low_bits(instruction);
    }
};

#pragma pack(pop)