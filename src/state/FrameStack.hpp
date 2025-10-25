//
// Created by aleksandr on 12.10.25.
//

#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fmt/core.h>

#include "../runtime/runtime_common.h"

extern void *__gc_stack_top;
extern void *__gc_stack_bottom;

void *__start_custom_data;
void *__stop_custom_data;

static_assert(sizeof(size_t) == sizeof(uint64_t*));
static_assert(sizeof(uint64_t) == sizeof(uint64_t*));
static_assert(sizeof(uint64_t) == sizeof(uint64_t**));

template<size_t max_stack>
class FrameStack {
    uint64_t** stack_data_links;
    uint64_t* stack_data;

    uint64_t **fp;
    uint64_t **sp;

    int global_size;
    std::vector<int> local_sizes, arg_sizes, ops_size;

    void check_ops(const uint32_t n) const {
        if (ops_size.empty()) {
            throw std::runtime_error("FrameStack: access an op, when number of stack frames is 0");
        }
        if (ops_size.back() < n) {
            throw std::runtime_error(
                fmt::format(
                    "FrameStack: access to an op, when ops stack is empty, or try to pop more ops than exist on op stack {} {}", n, ops_size.back()
                    )
                );
        }
    }

    void check_arg(const uint32_t index) const {
        if (arg_sizes.empty()) {
            throw std::runtime_error("FrameStack: access an argument, when number of stack frames is 0");
        }
        if (arg_sizes.back() <= index || index < 0) {
            throw std::runtime_error(
                fmt::format(
                    "FrameStack: access to an argument {}, that should lie within 0 <= arg < {}", index, arg_sizes.back()
                    )
                );
        }
    }

    void check_global(const uint32_t index) const {
        if (global_size <= index || index < 0) {
            throw std::runtime_error(
                fmt::format(
                    "FrameStack: access to a global {}, that should lie within 0 <= global < {}", index, global_size
                    )
                );
        }
    }

    void check_local(const uint32_t index) const {

        if (local_sizes.empty()) {
            throw std::runtime_error("FrameStack: access a local, when locals are not reserved (that happens in BEGIN)");
        }
        if (local_sizes.back() <= index || index < 0) {
            throw std::runtime_error(
                fmt::format(
                    "FrameStack: access to a local %d, that should lie within 0 <= local < %d", index, local_sizes.back()
                    )
                );
        }
    }

public:
    FrameStack(int global_area_size) {
        global_size = global_area_size;
        stack_data_links = new uint64_t*[max_stack];
        stack_data = new uint64_t[max_stack];

        stack_data_links++;
        for (uint32_t i = 0; i < global_area_size; i++) {
            stack_data_links[i] = stack_data + i;
        }

        fp = nullptr;
        sp = stack_data_links + global_area_size;

        __gc_stack_top = stack_data_links - 1;
        __gc_stack_bottom = (stack_data_links - 1) + global_area_size;

        __start_custom_data = __gc_stack_top;
        __stop_custom_data = __start_custom_data;
    }

    ~FrameStack() {
        delete[] stack_data_links;
        delete[] stack_data;
    }

    [[nodiscard]] uint64_t get_global(const uint32_t index) const {
        check_global(index);
        return stack_data[index];
    }

    void set_global(const uint32_t index, const uint64_t val) const {
        check_global(index);
        stack_data[index] = val;
    }

    void push_stack_frame(const uint32_t nargs, const char* ra) {
        ops_size.push_back(0);
        arg_sizes.push_back(nargs);
        if (nargs > 1) std::reverse(sp - nargs, sp);

        auto cur_write_sp = (sp - stack_data_links) + stack_data;
        *sp = cur_write_sp;
        *(sp + 1) = cur_write_sp + 1;
        *(sp + 2) = cur_write_sp + 2;

        *cur_write_sp = reinterpret_cast<uint64_t>(ra);
        *(cur_write_sp + 1) = reinterpret_cast<uint64_t>(fp);
        *(cur_write_sp + 2) = reinterpret_cast<uint64_t>(sp - nargs);

        __gc_stack_bottom = (sp += 3) - 1;
        fp = sp;
    }

    const char* pop_stack_frame() {
        if (ops_size.empty() || ops_size.empty() || local_sizes.empty()) {
            throw std::runtime_error("Exiting wrongly initialized block with the END instruction");
        }
        ops_size.pop_back();
        arg_sizes.pop_back();
        local_sizes.pop_back();

        uint64_t** prev_fp = fp;
        fp = reinterpret_cast<uint64_t**>(**(prev_fp - 2));
        __gc_stack_bottom = (sp = reinterpret_cast<uint64_t**>(**(prev_fp - 1))) - 1;
        auto ret = reinterpret_cast<const char*>(**(prev_fp - 3));
        return ret;
    }

    void reserve_locals(const uint32_t n_locals) {
        local_sizes.push_back(n_locals);
        auto cur_write_sp = (sp - stack_data_links) + stack_data;
        for (uint32_t i = 0; i < n_locals; i++) {
            *(sp + i) = cur_write_sp + i;
        }
        __gc_stack_bottom = (sp += n_locals) - 1;
    }

    [[nodiscard]] uint64_t get_arg(const uint32_t index) const {
        check_arg(index);
        return **(fp - 4 - index);
    }

    void set_arg(const uint32_t index, const uint64_t value) const {
        check_arg(index);
        **(fp - 4 - index) = value;
    }

    [[nodiscard]] uint64_t get_local(const uint32_t index) const {
        check_local(index);
        return **(fp + index);
    }

    void set_local(const uint32_t index, const uint64_t value) const {
        check_local(index);
        **(fp + index) = value;
    }

    void push_op(const uint64_t value) {
        check_ops(0);
        ops_size.back()++;
        auto cur_write_sp = (sp - stack_data_links) + stack_data;
        *sp = cur_write_sp;
        **sp = value;
        __gc_stack_bottom = (++sp) - 1;
    }

    [[nodiscard]] uint64_t peek_op() const {
        check_ops(1);
        return **(sp - 1);
    }

    [[nodiscard]] uint64_t peek_op(const uint32_t index) const {
        check_ops(index);
        return **(sp - index);
    }

    void pop_op() {
        check_ops(1);
        ops_size.back()--;
        __gc_stack_bottom = (--sp) - 1;
    }

    void pop_ops(const uint32_t n) {
        check_ops(n);
        ops_size.back() -= n;
        __gc_stack_bottom = (sp -= n) - 1;
    }

    uint64_t* get_ops_ptr(const u_int32_t n) {
        return *(sp - n);
    }
};