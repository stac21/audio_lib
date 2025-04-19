#pragma once

#include "signals.hpp"
#include "lock_free_queue.hpp"

enum class AudioThreadState {
	PLAYING,
	PAUSED,
	IDLE,
	STARTING
};

class AudioThreadData {
public:
	AudioThreadState                                 state            = AudioThreadState::IDLE;
	const dsp::Signal<dsp::sample_t>*                signal           = nullptr;
	dsp::Wave<dsp::sample_t, dsp::FRAMES_PER_BUFFER> wave;
	int32_t                                          sample_index     = 0;
	/// Volume of the wave
	dsp::amplitude_t                                 amplitude_scalar = 1.0;
	dsp::pitch_t                                     pitch_shift      = 0.0;
};

/*
 * TODO need to have some sort of structure that contains the fftw_plan,
 * buffers, and functionality to create and destroy these things that are
 * real-time audio thread safe
 */
