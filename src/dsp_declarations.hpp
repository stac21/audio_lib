#pragma once

#include <stdint.h>

namespace dsp {
using sample_t = float;
using phase_t = int32_t;
using frequency_t = double;
using amplitude_t = double;
using time_t = double;
using sample_rate_t = uint32_t;

constexpr sample_t SAMPLE_SILENCE = 0.0f;
constexpr uint32_t SAMPLE_RATE = 44100;
constexpr uint32_t FRAMES_PER_BUFFER = 256;
constexpr uint8_t NUM_CHANNELS = 2;
}
