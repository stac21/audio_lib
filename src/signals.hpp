#pragma once

#include <functional>
#include <array>

#include <portaudio.h>

#include "dsp_declarations.hpp"

// TODO: consider whether the samples should be clamped down to [-1.0, 1.0]

namespace dsp {
	template<typename _sample_t>
	class Frame {
	private:
		_sample_t m_left_sample;
		_sample_t m_right_sample;

	public:
		constexpr Frame() noexcept :
			m_left_sample(SAMPLE_SILENCE),
			m_right_sample(SAMPLE_SILENCE)
		{ }

		constexpr Frame(const _sample_t left_sample, const _sample_t right_sample) noexcept :
			m_left_sample(left_sample),
			m_right_sample(right_sample)
		{ }

		Frame(const Frame& rhs) = default;
		Frame& operator=(const Frame& rhs) = default;

		~Frame() = default;

		// getters
		constexpr _sample_t left_sample() const noexcept {
			return this->m_left_sample;
		}

		constexpr _sample_t right_sample() const noexcept {
			return this->m_right_sample;
		}

		// setters
		void set_left_sample(const _sample_t left_sample) noexcept {
			this->m_left_sample = left_sample;
		}

		void set_right_sample(const _sample_t right_sample) noexcept {
			this->m_right_sample = right_sample;
		}
	};

	// TODO make this into a class and initialize the class to SAMPLE_SILENCE by default
	// check ArrTest in main.cpp for guidance on how to do this
	template<typename _sample_t>
	class Wave : public std::array<Frame<_sample_t>, FRAMES_PER_BUFFER> {
	public:
		constexpr Wave() noexcept :
			std::array<Frame<_sample_t>, FRAMES_PER_BUFFER>{}
		{ }
	};

	template<typename _sample_t>
	class Signal {
	private:
		uint32_t m_sample_rate;
		std::vector<Frame<_sample_t>> m_frames;

	public:
		static constexpr uint32_t DEFAULT_SAMPLE_RATE = 44100;

		Signal() :
			m_sample_rate(DEFAULT_SAMPLE_RATE),
			m_frames()
		{ }

		Signal(const uint32_t sample_rate) :
			m_sample_rate(sample_rate),
			m_frames()
		{ }

		Signal(const std::vector<Frame<_sample_t>>& frames) :
			m_sample_rate(DEFAULT_SAMPLE_RATE),
			m_frames(frames)
		{ }

		Signal(const uint32_t sample_rate, const std::vector<Frame<_sample_t>>& frames) :
			m_sample_rate(sample_rate),
			m_frames(frames)
		{ }

		Signal(const Signal& other) = default;
		Signal& operator=(const Signal& other) = default;

		~Signal() = default;

		const std::vector<Frame<_sample_t>>& frames() const {
			return this->m_frames;
		}

		/**
		* This really shouldn't be a function I think?
		*/
		void set_frames(const std::vector<Frame<_sample_t>>& frames) {
			this->m_frames = frames;
		}

		/**
		* @brief Returns a wave, starting at the designated time
		* in the signal
		*
		* @param time the time in seconds to begin the wave
		*/
		Wave<_sample_t> make_wave(const time_t time) const {
			Wave<_sample_t> wave;

			const size_t frame_index = time / this->m_sample_rate;

			for (size_t i = 0; i < wave.size() && i + wave.size() < this->m_frames.size(); i++) {
				wave[i] = this->m_frames.at(i);
			}

			return wave;
		}
	};
}