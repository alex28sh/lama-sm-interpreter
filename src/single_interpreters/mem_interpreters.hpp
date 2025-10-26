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
    uint32_t index = inst.args[0];
    switch (inst.get_low_bits()) {
        case Global:
            state.frame_stack->push_op(state.frame_stack->get_global(index));
            break;
        case Local:
            state.frame_stack->push_op(state.frame_stack->get_local(index));
            break;
        case Argument:
            state.frame_stack->push_op(state.frame_stack->get_arg(index));
            break;
        case Closure:
            throw std::runtime_error("Load for closure not implemented.");
        default:
            throw std::runtime_error("Invalid load instruction lower bits.");
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
            throw std::runtime_error("Store for closure not implemented.");
        default:
            throw std::runtime_error("Invalid store instruction lower bits.");
    }
}

inline void stack_lda(const StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<InstructionWithArgsLowerBits<1>>();
    auto index = inst.args[0];
    switch (static_cast<MemVar>(inst.get_low_bits())) {
        case Global:
            state.frame_stack->push_op_link(state.frame_stack->get_global_link(index));
            break;
        case Local:
            state.frame_stack->push_op_link(state.frame_stack->get_local_link(index));
            break;
        case Argument:
            state.frame_stack->push_op_link(state.frame_stack->get_arg_link(index));
            break;
        case Closure:
            throw std::runtime_error("Store for closure not implemented.");
        default:
            throw std::runtime_error("Invalid store instruction lower bits.");
    }
}