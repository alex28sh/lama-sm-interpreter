//
// Created by aleksandr on 12.10.25.
//

#pragma once
#include "InstructionDecodersByArgsNumber.hpp"
#include "StackMachineState.hpp"

inline void helper_line(const StackMachineState& state) {
    state.instruction_decoder->consume_as<SimpleInstructionWithArgs<1>>();
}

inline void helper_fail(const StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<SimpleInstructionWithArgs<2>>();
    throw std::runtime_error(fmt::format("Matching failure at {}", inst.args[0]));
}