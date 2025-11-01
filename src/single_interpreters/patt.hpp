//
// Created by aleksandr on 30.10.25.
//

#pragma once

enum Patt : char {
    PattStrCmp = 0x00,
    PattString = 0x01,
    PattArray = 0x02,
    PattSexp = 0x03,
    PattBoxed = 0x04,
    PattUnBoxed = 0x05,
    PattClosure = 0x06,
};

extern "C" {
    aint Bclosure_tag_patt (void *x);
    aint Bstring_patt (void *x, void *y);

    aint Bboxed_patt (void *x);
    aint Bunboxed_patt (void *x);

    aint Barray_tag_patt (void *x);
    aint Bstring_tag_patt (void *x);
    aint Bsexp_tag_patt (void *x);
}

void patt(const StackMachineState& state) {
    auto inst = state.instruction_decoder->consume_as<InstructionWithArgsLowerBits<0>>();
    auto low_bits_inst = static_cast<Patt>(low_bits(inst.instruction));

    uint64_t is_patt;

    switch(low_bits_inst) {
        case PattClosure: {
            auto closure_ptr = reinterpret_cast<void*>(state.frame_stack->peek_op());
            is_patt = Bclosure_tag_patt(closure_ptr);
            break;
        }
        case PattStrCmp: {
            auto str1_ptr = reinterpret_cast<void*>(state.frame_stack->peek_op());
            state.frame_stack->pop_op();
            auto str2_ptr = reinterpret_cast<void*>(state.frame_stack->peek_op());
            is_patt = Bstring_patt(str1_ptr, str2_ptr);
            break;
        }
        case PattBoxed: {
            auto box_ptr = reinterpret_cast<void*>(state.frame_stack->peek_op());
            is_patt = Bboxed_patt(box_ptr);
            break;
        }
        case PattUnBoxed: {
            auto unbox_ptr = reinterpret_cast<void*>(state.frame_stack->peek_op());
            is_patt = Bunboxed_patt(unbox_ptr);
            break;
        }
        case PattArray: {
            auto array_ptr = reinterpret_cast<void*>(state.frame_stack->peek_op());
            is_patt = Barray_tag_patt(array_ptr);
            break;
        }
        case PattString: {
            auto string_ptr = reinterpret_cast<void*>(state.frame_stack->peek_op());
            is_patt = Bstring_tag_patt(string_ptr);
            break;
        }
        case PattSexp: {
            auto sexp_ptr = reinterpret_cast<void*>(state.frame_stack->peek_op());
            is_patt = Bsexp_tag_patt(sexp_ptr);
            break;
        }
        default: {
            throw std::runtime_error("Invalid patt instruction lower bits.");
        }
    }
    state.frame_stack->pop_op();
    state.frame_stack->push_op(is_patt);
}