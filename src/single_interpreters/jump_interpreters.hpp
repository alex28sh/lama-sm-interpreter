//
// Created by aleksandr on 12.10.25.
//

#pragma once
#include "StackMachineState.hpp"

void jump(StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<SimpleInstructionWithArgs<1>>();
    state.instruction_decoder->code_ptr = (state.bf->code_ptr + inst.args[0]);
}

void jump_if_zero(StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<SimpleInstructionWithArgs<2>>();
    auto val = state.frame_stack.peek_op();
    state.frame_stack.pop_op();
    if (val == 0) {
        state.instruction_decoder->code_ptr = (state.bf->code_ptr + inst.args[1]);
    }
}

void jump_if_not_zero(StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<SimpleInstructionWithArgs<2>>();
    auto val = state.frame_stack.peek_op();
    state.frame_stack.pop_op();
    if (val != 0) {
        state.instruction_decoder->code_ptr = (state.bf->code_ptr + inst.args[1]);
    }
}