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

#include <array>
#include <algorithm>
#include <experimental/random>

#include "audio.hpp"
#include "instruction.hpp"
#include "window.hpp"

#include "chip8.hpp"

using namespace std::chrono_literals;

namespace c8 {

namespace {

constexpr inline std::array glyphs = {
    std::array<uint8_t, 5>{0xf0, 0x90, 0x90, 0x90, 0xf0}, // 0
    std::array<uint8_t, 5>{0x20, 0x60, 0x20, 0x20, 0x70}, // 1
    std::array<uint8_t, 5>{0xF0, 0x10, 0xF0, 0x80, 0xF0}, // 2
    std::array<uint8_t, 5>{0xF0, 0x10, 0xF0, 0x10, 0xF0}, // 3
    std::array<uint8_t, 5>{0x90, 0x90, 0xF0, 0x10, 0x10}, // 4
    std::array<uint8_t, 5>{0xF0, 0x80, 0xF0, 0x10, 0xF0}, // 5
    std::array<uint8_t, 5>{0xF0, 0x80, 0xF0, 0x90, 0xF0}, // 6
    std::array<uint8_t, 5>{0xF0, 0x10, 0x20, 0x40, 0x40}, // 7
    std::array<uint8_t, 5>{0xF0, 0x90, 0xF0, 0x90, 0xF0}, // 8
    std::array<uint8_t, 5>{0xF0, 0x90, 0xF0, 0x10, 0xF0}, // 9
    std::array<uint8_t, 5>{0xF0, 0x90, 0xF0, 0x90, 0x90}, // A
    std::array<uint8_t, 5>{0xE0, 0x90, 0xE0, 0x90, 0xE0}, // B
    std::array<uint8_t, 5>{0xF0, 0x80, 0x80, 0x80, 0xF0}, // C
    std::array<uint8_t, 5>{0xE0, 0x90, 0x90, 0x90, 0xE0}, // D
    std::array<uint8_t, 5>{0xF0, 0x80, 0xF0, 0x80, 0xF0}, // E
    std::array<uint8_t, 5>{0xF0, 0x80, 0xF0, 0x80, 0x80}, // F
};

} // namespace

Chip8::Chip8(const std::shared_ptr<rom::Program> &program) {
    constexpr auto available = AddressSpaceEnd - ProgramStart;
    if (program->size() > available)
        ERROR("Program too large to fit in memory\n");

    // Seed random number generator
    std::experimental::reseed();

    // Set up glyph data
    std::copy(glyphs.begin(), glyphs.end(),
        reinterpret_cast<decltype(glyphs)::value_type *>(this->ram.begin()));

    // Set up program in address space
    this->regs.PC = ProgramStart;
    std::copy(program->begin(), program->end(),
        reinterpret_cast<ins::Opcode *>(this->ram.begin() + ProgramStart));

    this->timer_thread = std::thread([this]() {
        while (!this->timer_thread_should_stop) {
            std::this_thread::sleep_for(timer_rate);
            if (!this->window.should_pause && this->regs.DT)
                --this->regs.DT;
            if (!this->window.should_pause && this->regs.ST)
                --this->regs.ST;
        }
    });

    audio::initialize(this->regs.ST);
}

Chip8::~Chip8() {
    this->timer_thread_should_stop = true;
    this->timer_thread.join();
    audio::finalize();
}

void Chip8::cycle() {
    this->window.update();
    if (this->window.should_pause) {
        this->window.draw_pause();
        return;
    }

    std::uint16_t op = *reinterpret_cast<std::uint16_t *>(&this->ram[this->regs.PC]);
    auto ins = decode(ins::Opcode(__builtin_bswap16(op)));
    ins->execute(*this);
    this->regs.PC += 2;
}

ins::Instruction *Chip8::decode(ins::Opcode op) noexcept {
    cur_ins.reset(new ins::Instruction(op));
    switch ((op & 0xf000) >> 12) {
        case 0:
            if (ins::Cls::match(op))
                cur_ins.reset(new ins::Cls(op));
            else if (ins::Ret::match(op))
                cur_ins.reset(new ins::Ret(op));
            else
                cur_ins.reset(new ins::Sys(op));
            break;

        case 1:
            cur_ins.reset(new ins::Jp(op));
            break;

        case 2:
            cur_ins.reset(new ins::Call(op));
            break;

        case 3:
            cur_ins.reset(new ins::Se(op));
            break;

        case 4:
            cur_ins.reset(new ins::Sne(op));
            break;

        case 5:
            if (ins::Se::match(op))
                cur_ins.reset(new ins::Se(op));
            break;

        case 6:
            cur_ins.reset(new ins::Ld(op));
            break;

        case 7:
            cur_ins.reset(new ins::Add(op));
            break;

        case 8:
            switch (op & 0x000f) {
                case 0:
                    cur_ins.reset(new ins::Ld(op));   break;
                case 1:
                    cur_ins.reset(new ins::Or(op));   break;
                case 2:
                    cur_ins.reset(new ins::And(op));  break;
                case 3:
                    cur_ins.reset(new ins::Xor(op));  break;
                case 4:
                    cur_ins.reset(new ins::Add(op));  break;
                case 5:
                    cur_ins.reset(new ins::Sub(op));  break;
                case 6:
                    cur_ins.reset(new ins::Shr(op));  break;
                case 7:
                    cur_ins.reset(new ins::Subn(op)); break;
                case 0xe:
                    cur_ins.reset(new ins::Shl(op));  break;
            }
            break;

        case 9:
            if (ins::Sne::match(op))
                cur_ins.reset(new ins::Sne(op));
            break;

        case 0xa:
            cur_ins.reset(new ins::Ld(op));
            break;

        case 0xb:
            cur_ins.reset(new ins::Jp(op));
            break;

        case 0xc:
            cur_ins.reset(new ins::Rnd(op));
            break;

        case 0xd:
            cur_ins.reset(new ins::Drw(op));
            break;

        case 0xe:
            if (ins::Skp::match(op))
                cur_ins.reset(new ins::Skp(op));
            else if (ins::Sknp::match(op))
                cur_ins.reset(new ins::Sknp(op));
            break;

        case 0xf:
            if (ins::Add::match(op))
                cur_ins.reset(new ins::Add(op));
            else if (ins::Ld::match(op))
                cur_ins.reset(new ins::Ld(op));
            break;
    }

    return cur_ins.get();
}

} // namespace c8
