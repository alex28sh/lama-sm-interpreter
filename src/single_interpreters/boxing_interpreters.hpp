//
// Created by aleksandr on 12.10.25.
//

#pragma once
#include "StackMachineState.hpp"
#include <stdio.h>

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
    const auto args_ptr = state.frame_stack->get_ops_ptr(n_args);

    auto res = Bsexp(reinterpret_cast<aint*>(args_ptr), box_int(n_args));

    state.frame_stack->pop_ops(n_args);
    state.frame_stack->push_op((reinterpret_cast<uint64_t>(res)));
}

inline void boxing_string(const StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<SimpleInstructionWithArgs<1>>();
    auto string_ptr = state.frame_stack->get_string_ptr(inst.args[0]);
    // std::cerr << *(char*)string_ptr << std::endl;
    auto d = TO_DATA(string_ptr);
    // std::cerr << "TO_DATA " << d->contents << std::endl;
    auto val = reinterpret_cast<uint64_t>(string_ptr);
    // std::cerr << val << " " << box_ptr(val) << " " << unbox(box_ptr(val)) << std::endl;
    state.frame_stack->push_op((reinterpret_cast<uint64_t>(string_ptr)));
}