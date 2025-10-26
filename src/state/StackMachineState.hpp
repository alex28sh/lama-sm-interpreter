//
// Created by aleksandr on 12.10.25.
//

#pragma once
#include "bytefile.h"
#include "FrameStack.hpp"
#include "InstructionDecoder.hpp"

class StackMachineState {

public:
    explicit StackMachineState(bytefile* bf) {
        this->bf = bf;
        frame_stack = new FrameStack<stack_size>(bf->string_ptr, bf->stringtab_size, bf->global_area_size);
        instruction_decoder = new InstructionDecoder(bf->code_ptr);
    }

    bytefile *bf;
    constexpr static int stack_size = 64 * 1024 * 1024;
    FrameStack<stack_size> *frame_stack;
    InstructionDecoder *instruction_decoder;
};
