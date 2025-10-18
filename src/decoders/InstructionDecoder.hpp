//
// Created by aleksandr on 12.10.25.
//

#pragma once
#include "InstructionType.hpp"

constexpr char high_bits(char instruction) {
    return static_cast<char>((instruction & 0xF0) >> 4);
}

constexpr char low_bits(char instruction) {
    return static_cast<char>(instruction & 0x0F);
}

class InstructionDecoder {

public:
    const char* code_ptr;

    explicit InstructionDecoder(const char* code_ptr) : code_ptr(code_ptr) {}


    [[nodiscard]] InstructionType next_instruction_type() const {
        auto raw_instruction_type = static_cast<InstructionType>(*code_ptr);

        switch (high_bits(raw_instruction_type)) {
            case high_bits(BINOP) :
                return BINOP;
            case high_bits(LD) :
                return LD;
            case high_bits(LDA) :
                return LDA;
            case high_bits(ST) :
                return ST;
            case high_bits(PATT) :
                return PATT;
            default:
                return raw_instruction_type;
        }
    }

    template<class Command>
    const Command &consume_as() {
        auto command = reinterpret_cast<const Command *>(code_ptr);
        code_ptr += command->length();
        return *command;
    }

    template<class Command>
    const Command &view_as() {
        return *reinterpret_cast<const Command *>(code_ptr);
    }
};