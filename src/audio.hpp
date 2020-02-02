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
#include <thread>
#include <SDL.h>

#include "chip8.hpp"
#include "utils.hpp"

namespace c8::audio {

using namespace std::chrono_literals;

namespace impl {

inline int sample_nr;
inline SDL_AudioDeviceID dev;
inline SDL_AudioSpec     spec;
inline std::thread      audio_thread;
inline std::atomic_bool audio_thread_should_stop = false;

} // namespace impl

int initialize(std::uint8_t &sound_timer) {
    if (auto rc = SDL_InitSubSystem(SDL_INIT_AUDIO); rc != 0) {
        ERROR("Failed to init audio: %#x - %s\n", rc, SDL_GetError());
        return rc;
    }

    SDL_AudioSpec want;
    want.freq     = 44100;
    want.format   = AUDIO_S16SYS;
    want.channels = 1;
    want.samples  = 2048;
    want.userdata = nullptr;
    want.callback = +[](void *userdata, std::uint8_t *data, int length) {
        UNUSED(userdata);
        for (int i = 0; i < length / 2; ++i, ++impl::sample_nr) {
            float time = static_cast<float>(impl::sample_nr) / 44100.0f;
            reinterpret_cast<std::int16_t *>(data)[i] = 28000.0f * std::sin(2.0f * M_PI * 441.0f * time);
        }
    };

    if (SDL_OpenAudio(&want, &impl::spec) != 0) {
        ERROR("Failed to open audio device: %s\n", SDL_GetError());
        return 1;
    }

    impl::audio_thread = std::thread([&sound_timer] {
        while (!impl::audio_thread_should_stop) {
            if (sound_timer) {
                SDL_PauseAudio(0);
                std::this_thread::sleep_for(sound_timer * timer_rate);
                SDL_PauseAudio(1);
            }
            std::this_thread::sleep_for(1ms);
        }
    });

    return 0;
}

void finalize() {
    impl::audio_thread_should_stop = true;
    impl::audio_thread.join();

    SDL_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

} // namespace c8::audio
