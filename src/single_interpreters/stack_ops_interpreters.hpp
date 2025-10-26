//
// Created by aleksandr on 12.10.25.
//

#pragma once
#include "StackMachineState.hpp"

extern "C" {
#include "../runtime/runtime.h"
#include "../runtime/runtime_common.h"

    void *Bsta (void *x, aint i, void *v);
}

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
    state.frame_stack->push_op(box_int(inst.args[0]));
}

inline void stack_swap(const StackMachineState& state) {
    state.instruction_decoder->consume_as<NoArgsInstruction>();
    auto a = state.frame_stack->peek_op();
    state.frame_stack->pop_op();
    auto b = state.frame_stack->peek_op();
    state.frame_stack->pop_op();
    state.frame_stack->push_op(a);
    state.frame_stack->push_op(b);
}

inline void stack_sta(const StackMachineState& state) {
    state.instruction_decoder->consume_as<NoArgsInstruction>();
    auto z = state.frame_stack->peek_op();
    state.frame_stack->pop_op();
    auto i = state.frame_stack->peek_op();
    state.frame_stack->pop_op();
    auto ptr_z = reinterpret_cast<uint64_t*>(z);
    if (is_boxed_int(i)) {
        auto x = state.frame_stack->peek_op();
        auto ptr_x = reinterpret_cast<uint64_t*>(x);
        state.frame_stack->pop_op();
        auto res = Bsta(ptr_x, i, ptr_z);
    } else {
        auto ptr_i = reinterpret_cast<uint64_t*>(i);
        *ptr_z = *ptr_i;
    }
    state.frame_stack->push_op(z);
}

inline void stack_sti(const StackMachineState& state) {
    state.instruction_decoder->consume_as<NoArgsInstruction>();
    auto z = state.frame_stack->peek_op();
    state.frame_stack->pop_op();
    auto i = state.frame_stack->peek_op();
    state.frame_stack->pop_op();
    auto ptr_z = reinterpret_cast<uint64_t*>(z);
    if (is_boxed_int(i)) {
        throw std::runtime_error("For STI second argument should be a variable");
    }
    auto ptr_i = reinterpret_cast<uint64_t*>(i);
    *ptr_z = *ptr_i;
    state.frame_stack->push_op(z);
}