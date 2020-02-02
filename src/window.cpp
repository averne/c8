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

#include <cstring>

#include "window.hpp"

using namespace std::chrono_literals;

namespace c8::win {

Window::Window(): win(initscr()),
        pause_win(newwin(pause_win_height, pause_win_witdh, pause_win_y, pause_win_x)) {
    wresize(this->win, window_height, window_width);
    cbreak();
    noecho();
    nodelay(stdscr, true);
    curs_set(0); // Hide cursor
}

Window::~Window() {
    nocbreak();
    echo();
    delwin(this->win);
    endwin();
}

void Window::update() {
    // Update keys
    int chr;
    while ((chr = getch()) != ERR) {
        if (auto key = chr_to_key(chr); is_key_in_range(key))
            ++keys[key];
        if (chr == ' ')
            this->should_pause ^= 1;
    }

    if (this->should_pause)
        return;

    // Update screen
    box(this->win, 0, 0);
    for (std::uint8_t y = 0; y < height; ++y) {
        for (std::uint8_t x = 0; x < width; ++x) {
            auto px = this->buf[y * width + x];
            if (px)
                attron(A_REVERSE);
            mvwaddch(this->win, y + 1, 2 * x + 1, ' ');
            mvwaddch(this->win, y + 1, 2 * x + 2, ' ');
            if (px)
                attroff(A_REVERSE);
        }
    }
    wrefresh(this->win);
}

void Window::draw_pause() {
    box(this->pause_win, 0, 0);
    attron(A_BOLD);
    mvwprintw(this->pause_win, 2, 4, "PAUSED");
    attroff(A_BOLD);
    wrefresh(this->pause_win);
}

void Window::clear() {
    std::memset(this->buf.begin(), 0, this->buf.size());
}

bool Window::apply_sprite(const Sprite &sprite, std::uint8_t x, std::uint8_t y) {
    bool collision = false;
    for (std::uint8_t sprite_y = 0; sprite_y < sprite.size(); ++sprite_y) {
        for (std::uint8_t sprite_x = 0; sprite_x < 7; ++sprite_x) {
            auto &old_px = this->buf[width * ((y + sprite_y) % height) + (x + sprite_x) % width];
            auto  new_px = !!(sprite[sprite_y] & (1 << (7 - sprite_x)));
            if (old_px && new_px)
                collision = true;
            old_px ^= new_px;
        }
    }
    return collision;
}

Key Window::wait_for_key() {
    while (true) {
        update();
        for (std::size_t i = 0; i < this->keys.size(); ++i) {
            if (this->keys[i]) {
                --this->keys[i];
                return static_cast<Key>(i);
            }
        }
        std::this_thread::sleep_for(2ms);
    }
}

bool Window::is_key_down(Key key) {
    auto ret = this->keys[key];
    if (this->keys[key])
        --this->keys[key]; // Consume key
    return ret;
}

bool Window::is_key_up(Key key) {
    auto ret = !this->keys[key];
    if (this->keys[key])
        --this->keys[key]; // Consume key
    return ret;
}

} // namespace c8::win
