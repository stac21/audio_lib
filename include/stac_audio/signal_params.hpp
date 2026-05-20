#pragma once

#include "dsp_declarations.hpp"

namespace stac {
struct SignalParams {
	dsp::frequency_hz_t fundamental_frequency = 0.0;
	dsp::sample_rate_t sample_rate = 0;
	size_t length = 0;
};
}
