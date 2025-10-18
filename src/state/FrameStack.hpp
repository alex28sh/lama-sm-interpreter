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

template<size_t max_stack>
class FrameStack {
    uint32_t* stack_data = new uint32_t[max_stack];
    uint32_t *fp = nullptr;
    uint32_t *sp = stack_data;

public:
    FrameStack() {
        __gc_stack_top = stack_data;
        __gc_stack_bottom = stack_data;
    }

    ~FrameStack() {
        delete[] stack_data;
    }

    void push_stack_frame(const int32_t nargs, const char* ra) {
        if (nargs > 1) std::reverse(sp - nargs, sp);

        *sp = reinterpret_cast<uint64_t>(ra);
        *(sp + 1) = reinterpret_cast<uint64_t>(fp);
        *(sp + 2) = reinterpret_cast<uint64_t>(sp - nargs);
        __gc_stack_bottom = sp += 3;
        fp = sp;
    }

    const char* pop_stack_frame() {
        auto prev_fp = fp;
        fp = reinterpret_cast<uint32_t*>(*(prev_fp - 2));
        __gc_stack_bottom = sp = reinterpret_cast<uint32_t*>(*(prev_fp - 1));
        return reinterpret_cast<const char*>(*(prev_fp - 3));
    }

    [[nodiscard]] uint32_t get_arg(const int32_t index) const {
        return *(fp - 4 - index);
    }

    void reserve_locals(const int32_t n_locals) {
        __gc_stack_bottom = sp += n_locals;
    }

    [[nodiscard]] uint32_t get_local(const int32_t index) const {
        return *(fp + index);
    }

    void set_local(const int32_t index, const uint32_t value) {
        *(fp + index) = value;
    }

    void set_arg(const int32_t index, const uint32_t value) {
        *(fp - 4 - index) = value;
    }

    void push_op(const uint32_t value) {
        *sp = value;
        __gc_stack_bottom = ++sp;
    }

    [[nodiscard]] uint32_t peek_op() const {
        return *(sp - 1);
    }

    [[nodiscard]] uint32_t peek_op(const int32_t index) const {
        return *(sp - index);
    }

    void pop_op() {
        __gc_stack_bottom = --sp;
    }

    void pop_ops(const int32_t n) {
        __gc_stack_bottom = sp -= n;
    }
};