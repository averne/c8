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
#include <type_traits>

#include "utils.hpp"

namespace c8 {

class Chip8;

} // namespace c8

namespace c8::ins {

struct Opcode {
    std::uint16_t value = 0;

    constexpr inline Opcode() noexcept { }
    constexpr inline Opcode(const Opcode &op) noexcept: value(op.value) { }
    explicit constexpr inline Opcode(const std::uint16_t val) noexcept: value(val) { }

    constexpr inline operator const std::uint16_t &() const noexcept {
        return this->value;
    }

    constexpr inline operator std::uint16_t &() noexcept {
        return this->value;
    }

    constexpr inline std::uint16_t addr() const noexcept {
        return this->value & 0x0fff;
    }

    constexpr inline std::uint8_t x() const noexcept {
        return (this->value & 0x0f00) >> 8;
    }

    constexpr inline std::uint8_t y() const noexcept {
        return (this->value & 0x00f0) >> 4;
    }

    constexpr inline std::uint8_t byte() const noexcept {
        return this->value & 0x00ff;
    }

    constexpr inline std::uint8_t nibble() const noexcept {
        return this->value & 0x000f;
    }

#define DECL_BINARY_OPERATOR(op)                                                    \
    constexpr inline Opcode operator op(const Opcode &ins) const noexcept {         \
        return Opcode(this->value op ins.value);                                    \
    }

#define DECL_UNARY_OPERATOR(op)                                                     \
    constexpr inline Opcode &operator op(const Opcode &ins) noexcept {              \
        this->value op ins.value;                                                   \
        return *this;                                                               \
    }                                                                               \
                                                                                    \
    constexpr inline Opcode &operator op(const std::uint16_t &ins) noexcept {       \
        this->value op ins;                                                         \
        return *this;                                                               \
    }

#define DECL_COMP_OPERATOR(op)                                                      \
    constexpr inline bool operator op(const Opcode &ins) const noexcept {           \
        return this->value op ins.value;                                            \
    }

    DECL_BINARY_OPERATOR(+)
    DECL_BINARY_OPERATOR(-)
    DECL_BINARY_OPERATOR(*)
    DECL_BINARY_OPERATOR(/)
    DECL_BINARY_OPERATOR(&)
    DECL_BINARY_OPERATOR(|)
    DECL_BINARY_OPERATOR(^)
    DECL_BINARY_OPERATOR(>>)
    DECL_BINARY_OPERATOR(<<)

    DECL_UNARY_OPERATOR(=)
    DECL_UNARY_OPERATOR(+=)
    DECL_UNARY_OPERATOR(-=)
    DECL_UNARY_OPERATOR(/=)
    DECL_UNARY_OPERATOR(*=)
    DECL_UNARY_OPERATOR(&=)
    DECL_UNARY_OPERATOR(|=)
    DECL_UNARY_OPERATOR(^=)
    DECL_UNARY_OPERATOR(>>=)
    DECL_UNARY_OPERATOR(<<=)

    DECL_COMP_OPERATOR(==)
    DECL_COMP_OPERATOR(!=)
    DECL_COMP_OPERATOR(>)
    DECL_COMP_OPERATOR(<)
    DECL_COMP_OPERATOR(>=)
    DECL_COMP_OPERATOR(<=)

#undef DECL_BINARY_OPERATOR
#undef DECL_UNARY_OPERATOR
#undef DECL_COMP_OPERATOR
};
ASSERT_SIZE(Opcode, 2);

static constexpr inline std::uint16_t mask_1 = 0xf000;
static constexpr inline std::uint16_t mask_2 = 0xf00f;

struct Instruction {
    constexpr inline Instruction(Opcode op): op(op) { }
    virtual ~Instruction() = default;

    virtual void execute(Chip8 &chip) const;
    virtual void print() const;

protected:
    Opcode op;
};

struct Cls: public Instruction {
    constexpr inline Cls(Opcode op) noexcept: Instruction(op) { }
    virtual void execute(Chip8 &chip) const override;
    virtual void print() const override;

