#pragma once

#include <iostream>
#include <numbers>
#include <cmath>

#include "dsp_declarations.hpp"
#include "signals.hpp"
#include "dsp_utils.hpp"

namespace stac {
template<typename _sample_t>
dsp::Signal<_sample_t> generate_sin_signal(const dsp::frequency_hz_t frequency, const dsp::sample_rate_t sample_rate) {
	dsp::Signal<_sample_t> signal;
	signal.sample_rate = sample_rate;

	const dsp::time_t period = 1.0 / frequency * 1000.0;
	const size_t num_samples = dsp::utils::sample_index_from_time(sample_rate, period);
	std::cout << "period: " << period << ", num_samples: " << num_samples << "\n";

	signal.samples.reserve(num_samples);

	using s = typename decltype(signal)::sample_type::sample_type;
	s curr_sample = {};

	for (size_t sample_index = 0; sample_index < num_samples; sample_index++) {
		curr_sample = static_cast<s>(std::sin(frequency * 2 * std::numbers::pi * (static_cast<double>(sample_index) / sample_rate)));

		signal.samples.emplace_back(curr_sample, curr_sample);
	}

	return signal;
}

// I don't think that this is actually the right way to do it,
// but check the google thing for descriptions of ways to do sawtooth
// properly (algorithms). There is an example in the audio_sandbox directory
enum class SawtoothGenerationStrategy {
	ADDITIVE = 0,
	PHASE_ACCUMULATOR = 1,
};

template<typename _sample_t>
dsp::Signal<_sample_t> generate_sawtooth_signal(const dsp::frequency_hz_t fundamental_frequency, const uint16_t num_harmonics, const dsp::sample_rate_t sample_rate) {
	dsp::Signal<_sample_t> signal(sample_rate);

	// TODO reserve proper num frames and calculate proper num samples
	signal.samples.reserve(1);
	size_t num_samples = 1;

	_sample_t curr_sample = 0.0;

	for (size_t sample_index = 0; sample_index < num_samples; sample_index++) {
		for (size_t harmonic_num = 1; harmonic_num <= num_harmonics; harmonic_num++) {
			// curr_sample = (1 / static_cast<float>(harmonic_num)) * static_cast<_sample_t>(std::)

			// signal.frames.emplace_back(curr_sample);
		}
	}

	return signal;
}
}
