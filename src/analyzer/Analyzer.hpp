#pragma once
#include <map>
#include <vector>
#include <magic_enum/magic_enum.hpp>

#include "binop_interpreter.hpp"
#include "InstructionDecoder.hpp"
#include "InstructionDecodersByArgsNumber.hpp"
#include "mem_interpreters.hpp"

class Analyzer {

    struct Instruction {
        std::string instruction_type;
        std::vector<uint32_t> args;

        bool operator==(const Instruction&) const = default;
        auto operator<=>(const Instruction&) const = default;
    };

    std::vector<int> labels;
    std::map<int, std::map<std::vector<Instruction>, int>> sequences_counter;
    std::vector<Instruction> cur_sequence;

    void flush_instructions() {
        cur_sequence.clear();
    }

    void add_instruction(Instruction instruction) {
        cur_sequence.push_back(instruction);
        for (int i = 1; i <= std::min(2, (int)cur_sequence.size()); i++) {
            std::vector suffix(cur_sequence.end() - i, cur_sequence.end());
            if (!sequences_counter.contains(i)) {
                sequences_counter[i] = std::map<std::vector<Instruction>, int>();
            }
            sequences_counter[i][suffix]++;
        }
    }

    void collect_labels() {

        bool finish = false;
        while (!finish) {

            auto cur_offset = instruction_decoder.code_ptr - code_ptr;

            auto instruction_type = instruction_decoder.next_instruction_type();
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
                case RET:
                case END: {
                    instruction_decoder.consume_as<NoArgsInstruction>();
                    break;
                }
                case JMP:
                case CJMPz:
                case CJMPnz: {
                    auto inst = instruction_decoder.consume_as<SimpleInstructionWithArgs<1>>();
                    labels.push_back(inst.args[0]);
                    break;
                }
                case CONST:
                case CALL_ARRAY:
                case LINE:
                case ARRAY:
                case STRING:
                case CALLC: {
                    // SimpleInstructionWithArgs<1>
                    instruction_decoder.consume_as<SimpleInstructionWithArgs<1>>();
                    break;
                }
                case CALL:
                case CLOSURE: {
                    auto inst = instruction_decoder.consume_as<SimpleInstructionWithArgs<2>>();
                    labels.push_back(inst.args[0]);
                    break;
                }
                case SEXP:
                case TAG:
                case FAIL: {
                    instruction_decoder.consume_as<SimpleInstructionWithArgs<2>>();
                    break;
                }
                case BEGIN:
                case CBEGIN: {
                    // SimpleInstructionWithArgs<2>
                    instruction_decoder.consume_as<SimpleInstructionWithArgs<2>>();
                    labels.push_back(cur_offset);
                    break;
                }
                case BINOP:
                case PATT: {
                    instruction_decoder.consume_as<InstructionWithArgsLowerBits<0>>();
                    break;
                }
                case LD:
                case ST:
                case LDA: {
                    // InstructionWithArgsLowerBits<1>
                    instruction_decoder.consume_as<InstructionWithArgsLowerBits<1>>();
                    break;
                }
                default:
                    if (*instruction_decoder.code_ptr == static_cast<char>(0xFF)) {
                        finish = true;
                        labels.push_back(cur_offset);
                        break;
                    }
                    throw std::runtime_error("invalid instruction type");
            }

        }
        sort(labels.begin(), labels.end());
    }

    void collect_sequences() {

        InstructionType instruction_type;

        for (int i = 0; i < labels.size() - 1; ++i) {

            flush_instructions();

            instruction_decoder.code_ptr = code_ptr + labels[i];
            while (instruction_decoder.code_ptr < code_ptr + labels[i + 1]) {

                Instruction instruction;
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
                    case STI:
                    case STA:
                    case RET:
                    case END: {
                        instruction_decoder.consume_as<NoArgsInstruction>();
                        auto inst_type_str = std::string(magic_enum::enum_name(instruction_type));
                        instruction = {inst_type_str, {}};
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
                        // SimpleInstructionWithArgs<1>
                        auto inst = instruction_decoder.consume_as<SimpleInstructionWithArgs<1>>();
                        auto inst_type_str = std::string(magic_enum::enum_name(instruction_type));
                        instruction = {inst_type_str, std::vector {inst.args[0]}};
                        break;
                    }
                    case CALL:
                    case CLOSURE:
                    case SEXP:
                    case TAG:
                    case BEGIN:
                    case CBEGIN: {
                        // SimpleInstructionWithArgs<2>
                        auto inst = instruction_decoder.consume_as<SimpleInstructionWithArgs<2>>();
                        auto inst_type_str = std::string(magic_enum::enum_name(instruction_type));
                        instruction = {inst_type_str, std::vector {inst.args[0], inst.args[1]}};
                        break;
                    }
                    case BINOP:
                    case PATT: {
                        auto inst = instruction_decoder.consume_as<InstructionWithArgsLowerBits<0>>();
                        auto inst_type_str = std::string(magic_enum::enum_name(instruction_type));
                        instruction = {inst_type_str, std::vector {static_cast<uint32_t>(inst.get_low_bits())}};
                        break;
                    }
                    case LD:
                    case ST:
                    case LDA: {
                        // InstructionWithArgsLowerBits<1>
                        auto inst = instruction_decoder.consume_as<InstructionWithArgsLowerBits<1>>();
                        auto inst_type_str = std::string(magic_enum::enum_name(instruction_type));
                        instruction = {inst_type_str, std::vector {static_cast<uint32_t>(inst.get_low_bits()), inst.args[0]}};
                        break;
                    }
                    case LINE: {
                        // SimpleInstructionWithArgs<1>
                        instruction_decoder.consume_as<SimpleInstructionWithArgs<1>>();
                        break;
                    }
                    case FAIL: {
                        instruction_decoder.consume_as<SimpleInstructionWithArgs<2>>();
                        break;
                    }
                    default:
                        if (*instruction_decoder.code_ptr == static_cast<char>(0xFF)) {
                            throw std::runtime_error("unreachable code");
                        }
                        throw std::runtime_error("invalid instruction type");
                }
                if (instruction.instruction_type != "") {
                    add_instruction(instruction);
                }
            }
        }
    }

