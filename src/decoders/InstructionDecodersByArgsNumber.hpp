//
// Created by aleksandr on 12.10.25.
//

#pragma once
#include "InstructionType.hpp"

template <size_t args>
class SimpleInstructionWithArgs {

    static constexpr size_t length() {
        return 1 + args * sizeof(int);
    }
};
