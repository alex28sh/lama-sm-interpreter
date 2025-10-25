//
// Created by aleksandr on 12.10.25.
//

#pragma once
#include "StackMachineState.hpp"
#include <stdio.h>

extern "C" {
    #include "../runtime/runtime_common.h"

    void *Bsexp (aint* args, aint bn);
}

static_assert(sizeof(void*) == sizeof(size_t));

inline void boxing_sexp(const StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<SimpleInstructionWithArgs<2>>();

    const auto s = inst.args[0];
    state.frame_stack->push_op(box(s));

    const auto n_args = inst.args[1] + 1;
    const auto args_ptr = state.frame_stack->get_ops_ptr(n_args);

    auto res = Bsexp(reinterpret_cast<aint*>(args_ptr), box(n_args));

    state.frame_stack->pop_ops(n_args);
    state.frame_stack->push_op(reinterpret_cast<uint64_t>(res));
}