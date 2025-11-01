//
// Created by aleksandr on 26.10.25.
//

#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

#pragma pack(push, 1)

class Designation {
public:

    MemVar mem_var;
    uint32_t value;

    static constexpr size_t length() {
        return 1 + sizeof(uint32_t);
    }
};

#pragma pack(pop)

typedef std::vector<Designation> Designations;