public:

    const char* code_ptr;
    InstructionDecoder instruction_decoder;

    explicit Analyzer(const char* code_ptr) : code_ptr(code_ptr), instruction_decoder(InstructionDecoder(code_ptr)) {}

    void analyze() {
        collect_labels();
        collect_sequences();
    }

    void results() {
        for (const auto& [key, map_sequences_length] : sequences_counter) {

            std::vector<std::pair<std::vector<Instruction>, int>> items(map_sequences_length.begin(), map_sequences_length.end());

            // Сортируем по значению по убыванию
            std::sort(items.begin(), items.end(),
                [](const auto& a, const auto& b) {
                    return a.second > b.second; // > для убывания
                });

            std::cout << "Parametrized count for length " << key << std::endl;
            for (auto item : items) {

                for (int i = 0; i < item.first.size(); i++) {
                    auto instruction = item.first[i];
                    std::cout << instruction.instruction_type;
                    if (instruction.instruction_type == "LD" || instruction.instruction_type == "LDA" || instruction.instruction_type == "ST") {
                        std::cout << " " << magic_enum::enum_name(static_cast<MemVar>(instruction.args[0]))
                                  << " " << instruction.args[1];
                    } else if (instruction.instruction_type == "BINOP") {
                        std::cout << " " << magic_enum::enum_name(static_cast<BinOp>(instruction.args[0]));
                    } else {
                        for (auto arg : instruction.args) {
                            std::cout << " " << arg;
                        }
                    }
                    if (i != item.first.size() - 1) {
                        std::cout << "; ";
                    }
                }
                std::cout << ": " << item.second << std::endl;
            }
            std::cout << std::endl;
        }
    }
};
