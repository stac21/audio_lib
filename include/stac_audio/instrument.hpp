#pragma once

#include <functional>

#include "tone.hpp"
#include "signals.hpp"

namespace stac {
template<typename _sample_t>
struct Instrument {
	using sample_type = _sample_t;
	// TODO define what an instrument is
	explicit Instrument(const std::function<dsp::Signal<sample_type>(dsp::frequency_t)>& signal_func);

	void generate_signals();

	/**
	 * @brief Get the signal corresponding to the provided tone
	 * @param tone Tone which a signal corresponds to
	 */
	const dsp::Signal<sample_type>& get_signal(Tone tone) const;
};
}
