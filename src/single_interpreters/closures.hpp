//
// Created by aleksandr on 26.10.25.
//

#pragma once
#include <iomanip>

#include "Designations.hpp"
#include "StackMachineState.hpp"

extern "C" {
    void *Belem (void *p, aint i);
    void *Bclosure (aint* args, aint bn);
}

inline void closures_create(const StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<SimpleInstructionWithArgs<2>>();
    auto ip = inst.args[0];
    auto n_args = inst.args[1];

    Designations designations = {};

    for (uint32_t i = 0; i < n_args; i++) {
        auto designation = state.instruction_decoder->consume_as<Designation>();
        designations.push_back(designation);
    }
    std::reverse(designations.begin(), designations.end());

    for (auto designation : designations) {
        auto index = designation.value;

        switch (designation.mem_var) {
            case Local:
                state.frame_stack->push_op(state.frame_stack->get_local(index));
                break;
            case Argument: {
                state.frame_stack->push_op(state.frame_stack->get_arg(index));
                break;
            }
            case Closure:
                state.frame_stack->push_op(state.frame_stack->get_closure(index));
                break;
            default:
                throw std::runtime_error("Invalid designation for closure.");
        }
    }

    state.frame_stack->push_op(box_int(ip));

    auto res = Bclosure(reinterpret_cast<aint*>(__gc_stack_top + 1), box_int(n_args));
    state.frame_stack->pop_ops(n_args + 1);
    state.frame_stack->push_op(reinterpret_cast<uint64_t >(res));
}

inline void closures_call(const StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<SimpleInstructionWithArgs<1>>();
    auto nargs = inst.args[0];

    auto closure_ptr = reinterpret_cast<void*>(*(__gc_stack_top + 1 + nargs));
    auto elem = Belem(closure_ptr, box_int(0));
    auto jump = unbox(reinterpret_cast<uint64_t>(elem));

    state.frame_stack->push_stack_frame(nargs, state.instruction_decoder->code_ptr, 1);
    state.instruction_decoder->code_ptr = state.bf->code_ptr + jump;
}
