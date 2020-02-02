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

#include <cstdio>
#include <string>
#include <vector>

#define _STRINGIFY(x)      #x
#define _CONCATENATE(x, y) x##y
#define  STRINGIFY(x)      _STRINGIFY(x)
#define  CONCATENATE(x, y) _CONCATENATE(x, y)

#define ASSERT_SIZE(t, sz) static_assert(sizeof(t) == sz, "Wrong size for " STRINGIFY(t))

#define UNUSED(x) ((void)(x))

#ifdef DEBUG
#   define LOG(...)     printf(__VA_ARGS__)
#else
#   define LOG(...)     do { } while(false)
#endif

#define TRACE(...)      LOG("[TRACE]: " __VA_ARGS__)
#define INFO(...)       LOG("[INFO]:  " __VA_ARGS__)
#define ERROR(...)      LOG("[ERROR]: " __VA_ARGS__)
#define FATAL(...)      LOG("[FATAL]: " __VA_ARGS__)

namespace c8::utils {

static inline FILE *open_file(const std::string &path, const std::string &mode = "r") {
    FILE *fp = fopen(path.c_str(), mode.c_str());
    if (!fp)
        ERROR("Failed to open %s\n", path.c_str());
    else
        TRACE("Loaded %s\n", path.c_str());
    return fp;
}

template <typename T>
static inline std::vector<T> &read_file(std::vector<T> &container, const std::string &path) {
    container.clear();

    FILE *fp = open_file(path);
    if (!fp)
        return container;

    fseek(fp, 0, SEEK_END);
    std::size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    container.resize(size / sizeof(T) + 1);
    if (fread(container.data(), 1, size, fp) != size)
        ERROR("Failed to read %s\n", path.c_str());
    container[size] = static_cast<T>(0); // Ensure null termination

    return container;
}

} // namespace c8::utils
