//
// Created by aleksandr on 12.10.25.
//

#pragma once
#include <cstddef>

template <std::size_t n_args>
class SimpleInstructionWithArgs {
public:
    char instruction;
    int args[n_args];

    static constexpr size_t length() {
        return 1 + n_args * sizeof(int);
    }
};

typedef SimpleInstructionWithArgs<0> NoArgsInstruction;

template <std::size_t n_args>
class InstructionWithArgsLowerBits {
public:
    char instruction;
    int args[n_args];

    static constexpr size_t length() {
        return 1 + n_args * sizeof(int);
    }

    constexpr char get_low_bits() {
        return low_bits(instruction);
    }
};