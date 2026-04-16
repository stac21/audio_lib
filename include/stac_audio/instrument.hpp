#pragma once

#include <iostream>
#include <functional>
#include <vector>

#include "tone.hpp"
#include "signals.hpp"
#include "dsp_declarations.hpp"

namespace stac {
/**
 * @brief A simplified version of what an instrument is. Currently only supports having functions passed in
 * which are used to generate signals for the instrument though could be modified to support having audio
 * clips as the signals as well.
 */
template<typename _sample_t>
struct Instrument {
	using sample_type = _sample_t;
	/**
	 * @brief Function to use to generate signals. First argument is the fundamental frequency of the signal,
	 * second argument is the sample rate of the signal, third argument is the number of samples in the signal.
	 * Functions are expected to have the number of harmonics and other characeristics of the signal contained within them
	 */
	using signal_func_type = std::function<std::optional<dsp::Signal<sample_type>>(dsp::frequency_t, dsp::sample_rate_t, size_t)>;

	// TODO switch this to a result to indicate more granular errors
	static std::optional<Instrument> create(const signal_func_type& signal_func, const dsp::sample_rate_t sample_rate, size_t num_samples_in_signals) {
		Instrument instrument;

		instrument.signals.reserve(NUM_SEMITONES);

		for (size_t signal_index = 0; signal_index < NUM_SEMITONES; signal_index++) {
			std::optional<Tone> toneOpt = Instrument::calc_tone_from_signal_index(signal_index);

			// this case should never happen but might as well cover it anyways
			if (!toneOpt.has_value()) {
				// TODO this error should be more granular
				return std::nullopt;
			}

			const Tone& tone = toneOpt.value();

			std::optional<dsp::Signal<sample_type>> signalOpt = signal_func(tone.frequency(), sample_rate, num_samples_in_signals);

			if (!signalOpt.has_value()) {
				// TODO this error should be more granular
				return std::nullopt;
			}

			const dsp::Signal<sample_type>& signal = signalOpt.value();

			instrument.signals.push_back(std::move(signal));
		}

		return instrument;
	}

	/**
	 * @brief Get the signal corresponding to the provided tone
	 * @param tone Tone which a signal corresponds to
	 * @return Signal if the signals have been generated
	 */
	const dsp::Signal<sample_type>& get_signal(const Tone& tone) const {
		return this->signals.at(Instrument::calc_signals_index(tone));
	}

private:
	Instrument() = default;

	static constexpr size_t calc_signals_index(const Tone& tone) {
		return (static_cast<size_t>(tone.note())) * static_cast<size_t>(Octave::NUM_ENTRIES) + static_cast<size_t>(tone.octave());
	}

	static constexpr std::optional<Tone> calc_tone_from_signal_index(const size_t index) {
		const Note   note   = static_cast<Note>(  index / static_cast<size_t>(Octave::NUM_ENTRIES));
		const Octave octave = static_cast<Octave>(index % static_cast<size_t>(Octave::NUM_ENTRIES));

		return Tone::create(note, octave);
	}

	signal_func_type signal_func;
	/**
	 * Container holding the signals for the instrument. One signal per semitone.
	 * Indices calculated via calc_signals_index(Tone)
	 *
	 * This could probably be made into a single buffer for efficiency in
	 * allocation/deallocation rather than splitting the buffers between each
	 * signal, though this is fine for now
	 */
	std::vector<dsp::Signal<sample_type>> signals;
};
}
