//
// Created by aleksandr on 15.10.25.
//

#pragma once
#include <cstdint>

constexpr int64_t box_int(int64_t value) {
    return (value << 1) | 1;
}

constexpr int64_t box_ptr(int64_t value) {
    return (value << 1);
}

constexpr int64_t unbox(int64_t value) {
    return value >> 1;
}

constexpr bool is_boxed_int(int64_t value) {
    return value & 1;
}