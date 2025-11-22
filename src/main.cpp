#include <csignal>
#include <cstring>
#include <iostream>

#include "Analyzer.hpp"
#include "binop_interpreter.hpp"
#include "boxing_interpreters.hpp"
#include "builtin_interpreters.hpp"
#include "bytefile.h"
#include "call_stack_interpreters.hpp"
#include "jump_interpreters.hpp"
#include "helper_interpreters.hpp"
#include "StackMachineState.hpp"
#include "InstructionDecoder.hpp"
#include "mem_interpreters.hpp"
#include "stack_ops_interpreters.hpp"
#include "unboxing_interpreters.hpp"
#include "closures.hpp"
#include "../runtime/runtime.h"
#include "printers.hpp"

StackMachineState* external_state;

void segfaultHandler(int signal) {
    std::cerr << "Caught SIGSEGV!" << std::endl;
    std::cerr << "index: 0x"
              << std::setw(8) << std::setfill('0') << std::hex
              << external_state->instruction_decoder->code_ptr - external_state->bf->code_ptr - 1
              << "\n" << std::dec;
    std::cerr << magic_enum::enum_name(external_state->instruction_decoder->next_instruction_type()) << std::endl;
    exit(1);
}

void interpret(bytefile *bf) {
    auto state = StackMachineState(bf);
    external_state = &state;
    state.frame_stack->push_stack_frame(0, nullptr);

    signal(SIGSEGV, segfaultHandler);
    while (true) {
        InstructionType inst;
        bool is_correct_inst = false;
        try {
            inst = state.instruction_decoder->next_instruction_type();
            is_correct_inst = true;
            switch (inst) {
                case BINOP: {
                    binop_interpeter(state);
                    break;
                }
                case CONST: {
                    stack_const(state);
                    break;
                }
                case SEXP: {
                    boxing_sexp(state);
                    break;
                }
                case JMP: {
                    jump(state);
                    break;
                }
                case CJMPz: {
                    jump_if_zero(state);
                    break;
                }
                case CJMPnz: {
                    jump_if_not_zero(state);
                    break;
                }
                case DROP: {
                    stack_drop(state);
                    break;
                }
                case DUP: {
                    stack_dup(state);
                    break;
                }
                case LD: {
                    mem_load(state);
                    break;
                }
                case ST: {
                    mem_store(state);
                    break;
                }
                case CALL_READ: {
                    call_read(state);
                    break;
                }
                case CALL_WRITE: {
                    call_write(state);
                    break;
                }
                case CALL_LENGTH: {
                    call_length(state);
                    break;
                }
                case CALL_STRING: {
                    call_string(state);
                    break;
                }
                case CALL_ARRAY: {
                    call_array(state);
                    break;
                }
                case CBEGIN:
                case BEGIN: {
                    call_stack_begin(state);
                    break;
                }
                case CALL: {
                    call_stack_call(state);
                    break;
                }
                case END: {
                    call_stack_end(state);
                    break;
                }
                case ELEM: {
                    call_elem(state);
                    break;
                }
                case TAG: {
                    unboxing_tag(state);
                    break;
                }
                case ARRAY: {
                    unboxing_array(state);
                    break;
                }
                case FAIL: {
                    helper_fail(state);
                    break;
                }
                case LINE: {
                    helper_line(state);
                    break;
                }
                case STRING: {
                    boxing_string(state);
                    break;
                }
                case SWAP: {
                    stack_swap(state);
                    break;
                }
                case STA: {
                    stack_sta(state);
                    break;
                }
                case STI: {
                    stack_sti(state);
                    break;
                }
                case LDA: {
                    stack_lda(state);
                    break;
                }
                case CLOSURE: {
                    closures_create(state);
                    break;
                }
                case CALLC: {
                    closures_call(state);
                    break;
                }
                case PATT: {
                    patt(state);
                    break;
                }
                case RET:
                default:
                    is_correct_inst = false;
                    throw std::runtime_error("Unknown instruction");
            }
        } catch (const std::runtime_error& e) {
            std::cerr << "index: 0x"
                      << std::setw(8) << std::setfill('0') << std::hex
                      << state.instruction_decoder->code_ptr - bf->code_ptr - 1
                      << "\n" << std::dec;
            if (is_correct_inst) {
                std::cerr << magic_enum::enum_name(inst) << std::endl;
            }
            std::cerr << e.what() << std::endl;
            exit(1);
        } catch (...) {
            std::cerr << "index: 0x"
                      << std::setw(8) << std::setfill('0') << std::hex
                      << state.instruction_decoder->code_ptr - bf->code_ptr - 1
                      << "\n" << std::dec;
            if (is_correct_inst) {
                std::cerr << magic_enum::enum_name(inst) << std::endl;
            }
            std::cerr << "Unknown exception occurred" << std::endl;
            exit(1);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "File is not given");
        exit(1);
    }
    bytefile *bf = read_file(argv[2]);
    if (std::strcmp(argv[1], "-a") == 0) {
        auto analyzer = Analyzer(bf);
        analyzer.collect_marks();
        analyzer.analyze();
        analyzer.results();
    } else if (std::strcmp(argv[1], "-i") == 0) {
        interpret(bf);
    } else if (std::strcmp(argv[1], "-p") == 0) {
        print_instructions(bf);
    } else {
        throw std::runtime_error(fmt::format("wrong argument {}", argv[1]));
    }
}