    static constexpr std::uint16_t mask      = 0xffff;
    static constexpr std::uint16_t compare   = 0x00e0;
    constexpr static inline bool match(Opcode op) noexcept {
        return (op & mask) == compare;
    }
};

struct Ret: public Instruction {
    constexpr inline Ret(Opcode op) noexcept: Instruction(op) { }
    virtual void execute(Chip8 &chip) const override;
    virtual void print() const override;

    constexpr static std::uint16_t mask      = 0xffff;
    constexpr static std::uint16_t compare   = 0x00ee;
    constexpr static inline bool match(Opcode op) noexcept {
        return (op & mask) == compare;
    }
};

struct Sys: public Instruction {
    constexpr inline Sys(Opcode op) noexcept: Instruction(op) { }
    virtual void execute(Chip8 &chip) const override;
    virtual void print() const override;

    constexpr static std::uint16_t compare   = 0x0000;
    constexpr static inline bool match(Opcode op) noexcept {
        return (op & mask_1) == compare;
    }
};

struct Jp: public Instruction {
    constexpr inline Jp(Opcode op) noexcept: Instruction(op) { }
    virtual void execute(Chip8 &chip) const override;
    virtual void print() const override;

    constexpr static std::uint16_t compare_1 = 0x1000;
    constexpr static std::uint16_t compare_2 = 0xb000;
    constexpr static inline bool match(Opcode op) noexcept {
        return (op & mask_1) == compare_1
            || (op & mask_1) == compare_2;
    }
};

struct Call: public Instruction {
    constexpr inline Call(Opcode op) noexcept: Instruction(op) { }
    virtual void execute(Chip8 &chip) const override;
    virtual void print() const override;

    constexpr static std::uint16_t compare   = 0x2000;
    constexpr static inline bool match(Opcode op) noexcept {
        return (op & mask_1) == compare;
    }
};

struct Se: public Instruction {
    constexpr inline Se(Opcode op) noexcept: Instruction(op) { }
    virtual void execute(Chip8 &chip) const override;
    virtual void print() const override;


    constexpr static std::uint16_t compare_1 = 0x3000;
    constexpr static std::uint16_t compare_2 = 0x5000;
    constexpr static inline bool match(Opcode op) noexcept {
        return (op & mask_1) == compare_1
            || (op & mask_2) == compare_2;
    }
};

struct Sne: public Instruction {
    constexpr inline Sne(Opcode op) noexcept: Instruction(op) { }
    virtual void execute(Chip8 &chip) const override;
    virtual void print() const override;

    constexpr static std::uint16_t compare_1 = 0x4000;
    constexpr static std::uint16_t compare_2 = 0x9000;
    constexpr static inline bool match(Opcode op) noexcept {
        return (op & mask_1) == compare_1
            || (op & mask_2) == compare_2;
    }
};

struct Ld: public Instruction {
    constexpr inline Ld(Opcode op) noexcept: Instruction(op) { }
    virtual void execute(Chip8 &chip) const override;
    virtual void print() const override;

    constexpr static std::uint16_t compare_1 = 0x6000;
    constexpr static std::uint16_t compare_2 = 0x8000;
    constexpr static inline bool match(Opcode op) noexcept {
        const auto x = op & 0xf0ff;
        return (op & mask_1) == compare_1
            || (op & mask_1) == 0xa000
            || (op & mask_2) == compare_2
            || x == 0xf007 || x == 0xf00a || x == 0xf015 || x == 0xf018
            || x == 0xf029 || x == 0xf033 || x == 0xf055 || x == 0xf065;

    }
};

struct Add: public Instruction {
    constexpr inline Add(Opcode op) noexcept: Instruction(op) { }
    virtual void execute(Chip8 &chip) const override;
    virtual void print() const override;

    constexpr static std::uint16_t compare_1 = 0x7000;
    constexpr static std::uint16_t compare_2 = 0x8004;
    constexpr static inline bool match(Opcode op) {
        return (op & mask_1) == compare_1
            || (op & mask_2) == compare_2
            || (op & 0xf0ff) == 0xf01e;
    }
};

struct Or: public Instruction {
    constexpr inline Or(Opcode op) noexcept: Instruction(op) { }
    virtual void execute(Chip8 &chip) const override;
    virtual void print() const override;

