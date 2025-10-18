//
// Created by aleksandr on 15.10.25.
//

#pragma once
#include <cstdint>

constexpr uint32_t box(uint32_t value) {
    return (value << 1) | 1;
}

constexpr uint32_t unbox(uint32_t value) {
    return value >> 1;
}

constexpr bool is_boxed(uint32_t value) {
    return value & 1;
}