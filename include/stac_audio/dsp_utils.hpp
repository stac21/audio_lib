#pragma once

#include <cstdint>
#include <cmath>
#include <fstream>

#include "dsp_declarations.hpp"
#include "signals.hpp"

namespace dsp::utils {
/**
* @brief Return the value, in dB, of a sample's amplitude
*
* @param sample the sample, in amplitude scale, to get the dB value of
*/
template<typename _sample_t>
constexpr _sample_t amp_to_db(_sample_t sample) {
	return static_cast<_sample_t>(20 * std::log10(sample));
}
/**
* @brief Return the value, in amplitude scale, of a sample's dB value
*
* @param sample the sample, in dB, to get the amplitude scale value of
*/
template<typename _sample_t>
constexpr _sample_t db_to_amp(_sample_t sample) {
	return static_cast<_sample_t>(std::pow(10, sample / 20));
}

enum class WriteResult : uint8_t {
	SUCCESS      = 0,
	OPEN_FAILED  = 1,
	FLUSH_FAILED = 2
};

/**
* @brief Write the specified signal's frame data to the specified file.
* Data will be in csv format with left channel data as the first element
* of each line and right channel data as the second element of each line.
* Will overwrite any existing data that is contained with the file.
*
* THROWS std::ifstream::failure if file is unable to be opened/written to
*
* @param signal the signal to write to a file
* @param file_path the path of the file to write the signal to
*/
template<typename _sample_t, size_t _capacity>
WriteResult write_signal_to_file(const Signal<_sample_t, _capacity>& signal, const std::string& file_path) {
	std::fstream file(file_path, std::ios_base::out | std::ios_base::trunc);

	if (!file.is_open()) {
		return WriteResult::OPEN_FAILED;
	}

	file << signal.sample_rate << "\n";

	for (size_t i = 0; i < signal.samples.size(); i++) {
		static constexpr size_t num_frames_to_write_before_flushing = 50;

		const frame_real_t& frame = signal.samples.at(i);

		file << frame.left_sample << "," << frame.right_sample << "\n";

		if (i % num_frames_to_write_before_flushing == 0) {
			file.flush();

			if (file.bad()) {
				return WriteResult::FLUSH_FAILED;
			}
		}
	}

	return WriteResult::SUCCESS;
}

/**
 * @brief Get a signal sample index from a time in ms
 * @param sample_rate Sample rate of the signal
 * @param time Time in ms
 * @return Sample index corresponding to the time and sample rate
 */
constexpr size_t sample_index_from_time(const sample_rate_t sample_rate, const time_ms_t time) {
	return static_cast<size_t>(time * (sample_rate / 1000.0));
}
} // namespace dsp::utils
