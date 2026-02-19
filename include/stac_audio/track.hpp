#pragma once

#include <cstdint>
#include <vector>

#include "dsp_declarations.hpp"
#include "signals.hpp"

struct Clip
{
	/// It might make sense to have this == the index of the Clip in the Track::clips vector?
	/// So that we can easily reference clips in constant time
	uint64_t id;
	/// Whether this item should be included in the track's audio calculation
	bool     is_active;
	/**
	 * This might not make sense to include here, since the is_active flag exists already.
	 * Or the is_active flag could be removed and changed into an is_active() function
	 * which uses is_being_edited and possibly other flags to determine whether to include
	 * the clip in the resulting signal
	 *
	 * I think that this makes sense. is_active can mean that the item has been explicitly chosen
	 * to not be included in the track, while is_being_edited can mean that it is to be included
	 * in the real-time calculation but not in the static signal calculated ehad of time
	 * within the controller thread
	 */
	//
	bool     is_being_edited;
	/// Starting time of the clip, in ms
	dsp::time_ms_t start_time;
	/// Ending time of the clip, in ms. This might not need to be included in this struct
	// since the note/effect will likely have a duration field which can be used for the same thing
	dsp::time_ms_t end_time;
};

struct Track
{
	/**
	 * We could sort this vector by order of clip appearance witin the track.
	 * The disadvantage of this is that removing or adding items in the middle
	 * is slow. I could make an algorithm similar to what the heap has where
	 * it will get fragmented, though it is also unlikely that tracks would get so large
	 * that this would become a realistic problem. I could also just flag things as "removed"
	 * but keep them in the clip list until the program is restarted.
	 * This has the disadvantage of keeping things in memory when they don't need to,
	 * but does solve the issue of removing being slow. Vector might not be the best choice for this
	 * and I might need to make a more custom structure but for now I think that vector
	 * is a decent choice, especially if we are double buffering
	 */
	std::vector<Clip> clips;
	/**
	 * Calculate the resulting signal from the given clips
	 *
	 * it might make more sense to have the signal as a member variable and
	 */
	template<typename _sample_t>
	void calc_signal(dsp::Signal<_sample_t>& signal);
};
