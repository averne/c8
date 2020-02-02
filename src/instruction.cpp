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

#include <algorithm>
#include <climits>
#include <limits>
#include <experimental/random>

#include "chip8.hpp"
#include "instruction.hpp"
#include "window.hpp"
#include "utils.hpp"

#define COMP_MASK(mask, compare) ((op & mask) == compare)
#define COMP(n) COMP_MASK(CONCATENATE(mask_, n), CONCATENATE(compare_, n))

namespace c8::ins {

using reg_lim = std::numeric_limits<std::uint8_t>;

// Instruction implementations
void Instruction::execute(Chip8 &c) const {
    UNUSED(c);
    ERROR("Unknown instruction\n");
}

void Cls::execute(Chip8 &c) const {
    c.window.clear();
}

void Ret::execute(Chip8 &c) const {
    c.regs.PC = c.stack[--c.regs.SP];
}

void Sys::execute(Chip8 &c) const {
    UNUSED(c);
    // Instruction deprecated
    // c.regs.PC = op.addr() - 2;
}

void Jp::execute(Chip8 &c) const {
    if (COMP(1))
        c.regs.PC = op.addr();
    else if (COMP_MASK(mask_1, compare_2))
        c.regs.PC = c.regs.V0 + op.addr();
    c.regs.PC -= 2;
}

void Call::execute(Chip8 &c) const {
    c.stack[c.regs.SP++] = c.regs.PC;
    c.regs.PC = op.addr() - 2;
}

void Se::execute(Chip8 &c) const {
    if (COMP(1) && (c.regs[op.x()] == op.byte()))
        c.regs.PC += 2;
    else if (COMP(2) && (c.regs[op.x()] == c.regs[op.y()]))
        c.regs.PC += 2;
}

void Sne::execute(Chip8 &c) const {
    if (COMP(1) && (c.regs[op.x()] != op.byte()))
        c.regs.PC += 2;
    else if (COMP(2) && (c.regs[op.x()] != c.regs[op.y()]))
        c.regs.PC += 2;
}

void Ld::execute(Chip8 &c) const {
    if (COMP(1))
        c.regs[op.x()] = op.byte();
    else if (COMP_MASK(mask_1, 0xa000))
        c.regs.I = op.addr();
    else if (COMP(2))
        c.regs[op.x()] = c.regs[op.y()];
    else if (COMP_MASK(0x00ff, 0x0007))
        c.regs[op.x()] = c.regs.DT;
    else if (COMP_MASK(0x00ff, 0x000a))
        c.regs[op.x()] = c.window.wait_for_key();
    else if (COMP_MASK(0x00ff, 0x0015))
        c.regs.DT = c.regs[op.x()];
    else if (COMP_MASK(0x00ff, 0x0018))
        c.regs.ST = c.regs[op.x()];
    else if (COMP_MASK(0x00ff, 0x0029))
        c.regs.I = c.regs[op.x()] * 5 * sizeof(std::uint8_t);
    else if (COMP_MASK(0x00ff, 0x0033)) {
        c.ram[c.regs.I + 0] =  c.regs[op.x()] / 100;
        c.ram[c.regs.I + 1] = (c.regs[op.x()] / 10) % 10;
        c.ram[c.regs.I + 2] =  c.regs[op.x()] % 10;
    } else if (COMP_MASK(0x00ff, 0x0055)) {
        for (std::uint8_t i = 0; i <= op.x(); ++i)
            c.ram[c.regs.I + i] = c.regs[i];
    } else if (COMP_MASK(0x00ff, 0x0065)) {
        for (std::uint8_t i = 0; i <= op.x(); ++i)
            c.regs[i] = c.ram[c.regs.I + i];
    }
}

void Add::execute(Chip8 &c) const {
    if (COMP(1))
        c.regs[op.x()] += op.byte();
    else if (COMP(2)) {
        c.regs.Vf = (c.regs[op.x()] + c.regs[op.y()]) > reg_lim::max();
        c.regs[op.x()] = c.regs[op.x()] + c.regs[op.y()];
    } else if (COMP_MASK(0xf0ff, 0xf01e))
        c.regs.I += c.regs[op.x()];
}

void Or::execute(Chip8 &c) const {
    c.regs[op.x()] |= c.regs[op.y()];
}

void And::execute(Chip8 &c) const {
    c.regs[op.x()] &= c.regs[op.y()];
}

void Xor::execute(Chip8 &c) const {
    c.regs[op.x()] ^= c.regs[op.y()];
}

void Sub::execute(Chip8 &c) const {
    c.regs.Vf = c.regs[op.x()] > c.regs[op.y()];
    c.regs[op.x()] -= c.regs[op.y()];
}

void Shr::execute(Chip8 &c) const {
    c.regs.Vf = c.regs[op.x()] & (1 << 0);
    c.regs[op.x()] >>= 1;
}

void Subn::execute(Chip8 &c) const {
    c.regs.Vf = c.regs[op.y()] > c.regs[op.x()];
    c.regs[op.x()] = c.regs[op.y()] - c.regs[op.x()];
}

void Shl::execute(Chip8 &c) const {
    c.regs.Vf = !!(c.regs[op.x()] & (1 << (reg_lim::digits - 1)));
    c.regs[op.x()] <<= 1;
}

void Rnd::execute(Chip8 &c) const {
    c.regs[op.x()] = std::experimental::randint(static_cast<int>(reg_lim::min()), static_cast<int>(reg_lim::max())) & op.byte();
}

void Drw::execute(Chip8 &c) const {
    // Construct sprite
    win::Sprite sprite;
    sprite.resize(op.nibble());
    std::copy_n(&c.ram[c.regs.I], op.nibble(), sprite.data());

    // Apply sprite & update
    c.regs.Vf = c.window.apply_sprite(sprite, c.regs[op.x()], c.regs[op.y()]);
}

void Skp::execute(Chip8 &c) const {
    if (c.window.is_key_down(static_cast<win::Key>(c.regs[op.x()])))
        c.regs.PC += 2;
}

void Sknp::execute(Chip8 &c) const {
    if (c.window.is_key_up(static_cast<win::Key>(c.regs[op.x()])))
        c.regs.PC += 2;
}

// Instruction printing
void Instruction::print() const {
    printf("INS     Unknown instruction\n");
}

void Cls::print() const {
    printf("CLS\n");
}

void Ret::print() const {
    printf("RET\n");
}

void Sys::print() const {
    printf("SYS     %#x\n", op.addr());
}

void Jp::print() const {
    printf("JP      %#x\n", op.addr());
}

void Call::print() const {
    printf("CALL    %#x\n", op.addr());
}

void Se::print() const {
    if (COMP(1))
        printf("SE      V%x %#x\n", op.x(), op.byte());
    else if (COMP(2))
        printf("SE      V%x V%x\n", op.x(), op.y());
}

void Sne::print() const {
    if (COMP(1))
        printf("SNE     V%x %#x\n", op.x(), op.byte());
    else if (COMP(2))
        printf("SNE     V%x V%x\n", op.x(), op.y());
}

void Ld::print() const {
    if (COMP(1))
        printf("LD      V%x %#x\n", op.x(), op.byte());
    else if (COMP_MASK(mask_1, 0xa000))
        printf("LD      I %#x\n", op.addr());
    else if (COMP(2))
        printf("LD      V%x V%x\n", op.x(), op.y());
    else if (COMP_MASK(0x00ff, 0x0007))
        printf("LD      V%x DT\n", op.x());
    else if (COMP_MASK(0x00ff, 0x000a))
        printf("LD      V%x K\n", op.x());
    else if (COMP_MASK(0x00ff, 0x0015))
        printf("LD      DT V%x\n", op.x());
    else if (COMP_MASK(0x00ff, 0x0018))
        printf("LD      ST V%x\n", op.x());
    else if (COMP_MASK(0x00ff, 0x0029))
        printf("LD      F V%x\n", op.x());
    else if (COMP_MASK(0x00ff, 0x0033))
        printf("LD      B V%x\n", op.x());
    else if (COMP_MASK(0x00ff, 0x0055))
        printf("LD      [I] V%x\n", op.x());
    else if (COMP_MASK(0x00ff, 0x0065))
        printf("LD      V%x [I]\n", op.x());
}

void Add::print() const {
    if (COMP(1))
        printf("ADD     V%x %#x\n", op.x(), op.byte());
    else if (COMP(2))
        printf("ADD     V%x V%x\n", op.x(), op.y());
    else if (COMP_MASK(0xf0ff, 0xf01e))
        printf("ADD     I %#x\n", op.x());
}

void Or::print() const {
    printf("OR      V%x V%x\n", op.x(), op.y());
}

void And::print() const {
    printf("AND     V%x V%x\n", op.x(), op.y());
}

void Xor::print() const {
    printf("XOR     V%x V%x\n", op.x(), op.y());
}

void Sub::print() const {
    printf("SUB     V%x V%x\n", op.x(), op.y());
}

void Shr::print() const {
    printf("SHR     V%x V%x\n", op.x(), op.y());
}

void Subn::print() const {
    printf("SUBN    V%x V%x\n", op.x(), op.y());
}

void Shl::print() const {
    printf("SHL     V%x V%x\n", op.x(), op.y());
}

void Rnd::print() const {
    printf("RND     V%x V%x\n", op.x(), op.byte());
}

void Drw::print() const {
    printf("DRW     V%x V%x %#x\n", op.x(), op.y(), op.nibble());
}

void Skp::print() const {
    printf("SKP     V%x\n", op.x());
}

void Sknp::print() const {
    printf("SKNP    V%x\n", op.x());
}

} // namespace c8::ins
