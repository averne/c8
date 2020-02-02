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
#include <chrono>
#include <curses.h>
#include <signal.h>
#include <unistd.h>

#include "chip8.hpp"
#include "rom.hpp"
#include "utils.hpp"

using namespace std::chrono_literals;

static inline void print_usage([[maybe_unused]] char *progname) {
    FATAL("Usage: %s [-d] rom\n", progname);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    char *rom_path = nullptr;
    bool disassemble = false;

    INFO("Starting\n");

    int opt;
    while ((opt = getopt(argc, argv, "d")) != -1) {
        switch (opt) {
            case 'd':
                disassemble = true;
                break;
            default:
                print_usage(argv[0]);
        }
    }

    if (optind < argc) {
        rom_path = argv[optind];
    } else {
        print_usage(argv[0]);
    }

    auto rom = c8::rom::Rom(rom_path);
    if (rom.empty()) {
        FATAL("Failed to load rom %s\n", rom_path);
        return EXIT_FAILURE;
    }

    if (disassemble) {
        INFO("Disassembling:\n");
        std::uint16_t address = c8::ProgramStart;
        for (auto &op: *rom.get_code()) {
            printf("  %04x: %04x -> ", address, __builtin_bswap16(op));
            address += sizeof(c8::ins::Opcode);
            c8::Chip8::decode(c8::ins::Opcode(__builtin_bswap16(op)))->print();
        }
    }

    auto chip = c8::Chip8(rom.get_code());
    while (true) {
        chip.cycle();
        std::this_thread::sleep_for(2ms);
    }

    return EXIT_SUCCESS;
}
