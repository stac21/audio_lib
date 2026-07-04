#pragma once

#include <cstdint>
#include <complex>

#include "frame.hpp"

namespace dsp {
using sample_t        = float;
using phase_t         = int32_t;
using frequency_hz_t  = double;
using amplitude_t     = float;
using pitch_t         = double;
using time_t          = double;
using time_ms_t       = uint64_t;
using sample_rate_t   = uint32_t;
using bandwidth_t     = double;
using gain_db_t       = double;
using frame_real_t    = Frame<sample_t>;
using frame_complex_t = Frame<std::complex<sample_t>>;

constexpr sample_t      SAMPLE_SILENCE      = 0.0f;
constexpr sample_rate_t DEFAULT_SAMPLE_RATE = 44100;
constexpr uint32_t      FRAMES_PER_BUFFER   = 256;
constexpr uint8_t       NUM_CHANNELS        = 2;
}
