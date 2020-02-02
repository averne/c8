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
#include <array>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <curses.h>

namespace c8::win {

constexpr static std::uint8_t width         = 64;
constexpr static std::uint8_t height        = 32;
constexpr static std::uint8_t window_width  = width * 2 + 2;
constexpr static std::uint8_t window_height = height + 2;

constexpr static std::uint8_t pause_win_height = 5;
constexpr static std::uint8_t pause_win_witdh  = 14;
constexpr static std::uint8_t pause_win_x      = (window_width  + 2 - pause_win_witdh)  / 2;
constexpr static std::uint8_t pause_win_y      = (window_height + 2 - pause_win_height) / 2;

using Buffer = std::array<std::uint8_t, width * height>;
using Sprite = std::vector<std::uint8_t>;

enum Key: int {
    Key1       = 1,
    Key2       = 2,
    Key3       = 3,
    KeyC       = 0xc,
    Key4       = 4,
    Key5       = 5,
    Key6       = 6,
    KeyD       = 0xd,
    Key7       = 7,
    Key8       = 8,
    Key9       = 9,
    KeyE       = 0xe,
    KeyA       = 0xa,
    Key0       = 0,
    KeyB       = 0xb,
    KeyF       = 0xf,
    KeyInvalid = 0x10,
};

class Window {
    public:
        Window();
        ~Window();

        void update();

        void draw_pause();

        void clear();
        bool apply_sprite(const Sprite &sprite, std::uint8_t x, std::uint8_t y);

        static constexpr inline Key chr_to_key(int chr) {
            switch(chr) {
                case '"':  return Key1;
                case '\'': return Key2;
                case '(':  return Key3;
                case '-':  return KeyC;
                case 'e':  return Key4;
                case 'r':  return Key5;
                case 't':  return Key6;
                case 'y':  return KeyD;
                case 'd':  return Key7;
                case 'f':  return Key8;
                case 'g':  return Key9;
                case 'h':  return KeyE;
                case 'c':  return KeyA;
                case 'v':  return Key0;
                case 'b':  return KeyB;
                case 'n':  return KeyF;
                default:   return KeyInvalid;
            }
        }

        static constexpr inline bool is_key_in_range(Key key) {
            return key < KeyInvalid;
        }

        Key wait_for_key();
        bool is_key_down(Key key);
        bool is_key_up(Key key);

    public:
        Buffer buf{};

        bool should_pause = false;

    private:
        std::array<std::uint16_t, KeyInvalid> keys{};

        WINDOW *win, *pause_win;
};

} // namespace c8::win
