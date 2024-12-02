#pragma once

#include <cmath>
#include <fstream>

#include "dsp_declarations.hpp"
#include "signals.hpp"

namespace dsp {
/**
* @brief Return the value, in dB, of a sample's amplitude
*
* @param sample the sample, in amplitude scale, to get the dB value of
*/
sample_t amp_to_db(sample_t sample);
/**
* @brief Return the value, in amplitude scale, of a sample's dB value
*
* @param sample the sample, in dB, to get the amplitude scale value of
*/
sample_t db_to_amp(sample_t sample);
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
template<typename _sample_t>
void write_signal_to_file(const Signal<_sample_t>& signal, const std::string& file_path) {
	std::fstream file;
	file.exceptions(std::ios_base::badbit);

	file.open(file_path, std::ios_base::out | std::ios_base::trunc);

	const std::vector<Frame<_sample_t>>& frames = signal.frames();

	for (const Frame<_sample_t>& frame : frames) {
		file << frame.left_sample() << ", " << frame.right_sample() << "\n";
		file.flush();
	}

	file.close();
}

} // namespace dsp
