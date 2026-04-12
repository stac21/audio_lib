#include "tone.hpp"

bool stac::Tone::set_note(const Note note) noexcept {
	if (note >= Note::NUM_ENTRIES) {
		return false;
	}

	this->m_note = note;

	return true;
}

bool stac::Tone::set_octave(const Octave octave) noexcept {
	if (octave >= Octave::NUM_ENTRIES) {
		return false;
	}

	this->m_octave = octave;

	return true;
}
