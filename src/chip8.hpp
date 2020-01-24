// Copyright (C) 2020 averne
//
// This file is part of c8.
//
// c8 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// c8 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with c8.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <cstdint>
#include <atomic>
#include <memory>
#include <thread>
#include <chrono>

#include "instruction.hpp"
#include "rom.hpp"
#include "window.hpp"

namespace c8 {

using namespace std::chrono_literals;

using Address = std::uint16_t;

enum AddressSpace: Address {
    ReservedStart     = 0,
    ReservedEnd       = 0x1ff,
    ProgramStart      = 0x200,
    ProgramStartAlt   = 0x600,
    ProgramEnd        = 0xfff,
    AddressSpaceStart = 0,
    AddressSpaceEnd   = 0x1000,
};

using Stack = std::array<Address, 0x10>;
using Ram   = std::array<std::uint8_t, AddressSpaceEnd>;

static inline auto timer_rate = 16.67ms;

struct Registers {
    // General-purpose registers
    std::uint8_t V0, V1, V2, V3, V4, V5, V6, V7, V8, V9, Va, Vb, Vc, Vd, Ve;
    std::uint8_t Vf; // Flag register

    // 16-bit register
    std::uint16_t I;

    // Special registers
    std::uint16_t PC; // Program counter
    std::uint16_t SP; // Stack pointer
    std::uint8_t  DT; // Delay timer
    std::uint8_t  ST; // Sound timer

    inline std::uint8_t &operator [](std::size_t index) {
        return reinterpret_cast<std::uint8_t *>(this)[index];
    }
};

class Chip8 {
    public:
        Chip8(const std::shared_ptr<rom::Program> &program);
        ~Chip8();

        static ins::Instruction *decode(ins::Opcode op) noexcept;

        void cycle();

    protected:
        static inline std::unique_ptr<ins::Instruction> cur_ins;

    public:
        std::thread      timer_thread;
        std::atomic_bool timer_thread_should_stop = false;

        Registers   regs{};
        Ram         ram{};
        Stack       stack{};
        win::Window window{};
};

} // namespace c8
