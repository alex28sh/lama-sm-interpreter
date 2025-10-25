//
// Created by aleksandr on 12.10.25.
//

#pragma once
#include "StackMachineState.hpp"

extern "C" {
#include "../runtime/runtime_common.h"

    int Barray_patt (void *d, int n);
    aint Btag (void *d, aint t, aint n);
}

inline void unboxing_tag(const StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<SimpleInstructionWithArgs<2>>();
    auto tag = inst.args[0];
    auto n = inst.args[1];

    auto ptr = reinterpret_cast<void*>(state.frame_stack->peek_op());
    state.frame_stack->pop_op();

    auto res = Btag(ptr, box(tag), box(n));
    state.frame_stack->push_op(res);
}

inline void unboxing_array(const StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<SimpleInstructionWithArgs<1>>();
    auto ptr = reinterpret_cast<void*>(state.frame_stack->peek_op());
    state.frame_stack->pop_op();
    auto res = Barray_patt(ptr, box(inst.args[0]));
    state.frame_stack->push_op(res);
}