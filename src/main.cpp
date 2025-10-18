#include <iostream>

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
#include "../runtime/runtime.h"

[[noreturn]] void interpret(bytefile *bf) {
    auto decoder = InstructionDecoder(bf->code_ptr);
    auto state = StackMachineState(bf);
    while (true) {
        switch (decoder.next_instruction_type()) {
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
            case STRING:
            case STI:
            case STA:
            case RET:
            case SWAP:
            case LDA:
            case CLOSURE:
            case CALLC:
            case PATT:
                failure("not implemented");
            default:
                failure("unknown");
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "File is not given");
        exit(1);
    }
    bytefile *bf = read_file(argv[1]);
    interpret(bf);
}