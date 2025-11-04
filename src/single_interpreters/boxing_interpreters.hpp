//
// Created by aleksandr on 12.10.25.
//

#pragma once
#include "StackMachineState.hpp"

extern "C" {
    #include "../runtime/runtime_common.h"

    void *Bsexp (aint* args, aint bn);
    aint LtagHash (char *s);
}

static_assert(sizeof(void*) == sizeof(size_t));

inline void boxing_sexp(const StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<SimpleInstructionWithArgs<2>>();

    const auto tag = inst.args[0];
    auto tag_ptr = state.frame_stack->get_string_ptr(tag);

    state.frame_stack->push_op(LtagHash(reinterpret_cast<char*>(tag_ptr)));

    const auto n_args = inst.args[1] + 1;
    auto res = Bsexp(reinterpret_cast<aint *>(__gc_stack_top + 1), box_int(n_args));

    state.frame_stack->pop_ops(n_args);
    state.frame_stack->push_op((reinterpret_cast<uint64_t>(res)));
}

inline void boxing_string(const StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<SimpleInstructionWithArgs<1>>();
    auto string_ptr = state.frame_stack->get_string_ptr(inst.args[0]);
    state.frame_stack->push_op((reinterpret_cast<uint64_t>(string_ptr)));
}