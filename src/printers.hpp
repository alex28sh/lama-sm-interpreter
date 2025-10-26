#pragma once

#include <iomanip>
#include <magic_enum/magic_enum.hpp>
#include "bytefile.h"

inline void print_instructions(bytefile *bf) {
    auto instruction_decoder = InstructionDecoder(bf->code_ptr);

    InstructionType instruction_type;
    while (true) {

        std::cout << "0x"
                  << std::setw(8) << std::setfill('0') << std::hex
                  << instruction_decoder.code_ptr - bf->code_ptr
                  << ":\t" << std::dec;

        instruction_type = instruction_decoder.next_instruction_type();
        switch (instruction_type) {
            case CALL_READ:
            case CALL_WRITE:
            case CALL_LENGTH:
            case CALL_STRING:
            case ELEM:
            case DUP:
            case DROP:
            case SWAP:
            case END: {
                instruction_decoder.consume_as<NoArgsInstruction>();
                std::cout << magic_enum::enum_name(instruction_type) << std::endl;
                break;
            }
            case JMP:
            case CJMPz:
            case CJMPnz: {
                auto inst = instruction_decoder.consume_as<SimpleInstructionWithArgs<1>>();
                std::cout << magic_enum::enum_name(instruction_type) << " ";

                std::cout << "0x"
                          << std::setw(8) << std::setfill('0') << std::hex
                          << inst.args[0] << std::dec << std::endl;

                break;
            }
            case CONST:
            case CALL_ARRAY:
            case LINE:
            case ARRAY:
            case STRING: {
                // SimpleInstructionWithArgs<1>
                auto inst = instruction_decoder.consume_as<SimpleInstructionWithArgs<1>>();
                std::cout << magic_enum::enum_name(instruction_type) << " ";
                std::cout << inst.args[0] << std::endl;
                break;
            }
            case CALL: {
                // SimpleInstructionWithArgs<2>
                auto inst = instruction_decoder.consume_as<SimpleInstructionWithArgs<2>>();
                std::cout << magic_enum::enum_name(instruction_type) << " ";
                std::cout << "0x"
                          << std::setw(8) << std::setfill('0') << std::hex
                          << inst.args[0] << std::dec
                          << " " << inst.args[1] << std::endl;
                break;
            }
            case SEXP:
            case TAG: {
                // SimpleInstructionWithArgs<2>
                auto inst = instruction_decoder.consume_as<SimpleInstructionWithArgs<2>>();
                std::cout << magic_enum::enum_name(instruction_type) << " ";
                std::cout << get_string(bf, inst.args[0]) << "(" << inst.args[0] << ") " << inst.args[1] << std::endl;
                break;
            }
            case BEGIN:
            case FAIL: {
                // SimpleInstructionWithArgs<2>
                auto inst = instruction_decoder.consume_as<SimpleInstructionWithArgs<2>>();
                std::cout << magic_enum::enum_name(instruction_type) << " ";
                std::cout << inst.args[0] << " " << inst.args[1] << std::endl;
                break;
            }
            case BINOP: {
                // InstructionWithArgsLowerBits<0>
                auto inst = instruction_decoder.consume_as<InstructionWithArgsLowerBits<0>>();
                auto low_bits_inst = static_cast<BinOp>(low_bits(inst.instruction));
                // std::cout << magic_enum::enum_name(instruction_type) << " ";
                std::cout << magic_enum::enum_name(low_bits_inst) << std::endl;
                break;
            }
            case LD:
            case ST: {
                // InstructionWithArgsLowerBits<1>
                auto inst = instruction_decoder.consume_as<InstructionWithArgsLowerBits<1>>();
                auto low_bits_inst = static_cast<MemVar>(low_bits(inst.instruction));
                std::cout << magic_enum::enum_name(instruction_type) << " ";
                std::cout << magic_enum::enum_name(low_bits_inst) << " ";
                std::cout << inst.args[0] << std::endl;
                break;
            }
            case STI:
            case STA:
            case RET:
            case LDA:
            case CLOSURE:
            case CALLC:
            case PATT:
                throw std::runtime_error(fmt::format("not implemented {}", magic_enum::enum_name(instruction_type)));
            default:
                if (*instruction_decoder.code_ptr == static_cast<char>(0xFF)) {
                    std::cout << "<end>" << std::endl;
                    return;
                }
                throw std::runtime_error("unknown");
        }
    }
}
