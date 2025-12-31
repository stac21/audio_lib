#pragma once

#include <string>

#include "signals.hpp"

class AudioFileMetadata {
private:
	// format
	// num_channels
	// sample_rate
	// 
public:
};

class AudioFile {
private:
	AudioFileMetadata          metadata;
	dsp::Signal<dsp::sample_t> signal;
	std::string                file_path;

public:
	AudioFile() = default;
	AudioFile(const std::string& file_path);

	/**
	 * Should probably switch case off of the format
	 * to then read with the correct sf_read function
	 */
	void load(const std::string& file_path);
	void save();

};