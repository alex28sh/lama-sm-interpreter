#pragma once

#include "Analyzer.hpp"
#include <sstream>

uint32_t get_new_stack(uint32_t cur_stack, uint32_t symbol_offset, InstructionType instruction_type) {
    switch (instruction_type) {
        case BINOP:
            return cur_stack - 1;
        case CONST:
            return cur_stack + 1;
        case SEXP: {
            auto arg_num = *reinterpret_cast<const uint32_t*>(bf->code_ptr + symbol_offset + 1 + sizeof(uint32_t));
            return cur_stack + 1 - arg_num;
        }
        case JMP:
            return cur_stack;
        case CJMPz:
            return cur_stack - 1;
        case CJMPnz:
            return cur_stack - 1;
        case DROP:
            return cur_stack - 1;
        case DUP:
            return cur_stack + 1;
        case LD: {
            return cur_stack + 1;
        }
        case ST: {
            return cur_stack - 1;
        }
        case CALL_READ:
            return cur_stack + 1;
        case CALL_WRITE:
            return cur_stack - 1;
        case CALL_LENGTH:
            return cur_stack;
        case CALL_STRING:
            return cur_stack;
        case CALL_ARRAY: {
            auto arg_num = *reinterpret_cast<const uint32_t*>(bf->code_ptr + symbol_offset + 1);
            return cur_stack + 1 - arg_num;
        }
        case CBEGIN:
        case BEGIN: {
            return cur_stack;
        }
        case CALL: {
            auto arg_num = *reinterpret_cast<const uint32_t*>(bf->code_ptr + symbol_offset + 1);
            return cur_stack + 1 - arg_num;
        }
        case END:
            return cur_stack;
        case ELEM:
            return cur_stack - 1;
        case TAG:
            return cur_stack;
        case ARRAY: {
            auto arg_num = *reinterpret_cast<const uint32_t*>(bf->code_ptr + symbol_offset + 1);
            return cur_stack + 1 - arg_num;
        }
        case FAIL:
            return cur_stack;
        case LINE:
            return cur_stack;
        case STRING:
            return cur_stack + 1;
        case SWAP:
            return cur_stack;
        case STA:
            return cur_stack - 2;
        case STI:
            return cur_stack - 1;
        case LDA:
            return cur_stack + 1;
        case CLOSURE:
            return cur_stack + 1;
        case CALLC: {
            auto arg_num = *reinterpret_cast<const uint32_t*>(bf->code_ptr + symbol_offset + 1);
            return cur_stack - arg_num;
        }
        case PATT: {
            auto low_bits_inst = static_cast<Patt>(low_bits(instruction_type));

            switch(low_bits_inst) {
                case PattStrCmp:
                    return cur_stack - 1;
                case PattClosure:
                case PattBoxed:
                case PattUnBoxed:
                case PattArray:
                case PattString:
                case PattSexp:
                    return cur_stack;
                default: {
                    throw std::runtime_error("Invalid patt instruction lower bits.");
                }
            }
        }
        default:
            throw std::runtime_error("Uncreachable");
    }
}

void collect_marks(bytefile *bf, std::vector<int32_t> &visited) {
    std::queue<uint32_t> labels;
    for (int i = 0; i < bf->public_symbols_number; i++) {
        auto symbol_offset = get_public_offset(bf, i);
        if (strcmp(get_public_name(bf, i), "main") != 0) {
            continue;
        }
        if (visited[symbol_offset] == -1) {
            labels.push(symbol_offset);
            visited[symbol_offset] = 0;
        }
    }

    while (!labels.empty()) {
        auto symbol_offset = labels.front();
        labels.pop();
        auto cur_stack = visited[symbol_offset];

        while (true) {
            auto [nxt, instruction_type] = general_byterun(symbol_offset);
            if (instruction_type == CALLC || instruction_type == ARRAY) {
                auto arg_num = *reinterpret_cast<const uint32_t*>(bf->code_ptr + symbol_offset + 1);
                if (arg_num > cur_stack) {
                    std::ostringstream oss; 
                    oss << "argument number of instruction at offset " << symbol_offset << " is too large"; 
                    throw std::runtime_error(oss.str());
                }
            } else if (instruction_type == SEXP || instruction_type == CALL) {
                auto arg_num = *reinterpret_cast<const uint32_t*>(bf->code_ptr + symbol_offset + 1 + sizeof(uint32_t));
                if (arg_num > cur_stack) {
                    std::ostringstream oss; 
                    oss << "argument number of instruction at offset " << symbol_offset << " is too large"; 
                    throw std::runtime_error(oss.str());
                }
            }

            auto new_stack = get_new_stack(cur_stack, symbol_offset, instruction_type);
            if (new_stack < 0) {
                std::ostringstream oss; 
                oss << "Stack is exhausted at " << symbol_offset; 
                throw std::runtime_error(oss.str());
            }

            auto perform_step = [&](const uint32_t nxt_offset) {
                if (visited[nxt_offset] == -1) {
                    visited[new_stack] = cur_stack;
                    labels.push(new_stack);
                } else if (visited[nxt_offset] != cur_stack) {
                    std::ostringstream oss; 
                    oss << "Stack at " << nxt_offset << " differs"; 
                    throw std::runtime_error(oss.str());
                }
            };

            if (instruction_type == CJMPz || instruction_type == CJMPnz || instruction_type == CALL) {
                auto label_arg = *reinterpret_cast<const uint32_t*>(bf->code_ptr + symbol_offset + 1);
                perform_step(label_arg);
                perform_step(nxt);
                break;
            }
            if (instruction_type == JMP) {
                auto label_arg = *reinterpret_cast<const uint32_t*>(bf->code_ptr + symbol_offset + 1);
                perform_step(label_arg);
                break;
            }

            if (instruction_type == END || instruction_type == FAIL || instruction_type == RET) {
                break;
            }
            if (visited[nxt] == -1) {
                visited[nxt] = cur_stack;
                symbol_offset = nxt;
            } else if (visited[nxt] != cur_stack) {
                std::ostringstream oss; 
                oss << "Stack at " << nxt << " differs"; 
                throw std::runtime_error(oss.str());
            } else {
                break;
            }
        }
    }
}

