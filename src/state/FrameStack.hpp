//
// Created by aleksandr on 12.10.25.
//

#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>

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

public:
    FrameStack(int global_area_size) {
        stack_data_links = new uint64_t*[max_stack];
        stack_data = new uint64_t[max_stack];

        stack_data_links++;
        for (uint32_t i = 0; i < global_area_size; i++) {
            stack_data_links[i] = stack_data + i;
        }

        fp = nullptr;
        sp = stack_data_links + global_area_size;

        // fp1 = nullptr;
        // sp1 = stack_data_links + global_area_size;

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
        return stack_data[index];
    }

    void set_global(const uint32_t index, const uint64_t val) const {
        stack_data[index] = val;
    }

    void push_stack_frame(const uint32_t nargs, const char* ra) {
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
        uint64_t** prev_fp = fp;
        fp = reinterpret_cast<uint64_t**>(**(prev_fp - 2));
        __gc_stack_bottom = (sp = reinterpret_cast<uint64_t**>(**(prev_fp - 1))) - 1;
        auto ret = reinterpret_cast<const char*>(**(prev_fp - 3));
        return ret;
    }

    [[nodiscard]] uint64_t get_arg(const uint32_t index) const {
        return **(fp - 4 - index);
    }

    void reserve_locals(const uint32_t n_locals) {
        auto cur_write_sp = (sp - stack_data_links) + stack_data;
        for (uint32_t i = 0; i < n_locals; i++) {
            *(sp + i) = (cur_write_sp + i);
        }
        __gc_stack_bottom = (sp += n_locals) - 1;
    }

    [[nodiscard]] uint64_t get_local(const uint32_t index) const {
        return **(fp + index);
    }

    void set_local(const uint32_t index, const uint64_t value) const {
        **(fp + index) = value;
    //     *(fp + index) = value;
    }

    void set_arg(const uint32_t index, const uint64_t value) const {
        **(fp - 4 - index) = value;
    }

    void push_op(const uint64_t value) {
        auto cur_write_sp = (sp - stack_data_links) + stack_data;
        *sp = cur_write_sp;
        **sp = value;
        __gc_stack_bottom = (++sp) - 1;
    }

    [[nodiscard]] uint64_t peek_op() const {
        return **(sp - 1);
    }

    [[nodiscard]] uint64_t peek_op(const uint32_t index) const {
        return **(sp - index);
    }

    void pop_op() {
        __gc_stack_bottom = (--sp) - 1;
    }

    void pop_ops(const uint32_t n) {
        __gc_stack_bottom = (sp -= n) - 1;
    }

    uint64_t* get_args_ptr(const u_int32_t n) {
        return *(sp - n);
    }
};