#pragma once

#include <vector>
#include <array>

#include "dsp_declarations.hpp"

namespace dsp {
enum class CapacityType : bool {
	dynamic = false,
	fixed   = true
};

template<typename _sample_t, size_t _capacity = static_cast<size_t>(CapacityType::dynamic)>
struct Signal {
	using sample_type = _sample_t;

	static constexpr CapacityType capacity_type = CapacityType::fixed;

	sample_rate_t sample_rate = DEFAULT_SAMPLE_RATE;
	std::array<sample_type, _capacity> samples = {};
};

template<typename _sample_t>
struct Signal<_sample_t, static_cast<size_t>(CapacityType::dynamic)> {
	using sample_type = _sample_t;

	static constexpr CapacityType capacity_type = CapacityType::dynamic;

	sample_rate_t sample_rate = DEFAULT_SAMPLE_RATE;
	std::vector<sample_type> samples;
};
} // namespace dsp
