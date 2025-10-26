//
// Created by aleksandr on 12.10.25.
//

#pragma once
#include "InstructionDecodersByArgsNumber.hpp"
#include "StackMachineState.hpp"

extern "C" {
#include "../runtime/runtime.h"
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
    // std::cerr << "Length " << res << std::endl;
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
    // std::cerr << "call_string " << is_boxed_int(a) << " ";
    // std::cerr << Llength(res) << std::endl;

    state.frame_stack->pop_op();
    state.frame_stack->push_op((reinterpret_cast<uint64_t>(res)));
}

inline void call_array(const StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<SimpleInstructionWithArgs<1>>();
    auto n_args = inst.args[0];
    auto args_ptr = state.frame_stack->get_ops_ptr(n_args);

    auto res = Barray(reinterpret_cast<aint*>(args_ptr), box_int(n_args));
    state.frame_stack->pop_ops(n_args);
    state.frame_stack->push_op((reinterpret_cast<uint64_t>(res)));
}

inline void call_elem(const StackMachineState& state) {
    state.instruction_decoder->consume_as<NoArgsInstruction>();
    auto a = state.frame_stack->peek_op();
    state.frame_stack->pop_op();
    auto b = (state.frame_stack->peek_op());
    state.frame_stack->pop_op();
    // std::cerr << *reinterpret_cast<char *>(b) << " " << unbox(a) << std::endl;

    // auto boxed = reinterpret_cast<void*>(box(reinterpret_cast<uint64_t>(b)));

    auto res = Belem(reinterpret_cast<void *>(b), a);
    // std::cerr << "ELEM: " << a << " " << reinterpret_cast<uint64_t>(res) << std::endl;
    // auto d = TO_DATA(b);
    // std::cerr<< b << " " << d->contents << std::endl;
    // std::cerr << static_cast<char>(unbox(reinterpret_cast<uint64_t>(res))) << std::endl;
    state.frame_stack->push_op(reinterpret_cast<uint64_t>(res));
}