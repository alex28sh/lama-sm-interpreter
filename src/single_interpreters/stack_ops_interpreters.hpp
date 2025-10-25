//
// Created by aleksandr on 12.10.25.
//

#pragma once
#include "StackMachineState.hpp"

inline void stack_dup(const StackMachineState& state) {
    state.instruction_decoder->consume_as<NoArgsInstruction>();
    state.frame_stack->push_op(state.frame_stack->peek_op());
}

inline void stack_drop(const StackMachineState& state) {
    state.instruction_decoder->consume_as<NoArgsInstruction>();
    state.frame_stack->pop_op();
}

inline void stack_const(const StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<SimpleInstructionWithArgs<1>>();
    state.frame_stack->push_op(box(inst.args[0]));
}