void validate_variable(
    bytefile *bf,
    MemVar designation,
    uint32_t index,
    uint32_t args,
    uint32_t locals
) {
    switch (designation) {
        case Global:
            if (index >= bf->global_area_size) {
                std::ostringstream oss; 
                oss << "Global variable " << index << " out of bounds"; 
                throw std::runtime_error(oss.str());
            }
            break;
        case Local:
            if (index >= locals) {
                std::ostringstream oss; 
                oss << "Local variable " << index << " out of bounds"; 
                throw std::runtime_error(oss.str());
            }
            break;
        case Argument:
            if (index >= args) {
                std::ostringstream oss; 
                oss << "Argument " << index << " out of bounds"; 
                throw std::runtime_error(oss.str());
            }
            break;
        case Closure:
            break;
        default:
            {
                std::ostringstream oss; 
                oss << "Unknown designation " << char(designation); 
                throw std::runtime_error(oss.str());
            }
    }
}

void verify(bytefile *bf) {
    std::vector<int32_t> visited(bf->code_size);
    collect_marks(bf, visited);

    uint32_t symbol_offset = 0;
    int32_t max_stack = 0;

    std::vector<uint32_t> local_sizes, arg_sizes, begin_offsets;

    while (symbol_offset < bf->code_size) {
        auto [nxt, instruction_type] = general_byterun(symbol_offset);
        if (visited[symbol_offset] == -1) {
            symbol_offset = nxt;
            continue;
        }
        max_stack = std::max(max_stack, visited[symbol_offset]);

        if (instruction_type == END) {

            auto begin_offset = begin_offsets.back() + 1;
            if (reinterpret_cast<uint16_t *>(bf->code_ptr + begin_offset)[1] != 0) {
                std::ostringstream oss; 
                oss << "Could not encode max_stack: too many arguments at the basic block " << symbol_offset; 
                throw std::runtime_error(oss.str());
            }
            reinterpret_cast<uint16_t *>(bf->code_ptr + begin_offset)[1] = max_stack;

            begin_offsets.pop_back();
            local_sizes.pop_back();
            arg_sizes.pop_back();
            max_stack = -1;
        } else if (instruction_type == BEGIN || instruction_type == CBEGIN) {
            auto n_args = *reinterpret_cast<const uint32_t*>(bf->code_ptr + symbol_offset + 1);
            auto n_locals = *reinterpret_cast<const uint32_t*>(bf->code_ptr + symbol_offset + 1 + sizeof(uint32_t));
            local_sizes.push_back(n_locals);
            arg_sizes.push_back(n_args);
            begin_offsets.push_back(symbol_offset);
        } else if (instruction_type == LD || instruction_type == ST || instruction_type == LDA) {
            auto arg = *reinterpret_cast<const uint32_t*>(bf->code_ptr + symbol_offset + 1);
            validate_variable(bf, static_cast<MemVar>(low_bits(instruction_type)), arg, arg_sizes.back(), local_sizes.back());
        } else if (instruction_type == CLOSURE) {

            auto ptr =bf->code_ptr + symbol_offset + 1 + sizeof(uint32_t);
            auto n_args = *reinterpret_cast<const uint32_t*>(ptr);
            ptr += sizeof(uint32_t);

            Designations designations = {};

            for (uint32_t i = 0; i < n_args; i++) {
                auto designation = reinterpret_cast<const Designation *>(ptr);
                ptr += designation->length();
                validate_variable(bf, designation->mem_var, designation->value, arg_sizes.back(), local_sizes.back());
            }

        } else if (instruction_type == CALL || instruction_type == JMP ||
            instruction_type == CJMPz || instruction_type == CJMPnz) {
            auto jump = *reinterpret_cast<const uint32_t*>(bf->code_ptr + symbol_offset + 1);
            if (jump >= bf->code_size) {
                std::ostringstream oss; 
                oss << "Jump at " << jump << " outside of code section " << bf->code_size; 
                throw std::runtime_error(oss.str());
            }
        }
        symbol_offset = nxt;
    }
}