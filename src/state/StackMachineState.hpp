//
// Created by aleksandr on 12.10.25.
//

#pragma once
#include "bytefile.h"
#include "FrameStack.hpp"
#include "InstructionDecoder.hpp"

void *__start_custom_data;
void *__stop_custom_data;

class StackMachineState {

public:
    explicit StackMachineState(const bytefile* bf) {
        this->bf = bf;
        global_area = new uint32_t[bf->global_area_size];
        instruction_decoder = new InstructionDecoder(bf->code_ptr);
    }

    const bytefile* bf;
    FrameStack<64 * 1024 * 1024> frame_stack{};
    uint32_t *global_area;
    InstructionDecoder* instruction_decoder;
};
