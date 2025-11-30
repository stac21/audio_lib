#pragma once

#include <functional>
#include <array>
#include <span>
#include <tuple>
#include <type_traits>

#include "dsp_declarations.hpp"

namespace dsp {
template<typename _sample_t>
struct Frame {
	using sample_type = _sample_t;

	_sample_t left_sample  = SAMPLE_SILENCE;
	_sample_t right_sample = SAMPLE_SILENCE;

	constexpr Frame() noexcept = default;

	constexpr Frame(const _sample_t left_sample, const _sample_t right_sample) noexcept :
		left_sample(left_sample),
		right_sample(right_sample)
	{ }
};

// TODO make this into a class and initialize the class to SAMPLE_SILENCE by default
// check ArrTest in main.cpp for guidance on how to do this
template<typename _sample_t, size_t _capacity>
class Wave : public std::array<Frame<_sample_t>, _capacity> {
public:
	using sample_type = _sample_t;

private:
	// inherit constructors from parent
	using __Parent = std::array<Frame<_sample_t>, _capacity>;
	using __Parent::__Parent;
	/**
	 * @brief  Populate the wave with the provided samples
	 * @param  samples Span of samples to write to the wave
	 * @param  start_index Start index of the samples to write to the wave
	 * @
	 * @return Number of samples written to the wave
	 */
	//int32_t populate(const std::span<_sample_t>::iterator samples, const size_t start_index, const size_t len) {
	//	size_t num_samples_written = 0;

	//	for (auto it = begin; it != end && num_samples_written < _capacity; it++) {
	//		this->at(num_samples_written++) = *it;
	//	}

	//	return num_samples_written;
	//}
};

template<typename _sample_t>
struct Signal {
	using sample_type = _sample_t;

	/// Default sample rate in KHz
	static constexpr sample_rate_t DEFAULT_SAMPLE_RATE = 44100;
	/// Sample rate in KHz
	sample_rate_t sample_rate = DEFAULT_SAMPLE_RATE;
	std::vector<Frame<_sample_t>> frames;

	Signal() = default;

	Signal(const uint32_t sample_rate) :
		sample_rate(sample_rate)
	{ }

	Signal(const std::vector<Frame<_sample_t>>& frames) :
		frames(frames)
	{ }

	Signal(const uint32_t sample_rate, const std::vector<Frame<_sample_t>>& frames) :
		sample_rate(sample_rate),
		frames(frames)
	{ }

	///**
	// * @brief Populate a wave starting at the given sample index
	// * @param wave The wave to populate
	// * @param time Time in ms to begin populating the wave from
	// * @return Range of the sample indices
	// */
	//template<size_t _capacity>
	//int32_t populate_wave(Wave<_sample_t, _capacity>& wave, size_t sample_index) const {
	//	//const size_t sample_index = time * (this->m_sample_rate / 1000.0);
	//	// TODO make this return a range where the start is the sample_index and the end is where the wave ends

	//	if (sample_index >= this->frames.size()) {
	//		return -1;
	//	}

	//	for (size_t i = 0; i < wave.size() && (sample_index + i) < this->frames.size(); i++) {
	//		wave.at(i) = this->frames.at(sample_index + i);
	//	}

	//	return sample_index;
	//}
};
} // namespace dsp

/// Tuple size specialization for the Wave
template<typename _sample_t, size_t _capacity>
struct std::tuple_size<dsp::Wave<_sample_t, _capacity>> : public std::integral_constant<std::size_t, _capacity>
{};
