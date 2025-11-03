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

// inline void closures_ret(const StackMachineState& state) {
//     state.instruction_decoder->consume_as<NoArgsInstruction>();
//
//     auto val = state.frame_stack->peek_op();
//     auto code_ptr = state.frame_stack->pop_stack_frame();
//     if (code_ptr == nullptr) {
//         exit(0);
//     }
//     state.instruction_decoder->code_ptr = code_ptr;
//     state.frame_stack->push_op(val);
// }

/// TODO: a separate class as SimpleInstruction... for CLOSURE

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
        // std::cerr << magic_enum::enum_name(designation.mem_var) << " " << index << std::endl;

        switch (designation.mem_var) {
            case Local:
                state.frame_stack->push_op(state.frame_stack->get_local(index));
                break;
            case Argument: {
                // auto link = state.frame_stack->get_arg_link(index);
                // std::cerr << *link << std::endl;
                state.frame_stack->push_op(state.frame_stack->get_arg(index));
                // std::cerr << *state.frame_stack->peek_op_ptr() << std::endl;
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

    // auto args_ptr = state.frame_stack->get_ops_ptr(n_args + 1);

    // for (int i = 0; i < n_args; i++) {
    //     std::cerr << "Stored value " << *(args_ptr + i + 1) << std::endl;
    // }

    auto res = Bclosure(reinterpret_cast<aint*>(__gc_stack_top + 1), box_int(n_args));
    state.frame_stack->pop_ops(n_args + 1);
    state.frame_stack->push_op(reinterpret_cast<uint64_t >(res));
    // state.frame_stack->push_op_link((reinterpret_cast<uint64_t*>(res)));
    // state.frame_stack->push_op_link(reinterpret_cast<uint64_t *>(res));
    // std::cerr << reinterpret_cast<uint64_t>(res) << std::endl;

    // auto a = TO_DATA(res);
    // std::cerr << "TO_DATA(a)" << std::endl;
    // std::cerr << reinterpret_cast<uint64_t>(a) << std::endl;
    // std::cerr << TAG(a->data_header) << std::endl;
    //
    // std::cerr << "create_Belem" << std::endl;
    // auto elem = Belem(res, box_int(0));
    // std::cerr << "create_elem" << std::endl;
    // std::cerr << reinterpret_cast<uint64_t>(elem) << std::endl;

    // auto ptr = reinterpret_cast<void*>(unbox(state.frame_stack->peek_op()));
    // auto elem = Belem(ptr, box_int(0 + 1));
    // std::cerr << "closure_create " << n_args << " " << elem << std::endl;
}

inline void closures_call(const StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<SimpleInstructionWithArgs<1>>();
    auto nargs = inst.args[0];

    auto closure_ptr = reinterpret_cast<void*>(*(__gc_stack_top + 1 + nargs));

    // std::cerr << "Belem" << std::endl;
    // std::cerr << reinterpret_cast<uint64_t>(closure_ptr) << std::endl;
    // auto a = TO_DATA(closure_ptr);
    // std::cerr << "TO_DATA(a)" << std::endl;
    // std::cerr << reinterpret_cast<uint64_t>(a) << std::endl;
    // std::cerr << TAG(a->data_header) << std::endl;
    auto elem = Belem(closure_ptr, box_int(0));
    // std::cerr << "elem" << std::endl;
    // std::cerr << reinterpret_cast<uint64_t>(elem) << std::endl;

    auto jump = unbox(reinterpret_cast<uint64_t>(elem));

    // std::cerr << *jump << std::endl;

    // std::cerr << "0x"
    //           << std::setw(8) << std::setfill('0') << std::hex
    //           << jump << std::dec << std::endl;

    state.frame_stack->push_stack_frame(nargs, state.instruction_decoder->code_ptr, 1);
    state.instruction_decoder->code_ptr = state.bf->code_ptr + jump;
    // Belem()

    // state.frame_stack->pop_ops(n_args);
}

// inline void closures_cbegin(const StackMachineState& state) {
//     auto inst = state.instruction_decoder->consume_as<SimpleInstructionWithArgs<2>>();
//     throw std::runtime_error("not implemented");
// }