//
// Created by aleksandr on 12.10.25.
//

#pragma once
#include "InstructionDecodersByArgsNumber.hpp"
#include "StackMachineState.hpp"
#include "../runtime/runtime.h"
#include <magic_enum/magic_enum.hpp>

#include <map>

#include "boxing.hpp"

enum BinOp : char {
    PLUS = 0x01,
    MINUS = 0x02,
    MUL = 0x03,
    DIV = 0x04,
    MOD = 0x05,
    LT = 0x06,
    LE = 0x07,
    GT = 0x08,
    GE = 0x09,
    EQ = 0x0A,
    NEQ = 0x0B,
    AND = 0x0C,
    OR = 0x0D,
};

inline void boxed_unboxed(const StackMachineState& state, const std::function<int64_t(int64_t, int64_t)>& op) {
    auto b = unbox(state.frame_stack->peek_op());
    state.frame_stack->pop_op();
    auto a = unbox(state.frame_stack->peek_op());
    state.frame_stack->pop_op();
    auto res = op(a, b);
    state.frame_stack->push_op(box(res));
}

inline void boxed(const StackMachineState& state, const std::function<int64_t(int64_t, int64_t)>& op) {
    auto b = state.frame_stack->peek_op();
    state.frame_stack->pop_op();
    auto a = state.frame_stack->peek_op();
    state.frame_stack->pop_op();
    auto res = op(a, b);
    state.frame_stack->push_op(box(res));
}

const std::map<BinOp, std::function<int64_t(int64_t, int64_t)>> BinOpMap = {
    {PLUS, std::plus()},
    {MINUS, std::minus()},
    {MUL, std::multiplies()},
    {DIV, std::divides()},
    {MOD, std::modulus()},
    {LT, std::less()},
    {LE, std::less_equal()},
    {GT, std::greater()},
    {GE, std::greater_equal()},
    {AND, std::logical_and()},
    {OR, std::logical_or()},
    {EQ, std::equal_to()},
    {NEQ, std::not_equal_to()},
};

inline void binop_interpeter(const StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<InstructionWithArgsLowerBits<0>>();
    auto low_bits_inst = static_cast<BinOp>(low_bits(inst.instruction));
    switch (low_bits_inst) {
        case EQ:
        case NEQ: {
            boxed(state, BinOpMap.at(static_cast<BinOp>(low_bits_inst)));
            break;
        }

        case DIV:
        case MOD: {

            auto b = state.frame_stack->peek_op();
            state.frame_stack->pop_op();

            if (b == box(0)) {
                throw std::runtime_error(fmt::format("Second argument of {} is zero", magic_enum::enum_name(low_bits_inst)));
            }

            auto a = state.frame_stack->peek_op();
            state.frame_stack->pop_op();
            uint64_t res;
            if (low_bits_inst == DIV) {
                res = a / b;
            } else {
                res = a % b;
            }
            state.frame_stack->push_op(box(res));

            break;
        }

        case PLUS:
        case MINUS:
        case MUL:
        case LE:
        case LT:
        case GE:
        case GT:
        case AND:
        case OR:
            boxed_unboxed(state, BinOpMap.at(static_cast<BinOp>(low_bits_inst)));
            break;
        default:
            throw std::runtime_error(fmt::format("Unknown binop {}", magic_enum::enum_name(low_bits_inst)));
    }
}
