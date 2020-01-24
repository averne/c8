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

#include <memory>
#include <string>
#include <vector>

#include "utils.hpp"

namespace c8::rom {

using Program = std::vector<std::uint16_t>;

class Rom {
    public:
        Rom(const std::string &path) {
            c8::utils::read_file(*this->rom, path);
        }

        std::shared_ptr<Program> get_code() noexcept {
            return this->rom;
        }

        inline bool empty() {
            return this->rom->empty();
        }

    protected:
        std::shared_ptr<Program> rom = std::make_shared<Program>();
};

} // namespace c8::rom