    constexpr static std::uint16_t compare   = 0x8001;
    constexpr static inline bool match(Opcode op) noexcept {
        return (op & mask_2) == compare;
    }
};

struct And: public Instruction {
    constexpr inline And(Opcode op) noexcept: Instruction(op) { }
    virtual void execute(Chip8 &chip) const override;
    virtual void print() const override;

    constexpr static std::uint16_t compare   = 0x8002;
    constexpr static inline bool match(Opcode op) noexcept {
        return (op & mask_2) == compare;
    }
};

struct Xor: public Instruction {
    constexpr inline Xor(Opcode op) noexcept: Instruction(op) { }
    virtual void execute(Chip8 &chip) const override;
    virtual void print() const override;

    constexpr static std::uint16_t compare   = 0x8003;
    constexpr static inline bool match(Opcode op) noexcept {
        return (op & mask_2) == compare;
    }
};

struct Sub: public Instruction {
    constexpr inline Sub(Opcode op) noexcept: Instruction(op) { }
    virtual void execute(Chip8 &chip) const override;
    virtual void print() const override;

    constexpr static std::uint16_t compare   = 0x8005;
    constexpr static inline bool match(Opcode op) noexcept {
        return (op & mask_2) == compare;
    }
};

struct Shr: public Instruction {
    constexpr inline Shr(Opcode op) noexcept: Instruction(op) { }
    virtual void execute(Chip8 &chip) const override;
    virtual void print() const override;

    constexpr static std::uint16_t compare   = 0x8006;
    constexpr static inline bool match(Opcode op) noexcept {
        return (op & mask_2) == compare;
    }
};

struct Subn: public Instruction {
    constexpr inline Subn(Opcode op) noexcept: Instruction(op) { }
    virtual void execute(Chip8 &chip) const override;
    virtual void print() const override;

    constexpr static std::uint16_t compare   = 0x8007;
    constexpr static inline bool match(Opcode op) noexcept {
        return (op & mask_2) == compare;
    }
};

struct Shl: public Instruction {
    constexpr inline Shl(Opcode op) noexcept: Instruction(op) { }
    virtual void execute(Chip8 &chip) const override;
    virtual void print() const override;

    constexpr static std::uint16_t compare   = 0x800e;
    constexpr static inline bool match(Opcode op) noexcept {
        return (op & mask_2) == compare;
    }
};

struct Rnd: public Instruction {
    constexpr inline Rnd(Opcode op) noexcept: Instruction(op) { }
    virtual void execute(Chip8 &chip) const override;
    virtual void print() const override;

    constexpr static std::uint16_t compare   = 0xc000;
    constexpr static inline bool match(Opcode op) noexcept {
        return (op & mask_1) == compare;
    }
};

struct Drw: public Instruction {
    constexpr inline Drw(Opcode op) noexcept: Instruction(op) { }
    virtual void execute(Chip8 &chip) const override;
    virtual void print() const override;

    constexpr static std::uint16_t compare   = 0xd000;
    constexpr static inline bool match(Opcode op) noexcept {
        return (op & mask_1) == compare;
    }
};

struct Skp: public Instruction {
    constexpr inline Skp(Opcode op) noexcept: Instruction(op) { }
    virtual void execute(Chip8 &chip) const override;
    virtual void print() const override;

    constexpr static std::uint16_t mask      = 0xf0ff;
    constexpr static std::uint16_t compare   = 0xe09e;
    constexpr static inline bool match(Opcode op) noexcept {
        return (op & mask) == compare;
    }
};

struct Sknp: public Instruction {
    constexpr inline Sknp(Opcode op) noexcept: Instruction(op) { }
    virtual void execute(Chip8 &chip) const override;
    virtual void print() const override;

    constexpr static std::uint16_t mask      = 0xf0ff;
    constexpr static std::uint16_t compare   = 0xe0a1;
    constexpr static inline bool match(Opcode op) noexcept {
        return (op & mask) == compare;
    }
};

} // namespace c8::ins
