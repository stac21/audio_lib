#pragma once

#include <stdint.h>
#include <functional>
#include <chrono>

template<typename _value_t>
class Envelope {
public:
	enum Stage : uint8_t {
		IDLE    = 0,
		ATTACK  = 1,
		DECAY   = 2,
		SUSTAIN = 3,
		RELEASE = 4
	};

	/**
	 * Functions used to calculate the values during the attack, decay, and release stages of
	 * the envelope
	 */
	using ramping_func = std::function<_value_t(const std::chrono::nanoseconds&)>;
	ramping_func attack_func;
	ramping_func decay_func;
	ramping_func release_func;

	/**
	 * Amount of time it takes for the signal to reach the peak value after the trigger is
	 * initiated
	 */
	std::chrono::nanoseconds attack_duration;
	/// Peak value to be reached after the attack stage
	_value_t peak_value = _value_t();
	/**
	 * Amount of time it takes for the signal to reach the sustain value after the peak value is
	 * reached
	 */
	std::chrono::nanoseconds decay_duration;
	/// Value to remain at until the trigger is released
	_value_t sustain_value = _value_t();
	/// Amount of time it takes for the signal to reach zero once the trigger is released
	std::chrono::nanoseconds release_duration;
	/// Total duration of the envelope
	std::chrono::nanoseconds total_duration;

	Envelope(const ramping_func& attack_func, const ramping_func& decay_func,
	         const ramping_func& release_func) :
			attack_func(attack_func),
			decay_func(decay_func),
			release_func(release_func) {}

	Envelope(const ramping_func& attack_func, const ramping_func& decay_func,
	         const ramping_func& release_func, const std::chrono::nanoseconds& attack_duration,
	         const _value_t& peak_value, const std::chrono::nanoseconds& decay_duration,
	         const _value_t& sustain_value, const std::chrono::nanoseconds& release_duration) :
			attack_func(attack_func),
			decay_func(decay_func),
			release_func(release_func),
			attack_duration(attack_duration),
			peak_value(peak_value),
			decay_duration(decay_duration),
			sustain_value(sustain_value),
			release_duration(release_duration) {}

	/**
	 * @brief Trigger the envelope to begin
	 */
	template<typename _clock>
	bool trigger(const std::chrono::time_point<_clock>& time_point) {
		/*
		 * TODO figure out whether this time point is necessary.
		 * I can see it being useful for passing in another time point
		 * then getting the calculated sample at that time period
		 * or simply keeping track of when the envelope was triggered
		 */
		if (this->is_triggered()) {
			return false;
		}

		this->stage = Stage::ATTACK;

		return true;
	}

	bool is_triggered() const noexcept {
		return (this->stage != Stage::IDLE);
	}

	Stage get_stage() const noexcept {
		return this->stage;
	}

private:
	Stage stage;
};
