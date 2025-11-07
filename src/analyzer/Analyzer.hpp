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

#pragma pack(push, 1)
    struct InstructionElement {
        uint8_t tag; // also encodes type: 0, 1, 2
        union {
            struct { uint32_t a; uint32_t b; } two;
            struct { uint32_t a; } one;
            struct { } none;
        } data;
    };

struct Sequence {
    uint8_t count;   // 1 or 2
    InstructionElement elems[2];
};

#pragma pack(pop)

bool operator<(const Sequence& lhs, const Sequence& rhs) {
    // Compare binary memory since structs are tightly packed
    return std::memcmp(&lhs, &rhs, sizeof(Sequence)) < 0;
}

bool operator==(const Sequence& lhs, const Sequence& rhs) {
    return std::memcmp(&lhs, &rhs, sizeof(Sequence)) == 0;
}

class Analyzer {

    std::map<Sequence, int> sequences_counter;

    InstructionElement prev_elem;
    bool prev_initialized = false;

    void add_instruction(InstructionElement instruction) {
        Sequence seq1{};
        seq1.count = 1;
        seq1.elems[0] = instruction;
        sequences_counter[seq1]++;

        if (prev_initialized) {
            Sequence seq2{};
            seq2.count = 2;
            seq2.elems[0] = prev_elem;
            seq2.elems[1] = instruction;
            sequences_counter[seq2]++;
        }

        prev_initialized = true;
        prev_elem = instruction;
    }

    std::unordered_set<int> visited_labels;
    std::queue<int> labels;

    void process_label(int label) {

        prev_initialized = false;

        auto cur_ptr = bf->code_ptr + label;

        bool traverse = true;

        while (traverse) {
            auto raw_instruction_type = static_cast<InstructionType>(*cur_ptr);

            if (visited_labels.contains(cur_ptr - bf->code_ptr)) {
                return;
            }

            visited_labels.insert(cur_ptr - bf->code_ptr);

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

            InstructionElement elem{};
            elem.tag = static_cast<uint8_t>(instruction_type);

            // std::cerr << label << " " << magic_enum::enum_name(static_cast<InstructionType>(elem.tag)) << std::endl;

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
                case STA: {
                    cur_ptr += 1;
                    add_instruction(elem);
                    break;
                }
                case RET:
                case END: {
                    cur_ptr += 1;
                    add_instruction(elem);
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
                    auto arg = *reinterpret_cast<const uint32_t*>(cur_ptr + 1);
                    cur_ptr += 1 + sizeof(uint32_t);
                    elem.data.one.a = arg;
                    add_instruction(elem);

                    if (instruction_type == JMP) {
                        cur_ptr = bf->code_ptr + arg;
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
                    auto arg1 = *reinterpret_cast<const uint32_t*>(cur_ptr + 1);
                    auto arg2 = *reinterpret_cast<const uint32_t*>(cur_ptr + 1 + sizeof(uint32_t));
                    elem.data.two.a = arg1;
                    elem.data.two.b = arg2;
                    add_instruction(elem);

                    cur_ptr += 1 + 2 * sizeof(uint32_t);

                    if ((instruction_type == CALL || instruction_type == CLOSURE)) {
                        labels.push(arg1);
                    }
                    break;
                }
                case BINOP:
                case PATT: {
                    elem.data.one.a = static_cast<uint32_t>(raw_instruction_type);
                    add_instruction(elem);
                    cur_ptr += 1;
                    break;
                }
                case LD:
                case ST:
                case LDA: {
                    auto arg = *reinterpret_cast<const uint32_t*>(cur_ptr + 1);
                    elem.data.two.a = static_cast<uint32_t>(raw_instruction_type);
                    elem.data.two.b = arg;
                    add_instruction(elem);
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
    }

    void results() {
        std::vector<std::pair<Sequence, int>> items(sequences_counter.begin(), sequences_counter.end());

        // Сортируем по значению по убыванию
        std::sort(items.begin(), items.end(),
            [](const auto& a, const auto& b) {
                return a.second > b.second; // > для убывания
            });

        for (auto [seq, count] : items) {
            for (int i = 0; i < seq.count; i++) {
                auto inst = seq.elems[i];
                auto type = static_cast<InstructionType>(inst.tag);
                std::cout << magic_enum::enum_name(type);

                switch (type) {
                    case JMP:
                    case CJMPz:
                    case CJMPnz:
                    case CONST:
                    case CALL_ARRAY:
                    case LINE:
                    case ARRAY:
                    case STRING:
                    case CALLC:
                    case BINOP:
                    case PATT: {
                        // SimpleInstructionWithArgs<1>
                        std::cout << " " << inst.data.one.a;
                        break;
                    }
                    case CALL:
                    case CLOSURE:
                    case SEXP:
                    case TAG:
                    case BEGIN:
                    case CBEGIN:
                    case FAIL:
                    case LD:
                    case ST:
                    case LDA: {
                        // SimpleInstructionWithArgs<2>
                        std::cout << " " << inst.data.two.a;
                        std::cout << " " << inst.data.two.b;
                        break;
                    }
                    default:
                        break;
                }

                if (i + 1 != seq.count) {
                    std::cout << "; ";
                }
            }
            std::cout << " : " << count << std::endl;
        }
    }
};
