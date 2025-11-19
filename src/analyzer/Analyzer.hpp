#pragma once
#include <map>
#include <queue>
#include <unordered_set>
#include <vector>
#include <magic_enum/magic_enum.hpp>

#include "binop_interpreter.hpp"
#include "InstructionDecoder.hpp"
#include "InstructionDecodersByArgsNumber.hpp"
#include "mem_interpreters.hpp"

std::pair<InstructionType, InstructionType> get_instruction_type(char* ptr) {
    auto raw_instruction_type = static_cast<InstructionType>(*ptr);
    InstructionType instruction_type;
    switch (high_bits(raw_instruction_type)) {
        case 0x00 :
            instruction_type = BINOP;
            break;
        case 0x20 :
            instruction_type = LD;
            break;
        case 0x30 :
            instruction_type = LDA;
            break;
        case 0x40 :
            instruction_type = ST;
            break;
        case 0x60 :
            instruction_type = PATT;
            break;
        default:
            instruction_type = raw_instruction_type;
            break;
    }
    return {instruction_type, raw_instruction_type};
}

bool cmp_single(char* p1, char* p2) {
    auto [type1, type1_raw] = get_instruction_type(p1);
    auto [type2, type2_raw] = get_instruction_type(p2);
    if (type1_raw != type2_raw) {
        return type1_raw < type2_raw;
    }

    // std::cerr << magic_enum::enum_name(type1) << std::endl;

    switch (type1) {
        case CALL_READ:
        case CALL_WRITE:
        case CALL_LENGTH:
        case CALL_STRING:
        case ELEM:
        case DUP:
        case DROP:
        case SWAP:
        case STI:
        case STA:
        case BINOP:
        case PATT:
        case END:
            return false;
        case JMP:
        case CJMPz:
        case CJMPnz:
        case CONST:
        case CALL_ARRAY:
        case ARRAY:
        case STRING:
        case CALLC:
        case LD:
        case ST:
        case LDA:
        case LINE: {
            auto arg1 = *reinterpret_cast<const uint32_t*>(p1 + 1);
            auto arg2 = *reinterpret_cast<const uint32_t*>(p2 + 1);
            return arg1 < arg2;
        }
        case CALL:
        case CLOSURE:
        case SEXP:
        case TAG:
        case BEGIN:
        case CBEGIN:
        case FAIL: {
            auto arg1 = *reinterpret_cast<const uint32_t*>(p1 + 1);
            auto arg2 = *reinterpret_cast<const uint32_t*>(p2 + 1);
            if (arg1 != arg2) {
                return arg1 < arg2;
            }
            arg1 = *reinterpret_cast<const uint32_t*>(p1 + 1 + sizeof(uint32_t));
            arg2 = *reinterpret_cast<const uint32_t*>(p2 + 1 + sizeof(uint32_t));
            return arg1 < arg2;
        }
        default: throw std::logic_error("unsupported instruction type: 1");
    }
}

