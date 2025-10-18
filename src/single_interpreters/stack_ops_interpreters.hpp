//
// Created by aleksandr on 12.10.25.
//

#pragma once
#include "StackMachineState.hpp"

void stack_dup(StackMachineState& state) {
    state.instruction_decoder->consume_as<NoArgsInstruction>();
    state.frame_stack.push_op(state.frame_stack.peek_op());
}

void stack_drop(StackMachineState& state) {
    state.instruction_decoder->consume_as<NoArgsInstruction>();
    state.frame_stack.pop_op();
}

void stack_const(StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<SimpleInstructionWithArgs<1>>();
    state.frame_stack.push_op(box(inst.args[0]));
}