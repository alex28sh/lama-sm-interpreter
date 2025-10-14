//
// Created by aleksandr on 12.10.25.
//

#pragma once
#include "StackMachineState.hpp"

void call_stack_begin(StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<SimpleInstructionWithArgs<6>>();
    auto n_locals = inst.args[1];
    state.frame_stack.reserve_locals(n_locals);
}

void call_stack_end(StackMachineState& state) {
    state.instruction_decoder->consume_as<NoArgsInstruction>();

    auto val = state.frame_stack.peek_op();
    auto code_ptr = state.frame_stack.pop_stack_frame();
    if (code_ptr == nullptr) {
        exit(0);
    }
    state.instruction_decoder->code_ptr = code_ptr;
    state.frame_stack.push_op(val);
}

void call_stack_call(StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<SimpleInstructionWithArgs<2>>();

    auto ra = inst.args[0];
    auto nargs = inst.args[1];

    state.frame_stack.push_stack_frame(nargs, state.instruction_decoder->code_ptr);
    state.instruction_decoder->code_ptr = (state.bf->code_ptr + ra);
}