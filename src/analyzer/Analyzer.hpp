#pragma once
#include <iomanip>
#include <queue>
#include <vector>
#include <magic_enum/magic_enum.hpp>

#include "binop_interpreter.hpp"
#include "InstructionDecoder.hpp"
#include "InstructionDecodersByArgsNumber.hpp"
#include "mem_interpreters.hpp"

bytefile* bf;

InstructionType get_instruction_type(char* ptr) {
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
    return instruction_type;
}

std::pair<uint32_t, InstructionType> general_byterun(uint32_t i) {
    auto p = bf->code_ptr + i;
    auto type = get_instruction_type(p);

    switch (type) {
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
        case RET:
        case BINOP:
        case PATT:
        case END:
            return std::make_pair(i + 1, type);
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
            return std::make_pair(i + 1 + sizeof(uint32_t), type);
        }
        case CALL:
        case CLOSURE:
        case SEXP:
        case TAG:
        case BEGIN:
        case CBEGIN:
        case FAIL: {
            return std::make_pair(i + 1 + 2 * sizeof(uint32_t), type);
        }
        default: throw std::logic_error("unsupported instruction type");
    }
}

int compare_bytes(int st1, int end1, int st2, int end2) {
    for (int i = 0; i < std::min(end1 - st1, end2 - st2); i++) {
        auto b1 = bf->code_ptr[st1 + i];
        auto b2 = bf->code_ptr[st2 + i];
        if (b1 != b2) {
            return b1 - b2;
        }
    }
    return (end1 - st1) - (end2 - st2);
}

bool cmp_single(uint32_t i1, uint32_t i2) {
    auto [nxt1, type1] = general_byterun(i1);
    auto [nxt2, type2] = general_byterun(i2);

    return compare_bytes(i1, nxt1, i2, nxt2) < 0;
}

bool cmp_pair(uint32_t i1, uint32_t i2) {

    auto nxt1 = general_byterun(i1).first;
    auto nxt2 = general_byterun(i2).first;

    auto cmp1 = compare_bytes(i1, nxt1, i2, nxt2);
    if (cmp1 == 0) {
        return cmp_single(nxt1, nxt2);
    }
    return cmp1 < 0;
}

class Analyzer {

    // std::map<Sequence, int> sequences_counter;
    std::vector<uint32_t> collect_single, collect_pairs;
    std::vector<std::pair<uint32_t, uint32_t>> counter;

    char* prev_ptr;
    bool prev_initialized = false;

    void add_instruction(char* cur_ptr) {
        collect_single.push_back(cur_ptr - bf->code_ptr);

        if (prev_initialized) {
            collect_pairs.push_back(prev_ptr - bf->code_ptr);
        }

        prev_initialized = true;
        prev_ptr = cur_ptr;
    }

    std::vector<bool> visited;
    std::queue<int> labels;

    void check_nxt(uint32_t idx, bool &traverse) {

        if (visited[idx]) {

            traverse = false;

            // prev_initialized is always true
            // if not visited will be encountered later
            collect_pairs.push_back(prev_ptr - bf->code_ptr);
        }

        visited[idx] = true;
    }

    void process_label(int label) {
        // std::cerr << label << std::endl;

        prev_initialized = false;

        auto cur_ptr = bf->code_ptr + label;

        bool traverse = true;

        while (traverse) {

            // auto instruction_type = get_instruction_type(cur_ptr).first;
            auto [nxt, instruction_type] = general_byterun(cur_ptr - bf->code_ptr);
            add_instruction(cur_ptr);

            switch (instruction_type) {
                case RET:
                case END: {
                    traverse = false;
                    break;
                }
                case JMP:
                case CJMPz:
                case CJMPnz: {
                    auto arg = *reinterpret_cast<const uint32_t*>(cur_ptr + 1);

                    if (instruction_type == JMP) {
                        traverse = false;
                        if (!visited[arg]) {
                            labels.push(arg);
                            visited[arg] = true;
                        }
                    } else {
                        if (!visited[arg]) {
                            labels.push(arg);
                            visited[arg] = true;
                        }
                        check_nxt(nxt, traverse);
                    }
                    break;
                }
                case CALL:
                case CLOSURE:
                case FAIL: {
                    auto arg1 = *reinterpret_cast<const uint32_t*>(cur_ptr + 1);

                    if ((instruction_type == CALL || instruction_type == CLOSURE) && !visited[arg1]) {
                        labels.push(arg1);
                        visited[arg1] = true;
                        check_nxt(nxt, traverse);
                    } else if (instruction_type == FAIL) {
                        traverse = false;
                    }
                    break;
                }
                default: {
                    check_nxt(nxt, traverse);
                    break;
                }
            }

            cur_ptr = bf->code_ptr + nxt;
        }
    }

public:

    explicit Analyzer(bytefile* bf_) : visited(bf_->code_size, false) {
        bf = bf_;
    }

    void analyze() {
        for (int i = 0; i < bf->public_symbols_number; i++) {
            auto symbol_offset = get_public_offset(bf, i);
            labels.push(symbol_offset);
            visited[symbol_offset] = true;
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
            counter.push_back(std::make_pair(j - i, i << 1));
            // auto p = bf->code_ptr + collect_single[i];
            // print_instruction(p);
            // std::cout << cmp_single(collect_single[i], collect_single[j]) << std::endl;
            // std::cout << "A\n";
            i = j - 1;
        }

        for (int i = 0; i < collect_pairs.size(); i++) {
            int j = i;
            while (j < collect_pairs.size() && !cmp_pair(collect_pairs[i], collect_pairs[j])) {
                j++;
            }
            counter.push_back(std::make_pair(j - i, 1 | (i << 1)));
            i = j - 1;
        }

        sort(counter.begin(), counter.end());
        reverse(counter.begin(), counter.end());
    }

    void print_instruction(char* &ptr) {

        auto raw_instruction_type = static_cast<InstructionType>(*ptr);
        auto [nxt, instruction_type] = general_byterun(ptr - bf->code_ptr);
        std::cout << magic_enum::enum_name(instruction_type);

        ptr++;
        switch (instruction_type) {
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
                break;
            }
            case BINOP:
            case PATT: {
                // SimpleInstructionWithArgs<1>
                std::cout << " " << static_cast<uint32_t>(raw_instruction_type);
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
                break;
            }
            case LD:
            case ST:
            case LDA: {
                std::cout << " " << static_cast<uint32_t>(raw_instruction_type);
                std::cout << " " << *reinterpret_cast<uint32_t*>(ptr);
                break;
            }
            default:
                break;
        }
        ptr = bf->code_ptr + nxt;
    }

    void results() {
        // std::cerr << counter.size() << std::endl;
        for (auto [count, s] : counter) {
            auto i = s >> 1;
            auto is_two = (s & 1) != 0;
            auto ptr = bf->code_ptr;
            if (!is_two) {
                ptr += collect_single[i];
            } else {
                ptr += collect_pairs[i];
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
