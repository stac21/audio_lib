#pragma once

#include <complex>
#include <tuple>
#include <fftw3.h>
#include "dsp_declarations.hpp"
#include "signals.hpp"

enum class AudioThreadState {
	PLAYING,
	PAUSED,
	IDLE,
	STARTING
};

struct AudioThreadData {
	AudioThreadState                                   state            = AudioThreadState::IDLE;
	const dsp::Signal<dsp::sample_t>*                  signal           = nullptr;
	dsp::Wave<dsp::sample_t, dsp::FRAMES_PER_BUFFER>   wave;
	int32_t                                            sample_index     = 0;
	/// Volume of the wave
	dsp::amplitude_t                                   amplitude_scalar = 1.0;
	dsp::pitch_t                                       pitch_shift      = 0.0;
	/// Size of the complex wave is the size of the real wave / 2 + 1
	static constexpr size_t COMPLEX_WAVE_SIZE = std::tuple_size_v<decltype(wave)> / 2 + 1;
	dsp::Wave<std::complex<dsp::sample_t>, COMPLEX_WAVE_SIZE> complex_wave;
	/// TODO implement the dft functionality
	fftw_plan                                          r2c_plan;
	fftw_plan                                          c2r_plan;
};

/*
 * TODO need to have some sort of structure that contains the fftw_plan,
 * buffers, and functionality to create and destroy these things that are
 * real-time audio thread safe
 */
