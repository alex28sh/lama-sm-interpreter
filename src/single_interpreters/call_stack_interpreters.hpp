//
// Created by aleksandr on 12.10.25.
//

#pragma once
#include "StackMachineState.hpp"

inline void call_stack_begin(const StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<SimpleInstructionWithArgs<2>>();
    auto n_locals = inst.args[1];

    auto max_stack = inst.args[0] >> 16;
    if (__gc_stack_top + 1 < state.frame_stack->stack_data + max_stack + n_locals) {
        throw std::runtime_error(fmt::format("Stack overflow at {}", state.instruction_decoder->code_ptr - inst.length()));
    }

    state.frame_stack->reserve_locals(n_locals);
}

inline void call_stack_end(const StackMachineState& state) {
    state.instruction_decoder->consume_as<NoArgsInstruction>();

    auto val = state.frame_stack->peek_op();
    auto code_ptr = state.frame_stack->pop_stack_frame();
    if (code_ptr == nullptr) {
        exit(0);
    }
    state.instruction_decoder->code_ptr = code_ptr;
    state.frame_stack->push_op(val);
}

inline void call_stack_call(const StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<SimpleInstructionWithArgs<2>>();

    auto jump = inst.args[0];
    auto nargs = inst.args[1];

    state.frame_stack->push_stack_frame(nargs, state.instruction_decoder->code_ptr);
    state.instruction_decoder->code_ptr = (state.bf->code_ptr + jump);
}