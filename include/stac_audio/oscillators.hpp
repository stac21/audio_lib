#pragma once

#include <optional>
#include <numbers>
#include <cmath>

#include "dsp_declarations.hpp"
#include "signals.hpp"

namespace stac {
// it really doesn't make any sense to calculate more than one cycle for any of these so the
// num_signals parameter is not actually necessary


template<typename _sample_t>
std::optional<dsp::Signal<_sample_t>> generate_sin_signal(const dsp::frequency_t frequency, const dsp::sample_rate_t sample_rate, size_t num_samples) {
	dsp::Signal<_sample_t> signal(sample_rate);

	try {
		signal.frames.reserve(num_samples);
	} catch (const std::bad_alloc& e) {
		return std::nullopt;
	}

	_sample_t curr_sample = 0.0;

	for (size_t sample_index = 0; sample_index < num_samples; sample_index++) {
		curr_sample = static_cast<_sample_t>(std::sin(frequency * 2 * std::numbers::pi * (static_cast<double>(sample_index) / sample_rate)));

		signal.frames.emplace_back(curr_sample, curr_sample);
	}

	return signal;
}

// I don't think that this is actually the right way to do it,
// but check the google thing for descriptions of ways to do sawtooth
// properly (algorithms). There is an example in the audio_sandbox directory
enum class SawtoothGenerationStrategy {
	ADDITIVE = 0,
	PHASE_ACCUMULATOR = 1,
}

template<typename _sample_t>
std::optional<dsp::Signal<_sample_t>> generate_sawtooth_signal(const dsp::frequency_t fundamental_frequency, const uint16_t num_harmonics, const dsp::sample_rate_t sample_rate, const size_t num_samples) {
	dsp::Signal<_sample_t> signal(sample_rate);

	try {
		signal.frames.reserve(num_samples);
	} catch (const std::bad_alloc& e) {
		return std::nullopt;
	}

	_sample_t curr_sample = 0.0;

	for (size_t sample_index = 0; sample_index < num_samples; sample_index++) {
		for (size_t harmonic_num = 1; harmonic_num <= num_harmonics; harmonic_num++) {
			curr_sample = (1 / static_cast<float>(harmonic_num)) * static_cast<_sample_t>(std::)

			signal.frames.emplace_back(curr_sample);
		}
	}

	return signal;
}
}
