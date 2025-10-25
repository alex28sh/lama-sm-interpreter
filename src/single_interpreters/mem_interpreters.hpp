//
// Created by aleksandr on 12.10.25.
//

#pragma once
#include "InstructionDecodersByArgsNumber.hpp"
#include "StackMachineState.hpp"

enum MemVar : char {
    Global = 0x00,
    Local = 0x01,
    Argument = 0x02,
    Closure = 0x03,
};

inline void mem_load(const StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<InstructionWithArgsLowerBits<1>>();
    switch (inst.get_low_bits()) {
        case Global:
            state.frame_stack->push_op(state.frame_stack->get_global(inst.args[0]));
            break;
        case Local:
            state.frame_stack->push_op(state.frame_stack->get_local(inst.args[0]));
            break;
        case Argument:
            state.frame_stack->push_op(state.frame_stack->get_arg(inst.args[0]));
            break;
        case Closure:
            failure("Load for closure not implemented.");
        default:
            failure("Invalid load instruction lower bits.");
    }
}

inline void mem_store(const StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<InstructionWithArgsLowerBits<1>>();
    switch (static_cast<MemVar>(inst.get_low_bits())) {
        case Global:
            state.frame_stack->set_global(inst.args[0], state.frame_stack->peek_op());
            break;
        case Local:
            state.frame_stack->set_local(inst.args[0], state.frame_stack->peek_op());
            break;
        case Argument:
            state.frame_stack->set_arg(inst.args[0], state.frame_stack->peek_op());
            break;
        case Closure:
            failure("Store for closure not implemented.");
        default:
            failure("Invalid store instruction lower bits.");
    }
}