bool cmp_pair(char* p1, char* p2) {

    auto [type1, type1_raw] = get_instruction_type(p1);
    auto [type2, type2_raw] = get_instruction_type(p2);
    if (type1_raw != type2_raw) {
        return type1_raw < type2_raw;
    }

    switch (type1) {
        case CALL_READ:
        case CALL_WRITE:
        case CALL_LENGTH:
        case CALL_STRING:
        case ELEM:
        case DUP:
        case DROP:
        case SWAP:
        case STI:
        case STA:
        case BINOP:
        case PATT:
        case END:
            return cmp_single(p1 + 1, p2 + 1);
        case JMP:
        case CJMPz:
        case CJMPnz:
        case CONST:
        case CALL_ARRAY:
        case ARRAY:
        case STRING:
        case CALLC:
        case LD:
        case ST:
        case LDA:
        case LINE: {
            auto arg1 = *reinterpret_cast<const uint32_t*>(p1 + 1);
            auto arg2 = *reinterpret_cast<const uint32_t*>(p2 + 1);
            if (arg1 == arg2) {
                return cmp_single(p1 + 1 + sizeof(uint32_t), p2 + 1 + sizeof(uint32_t));
            }
            return arg1 < arg2;
        }
        case CALL:
        case CLOSURE:
        case SEXP:
        case TAG:
        case BEGIN:
        case CBEGIN:
        case FAIL: {
            auto arg1 = *reinterpret_cast<const uint32_t*>(p1 + 1);
            auto arg2 = *reinterpret_cast<const uint32_t*>(p2 + 1);
            if (arg1 != arg2) {
                return arg1 < arg2;
            }
            arg1 = *reinterpret_cast<const uint32_t*>(p1 + 1 + sizeof(uint32_t));
            arg2 = *reinterpret_cast<const uint32_t*>(p2 + 1 + sizeof(uint32_t));
            if (arg1 != arg2) {
                return arg1 < arg2;
            }
            return cmp_single(p1 + 1 + 2 * sizeof(uint32_t), p2 + 1 + 2 * sizeof(uint32_t));
        }
        default: throw std::logic_error("unsupported instruction type: 2");
    }
}

class Analyzer {

    // std::map<Sequence, int> sequences_counter;
    std::vector<char*> collect_single, collect_pairs;
    std::vector<std::pair<uint32_t, std::pair<uint32_t, bool>>> counter;

    char* prev_ptr;
    bool prev_initialized = false;

    void add_instruction(char* cur_ptr) {
        collect_single.push_back(cur_ptr);

        if (prev_initialized) {
            collect_pairs.push_back(prev_ptr);
        }

        prev_initialized = true;
        prev_ptr = cur_ptr;
    }

    std::unordered_set<int> visited_labels;
    std::queue<int> labels;

    void process_label(int label) {
        // std::cerr << label << std::endl;

        prev_initialized = false;

        auto cur_ptr = bf->code_ptr + label;

        bool traverse = true;

        while (traverse) {

            if (visited_labels.contains(cur_ptr - bf->code_ptr)) {
                return;
            }

            visited_labels.insert(cur_ptr - bf->code_ptr);

            auto instruction_type = get_instruction_type(cur_ptr).first;
            // std::cerr << label << " " << magic_enum::enum_name(instruction_type) << std::endl;

            switch (instruction_type) {
                case CALL_READ:
                case CALL_WRITE:
                case CALL_LENGTH:
                case CALL_STRING:
                case ELEM:
                case DUP:
                case DROP:
                case SWAP:
                case STI:
                case STA:
                case BINOP:
                case PATT: {
                    add_instruction(cur_ptr);
                    cur_ptr += 1;
                    break;
                }
                case RET:
                case END: {
                    add_instruction(cur_ptr);
                    cur_ptr += 1;
                    traverse = false;
                    break;
                }
                case JMP:
                case CJMPz:
                case CJMPnz:
                case CONST:
                case CALL_ARRAY:
                case ARRAY:
                case STRING:
                case CALLC: {
                    add_instruction(cur_ptr);
                    auto arg = *reinterpret_cast<const uint32_t*>(cur_ptr + 1);
                    cur_ptr += 1 + sizeof(uint32_t);

                    if (instruction_type == JMP) {
                        traverse = false;
                        labels.push(arg);
                    } else if (instruction_type == CJMPnz || instruction_type == CJMPz) {
                        labels.push(arg);
                    }
                    break;
                }
                case CALL:
                case CLOSURE:
                case SEXP:
                case TAG:
                case BEGIN:
                case CBEGIN: {
                    add_instruction(cur_ptr);
                    auto arg1 = *reinterpret_cast<const uint32_t*>(cur_ptr + 1);

                    cur_ptr += 1 + 2 * sizeof(uint32_t);

                    if (instruction_type == CALL || instruction_type == CLOSURE) {
                        labels.push(arg1);
                    }
                    break;
                }
                case LD:
                case ST:
                case LDA: {
                    add_instruction(cur_ptr);
                    cur_ptr += 1 + sizeof(uint32_t);
                    break;
                }
                case LINE: {
                    cur_ptr += 1 + sizeof(uint32_t);
                    break;
                }
                case FAIL: {
                    cur_ptr += 1 + 2 * sizeof(uint32_t);
                    break;
                }
                default:
                    if (*cur_ptr == static_cast<char>(0xFF)) {
                        throw std::runtime_error("unreachable code");
                    }
                    throw std::runtime_error("invalid instruction type");
            }
        }
    }

public:

