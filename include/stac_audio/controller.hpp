#pragma once

#include <array>
#include <vector>

#include "signals.hpp"

namespace stac {
/**
 * @brief Controls sending messages to the audio thread
 * as well as manages the active signal buffer which the audio thread
 * is given
 */
struct Controller {
	static constexpr uint8_t num_signal_buffers = 2;
	/**
	 * @brief Buffers which hold the signals to be sent to the audio thread.
	 * These get rotated similar to frame buffers in graphics and the workflow is
	 * as so: update inactive signal buffer -> update audio thread to point to inactive signal buffer
	 * -> receive confirmation that audio thread is now pointing to different signal buffer ->
	 * make the same updates to the now inactive signal buffer as were made to the now active buffer
	 */
	std::array<dsp::Signal<dsp::sample_t>, num_signal_buffers> signal_buffers;
	/**
	 * @brief Index of the signal buffer which the audio thread is currently pointing to
	 */
	uint8_t active_signal_buffer_index = 0;

	std::vector<dsp::Signal<dsp::sample_t>> signals;

	bool rotate_signal_buffers();
};
}
