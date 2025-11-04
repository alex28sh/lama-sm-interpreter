//
// Created by aleksandr on 12.10.25.
//

#pragma once
#include "InstructionDecodersByArgsNumber.hpp"
#include "StackMachineState.hpp"

extern "C" {
#include "../runtime/runtime_common.h"

    aint  Lread (void);
    aint  Lwrite (aint n);
    aint  Llength (void *);

    void *Belem (void *p, aint i);
    void *Lstring (aint* args);
    void *Barray (aint* args, aint bn);
}

inline void call_read(const StackMachineState& state) {
    state.instruction_decoder->consume_as<NoArgsInstruction>();
    auto val = Lread();
    state.frame_stack->push_op(val);
}

inline void call_write(const StackMachineState& state) {
    state.instruction_decoder->consume_as<NoArgsInstruction>();
    auto val = state.frame_stack->peek_op();
    Lwrite(val);
}

inline void call_length(const StackMachineState& state) {
    state.instruction_decoder->consume_as<NoArgsInstruction>();
    auto val = (state.frame_stack->peek_op());
    state.frame_stack->pop_op();
    auto res = Llength(reinterpret_cast<void *>(val));
    state.frame_stack->push_op(res);
}

inline void call_string(const StackMachineState& state) {
    state.instruction_decoder->consume_as<NoArgsInstruction>();
    auto a = state.frame_stack->peek_op();
    void *res;
    if (is_boxed_int(a)) {
        res = Lstring(reinterpret_cast<aint *>(state.frame_stack->peek_op_ptr()));
    } else {
        auto ptr = new aint*;
        *ptr = reinterpret_cast<aint *>((a));
        res = Lstring(reinterpret_cast<aint *>(ptr));
    }

    state.frame_stack->pop_op();
    state.frame_stack->push_op((reinterpret_cast<uint64_t>(res)));
}

inline void call_array(const StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<SimpleInstructionWithArgs<1>>();
    auto n_args = inst.args[0];

    std::reverse(__gc_stack_top + 1, __gc_stack_top + 1 + n_args);

    auto res = Barray(reinterpret_cast<aint *>(__gc_stack_top + 1), box_int(n_args));
    state.frame_stack->pop_ops(n_args);
    state.frame_stack->push_op((reinterpret_cast<uint64_t>(res)));
}

inline void call_elem(const StackMachineState& state) {
    state.instruction_decoder->consume_as<NoArgsInstruction>();
    auto a = state.frame_stack->peek_op();
    state.frame_stack->pop_op();
    auto b = (state.frame_stack->peek_op());
    state.frame_stack->pop_op();
    auto res = Belem(reinterpret_cast<void *>(b), a);
    state.frame_stack->push_op(reinterpret_cast<uint64_t>(res));
}