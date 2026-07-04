#pragma once

namespace dsp {
template<typename _sample_t>
struct Frame {
	using sample_type = _sample_t;

	sample_type left_sample = {};
	sample_type right_sample = {};
};
}
