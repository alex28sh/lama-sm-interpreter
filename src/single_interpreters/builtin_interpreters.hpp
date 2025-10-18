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


    // // Selected runtime helpers used by the VM
    // void *Bstring (aint* args);
    // aint  Lcompare (void *p, void *q);
    // void *LmakeString (aint length);
    // void *Lstringcat (aint* args);
    // void *Bclosure (aint* args, aint bn);
    // void *Bsexp (aint* args, aint bn);
    // aint  LtagHash (char *);
}

void call_read(StackMachineState& state) {
    state.instruction_decoder->consume_as<NoArgsInstruction>();
    state.frame_stack.push_op(Lread());
}

void call_write(StackMachineState& state) {
    state.instruction_decoder->consume_as<NoArgsInstruction>();
    auto val = state.frame_stack.peek_op();
    state.frame_stack.pop_op();
    Lwrite(val);
}

void call_length(StackMachineState& state) {
    state.instruction_decoder->consume_as<NoArgsInstruction>();
    auto val = state.frame_stack.peek_op();
    state.frame_stack.pop_op();
    Llength(reinterpret_cast<void *>(val));
}

void call_string(StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<SimpleInstructionWithArgs<1>>();

    /// TODO
    throw std::runtime_error("not implemented");
}

void call_array(StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<SimpleInstructionWithArgs<1>>();
    auto n_args = inst.args[0];

    state.frame_stack.pop_ops(n_args);

    /// TODO
    throw std::runtime_error("not implemented");
}

void call_elem(StackMachineState& state) {
    state.instruction_decoder->consume_as<NoArgsInstruction>();
    auto a = state.frame_stack.peek_op();
    state.frame_stack.pop_op();
    auto b = state.frame_stack.peek_op();
    state.frame_stack.pop_op();
    auto res = Belem(reinterpret_cast<void *>(a), b);
    state.frame_stack.push_op(reinterpret_cast<int32_t>(res));
}