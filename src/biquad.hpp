#pragma once

#include <stdint.h>

#include "dsp_declarations.hpp"

namespace dsp {
template<typename _sample_t>
class Biquad {
private:
	_sample_t a0, a1, a2, b1, b2;
	_sample_t z1, z2;
public:
	enum class Type {
		UNKNOWN    = 0,
		LOW_PASS   = 1,
		HIGH_PASS  = 2,
		BAND_PASS  = 3,
		NOTCH      = 4,
		PEAK       = 5,
		LOW_SHELF  = 6,
		HIGH_SHELF = 7
	};

	Type          type        = Type::UNKNOWN;
	sample_rate_t sample_rate = SAMPLE_RATE;
	_sample_t     f0          =_sample_t();
	bandwidth_t   q           = bandwidth_t();
	gain_db_t     peak_gain   = gain_db_t();

	Biquad() {
		this->commit();
	}

	Biquad(const Type type, const sample_rate_t sample_rate, const _sample_t f0, const bandwidth_t q, const gain_db_t peak_gain) :
			type(type),
			sample_rate(sample_rate),
			f0(f0),
			q(q),
			peak_gain(peak_gain) {
		this->commit();
	}

	/**
	 * @brief Commit changes to the parameters that occurred after initilization
	 */
	void set_params(const Type type, const sample_rate_t sample_rate, const _sample_t f0, const bandwidth_t q, const gain_db_t peak_gain) {
		this->type = type;
		this->sample_rate = sample_rate;
		this->peak_gain = peak_gain;

		this->commit();
	}

	/**
	 * @brief Apply biquad filter to a sample
	 * @param sample Sample to apply the biquad filter to
	 */
	template<typename __sample_t>
	void apply(__sample_t& sample) const {
		
	}
};
}
