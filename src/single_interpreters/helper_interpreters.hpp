//
// Created by aleksandr on 12.10.25.
//

#pragma once
#include "InstructionDecodersByArgsNumber.hpp"
#include "StackMachineState.hpp"
#include "../runtime/runtime.h"

void helper_line(StackMachineState& state) {
    state.instruction_decoder->consume_as<SimpleInstructionWithArgs<1>>();
}

void helper_fail(StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<SimpleInstructionWithArgs<2>>();
    failure("matching failure at %d", inst.args[0]);
}