    bytefile* bf;

    explicit Analyzer(bytefile* bf) : bf(bf) {}

    void analyze() {
        for (int i = 0; i < bf->public_symbols_number; i++) {
            auto symbol_offset = get_public_offset(bf, i);
            labels.push(symbol_offset);
        }
        while (!labels.empty()) {
            auto label = labels.front();
            labels.pop();
            process_label(label);
        }

        sort(collect_single.begin(), collect_single.end(), cmp_single);
        sort(collect_pairs.begin(), collect_pairs.end(), cmp_pair);
        // for (int i = 0; i < collect_single.size(); i++) {
        //     print_instruction(collect_single[i]);
        // }

        for (int i = 0; i < collect_single.size(); i++) {
            int j = i;
            while (j < collect_single.size() && !cmp_single(collect_single[i], collect_single[j])) {
                j++;
            }
            counter.push_back(std::make_pair(j - i, std::make_pair(i, false)));
            i = j - 1;
        }

        for (int i = 0; i < collect_pairs.size(); i++) {
            int j = i;
            while (j < collect_pairs.size() && !cmp_pair(collect_pairs[i], collect_pairs[j])) {
                j++;
            }
            counter.push_back(std::make_pair(j - i, std::make_pair(i, true)));
            i = j - 1;
        }

        sort(counter.begin(), counter.end());
        reverse(counter.begin(), counter.end());
    }

    void print_instruction(char* &ptr) {

        auto type = get_instruction_type(ptr);
        std::cout << magic_enum::enum_name(type.first);

        ptr++;
        switch (type.first) {
            case JMP:
            case CJMPz:
            case CJMPnz:
            case CONST:
            case CALL_ARRAY:
            case LINE:
            case ARRAY:
            case STRING:
            case CALLC: {
                // SimpleInstructionWithArgs<1>
                std::cout << " " << *reinterpret_cast<uint32_t*>(ptr);
                ptr += sizeof(uint32_t);
                break;
            }
            case BINOP:
            case PATT: {
                // SimpleInstructionWithArgs<1>
                std::cout << " " << static_cast<uint32_t>(type.second);
                break;
            }
            case CALL:
            case CLOSURE:
            case SEXP:
            case TAG:
            case BEGIN:
            case CBEGIN:
            case FAIL: {
                // SimpleInstructionWithArgs<2>
                std::cout << " " << *reinterpret_cast<uint32_t*>(ptr);
                std::cout << " " << *reinterpret_cast<uint32_t*>(ptr + sizeof(uint32_t));
                ptr += 2 * sizeof(uint32_t);
                break;
            }
            case LD:
            case ST:
            case LDA: {
                std::cout << " " << static_cast<uint32_t>(type.second);
                std::cout << " " << *reinterpret_cast<uint32_t*>(ptr);
                ptr += sizeof(uint32_t);
                break;
            }
            default:
                break;
        }
    }

    void results() {
        // std::cerr << counter.size() << std::endl;
        for (auto [count, s] : counter) {
            auto [i, is_two] = s;
            char* ptr;
            if (!is_two) {
                ptr = collect_single[i];
            } else {
                ptr = collect_pairs[i];
            }
            print_instruction(ptr);
            if (is_two) {
                std::cout << "; ";
                print_instruction(ptr);
            }
            std::cout << ": " << count << std::endl;
        }
    }
};
