#pragma once

#include <vector>
#include <cstdint>

namespace stac {
// TODO need to test this or just compare it to
// the one which I made at work
template<typename _sample_t>
class MovingAverage {
public:
	using sample_type = _sample_t;

	explicit MovingAverage(size_t capacity) {
		this->samples.reserve(capacity);
	}

	void insert(sample_type sample) {
		if (!this->is_valid()) {
			this->num_samples++;
		}

		if (this->samples.size() < this->samples.capacity()) {
			this->samples.push_back(std::move(sample));
		} else {
			const sample_type& curr_sample = this->samples.at(this->curr_index);

			this->sum -= curr_sample;
			this->sum += sample;

			this->samples.at(this->curr_index) = std::move(sample);
		}

		// wrap around when we have reached the maximum number of samples
		if (this->curr_index == (this->samples.capacity() - 1)) {
			curr_index = 0;
		} else {
			curr_index++;
		}
	}

	bool is_valid() const noexcept {
		return (this->num_samples == this->samples.capacity());
	}

	sample_type get_average() const {
		return this->sum / this->samples.capacity();
	}

	void reset() {
		this->curr_index = 0;
		this->num_samples = 0;
		this->sum = {};
	}

private:
	size_t curr_index = 0;
	size_t num_samples = 0;
	sample_type sum = {};
	std::vector<sample_type> samples;
};
}
