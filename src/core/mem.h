#pragma once

// Interface between the game and program. Provides memory read/write functions.

#include <cstdint>
#include <exception>
#include <string>
#include <vector>
#include <iostream>

namespace mem {
    template <typename T>
    T read(uintptr_t address) {
        if (address == 0 || address > 0x7ff000000000) throw std::exception();
        return *(T *)address;
    }

    template <typename T>
    void write(uintptr_t address, T value) {
        *(T *)address = value;
    }
}