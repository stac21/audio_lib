#include <iostream>

#include <stac_audio/signals.hpp>
#include <stac_audio/audio_thread_data.hpp>
#include <stac_audio/dsp_utils.hpp>
#include <stac_audio/biquad.hpp>

#include <portaudio.h>
#include <sndfile.h>
#include <charconv>
#include <future>
#include <limits>
#include <optional>
#include <lfmq/message.hpp>
#include <lfmq/lock_free_queue.hpp>

#define CHECK_PA_ERROR(err)\
	if (err != paNoError) {\
		std::cout << "PaError #: " << err << ", Message: " << Pa_GetErrorText(err) << "\n";\
	}\

int32_t audio_thread(dsp::Signal<dsp::sample_t> signal);
int32_t audio_thread_callback(const void* input_buffer, void* output_buffer,
	unsigned long frames_per_buffer, const PaStreamCallbackTimeInfo* time_info,
	PaStreamCallbackFlags status_flags, void* user_data);
// returns the number of messages processed
size_t process_messages(AudioThreadData& atd, const size_t num_messages);
bool process_message(AudioThreadData& atd, const lfmq::Message& msg);
bool process_play_message(AudioThreadData& atd, const dsp::time_ms_t time);
bool process_pause_message(AudioThreadData& atd);
bool process_volume_message(AudioThreadData& atd);
bool process_stop_message(AudioThreadData& atd);
std::optional<dsp::Signal<dsp::sample_t>> read_snd_file(const std::string& file_path);
void display_options();
lfmq::MessageType process_user_input();
lfmq::SpscQueue<lfmq::Message, 10> g_message_queue;

int main() {
	static constexpr char FILE_PATH[] = "C:/Users/MyNam/source/repos/audio_lib/test/file.wav";
	std::optional<dsp::Signal<dsp::sample_t>> signal;
	signal = read_snd_file(FILE_PATH);

	if (!signal.has_value()) {
		std::cout << "Unable to open file for reading: " << FILE_PATH << "\n";
		return 1;
	}

	std::future<int32_t> audio_t = std::async(std::launch::async, audio_thread, signal.value());
	lfmq::MessageType msg_type = lfmq::MessageType::UNKNOWN;

	while (msg_type != lfmq::MessageType::STOP) {
		display_options();
		msg_type = process_user_input();
	}

	audio_t.wait();

	return 0;
}

int32_t audio_thread(dsp::Signal<dsp::sample_t> signal) {
	std::cout << "Starting audio thread\n";
	PaStreamParameters stream_params;
	PaError err;

	err = Pa_Initialize();
	CHECK_PA_ERROR(err);

	stream_params.device = Pa_GetDefaultOutputDevice();
	const PaDeviceInfo* device_info = Pa_GetDeviceInfo(stream_params.device);
	if (device_info == NULL) {
		std::cout << "Error fetching device_info\n";
		return -1;
	}
	stream_params.suggestedLatency = device_info->defaultLowOutputLatency;
	stream_params.channelCount = dsp::NUM_CHANNELS;
	stream_params.sampleFormat = paFloat32;
	stream_params.hostApiSpecificStreamInfo = NULL;

	std::cout << "device_name: " << device_info->name << "\n";

	AudioThreadData atd;

	atd.state = AudioThreadState::PAUSED;
	atd.amplitude_scalar = 0.5f;

	atd.signal = &signal;

	PaStream* stream = NULL;

	err = Pa_OpenStream(&stream, NULL, &stream_params, atd.signal->sample_rate, dsp::FRAMES_PER_BUFFER,
		paClipOff, audio_thread_callback, &atd);
	CHECK_PA_ERROR(err);

	err = Pa_StartStream(stream);
	CHECK_PA_ERROR(err);

	while (Pa_IsStreamActive(stream)) {
		static constexpr std::chrono::milliseconds SLEEP_TIME(500);

		std::this_thread::sleep_for(SLEEP_TIME);
	}

	std::cout << "Past while loop\n";

	err = Pa_StopStream(stream);
	CHECK_PA_ERROR(err);

	std::cout << "Stopped stream\n";

	err = Pa_CloseStream(stream);
	CHECK_PA_ERROR(err);

	std::cout << "Closed stream\n";

	err = Pa_Terminate();
	CHECK_PA_ERROR(err)

	std::cout << "Audio thread finished execution\n";

	return err;
}

int32_t audio_thread_callback(const void* input_buffer, void* output_buffer,
		unsigned long frames_per_buffer, const PaStreamCallbackTimeInfo* time_info,
		PaStreamCallbackFlags status_flags, void* user_data) {
	PaStreamCallbackResult ret = paContinue;
	if (user_data == NULL) {
		std::cout << "user_data is null\n";
		return paComplete;
	} else if (output_buffer == NULL) {
		std::cout << "output_buffer is null\n";
		return paComplete;
	}

	AudioThreadData& atd = *static_cast<AudioThreadData*>(user_data);
	dsp::sample_t* const out_buf = static_cast<dsp::sample_t*>(output_buffer);
	const size_t out_buf_len = frames_per_buffer * dsp::NUM_CHANNELS;
	// size in bytes of out_buf
	const size_t out_buf_size = sizeof(dsp::sample_t) * out_buf_len;

	process_messages(atd, 1);

	// to measure the time it takes to complete this function, there could be a message sent
	// to the controller thread to signal when the function begins and when the function ends
	// then the controller thread could record the timstamps of each

	switch (atd.state) {
	case AudioThreadState::PLAYING:
		// populate the wave
		for (size_t i = 0; i < atd.wave.size(); i++) {
			// loop the audio
			if (atd.sample_index >= atd.signal->frames.size()) {
				atd.sample_index = 0;
			}

			atd.wave.at(i) = atd.signal->frames.at(atd.sample_index);
			atd.sample_index++;
		}

		// apply effects to the wave
		for (dsp::Frame<dsp::sample_t>& curr_frame : atd.wave) {
			// is the amplitude scalar applied before or after effects? It probably doesn't matter...
			// at least not for filters
			curr_frame.left_sample  *= atd.amplitude_scalar;
			curr_frame.right_sample *= atd.amplitude_scalar;
		}

		// copy wave into output buffer
		for (size_t i = 0; i < atd.wave.size(); i++) {
			const dsp::Frame<dsp::sample_t>& curr_frame = atd.wave.at(i);
			if (i * dsp::NUM_CHANNELS + 1 < out_buf_len) {
				out_buf[i * dsp::NUM_CHANNELS] = curr_frame.left_sample;
				out_buf[i * dsp::NUM_CHANNELS + 1] = curr_frame.right_sample;
			} else {
				// For debugging purposes. Should be replaced when a better debugging system is devised
				std::cout << "Exceeded output buffer size\n";
			}
		}

		break;
	case AudioThreadState::PAUSED:
		// TODO figure out whether this memset only needs to occur once or whether
		// it needs to occur every time the audio callback gets called.
		// Answer: This almost certainly needs to be called once, though is cheap so who really cares
		memset(out_buf, dsp::SAMPLE_SILENCE, out_buf_size);
		break;
	case AudioThreadState::IDLE:
		std::cout << "Idle. Exiting loop\n";
		ret = paComplete;
		break;
	}

	return ret;
}

size_t process_messages(AudioThreadData& atd, const size_t num_messages) {
	lfmq::Message msg;
	size_t num_messages_processed = 0;

	for (size_t i = 0; i < num_messages && g_message_queue.pop(&msg); i++) {
		process_message(atd, msg);

		num_messages_processed++;
	}

	return num_messages_processed;
}

bool process_message(AudioThreadData& atd, const lfmq::Message& msg) {
	bool successfully_processed;

	switch (msg.get_metadata().get_type()) {
	case lfmq::MessageType::PLAY_AT:
	{
		const dsp::time_ms_t play_time = msg.get_payload<dsp::time_ms_t>();
		successfully_processed = process_play_message(atd, play_time);
		break;
	}
	case lfmq::MessageType::PAUSE:
		successfully_processed = process_pause_message(atd);
		break;
	case lfmq::MessageType::VOLUME:
		successfully_processed = process_volume_message(atd);
		break;
	case lfmq::MessageType::STOP:
		successfully_processed = process_stop_message(atd);
		break;
	default:
		successfully_processed = false;
		break;
	}

	return successfully_processed;
}

bool process_play_message(AudioThreadData& atd, const dsp::time_ms_t time) {
	const size_t sample_index = dsp::utils::sample_index_from_time(atd.signal->sample_rate, time);

	if (sample_index >= atd.signal->frames.size()) {
		return false;
	}

	atd.state = AudioThreadState::PLAYING;
	atd.sample_index = sample_index;

	return true;
}

bool process_pause_message(AudioThreadData& atd) {
	if (atd.state == AudioThreadState::PAUSED) {
		atd.state = AudioThreadState::PLAYING;
	} else if (atd.state == AudioThreadState::PLAYING){
		atd.state = AudioThreadState::PAUSED;
	}

	return true;
}

bool process_volume_message(AudioThreadData& atd) {
	// toggle the mute status of the audio stream
	atd.amplitude_scalar = (static_cast<dsp::sample_t>(std::abs(atd.amplitude_scalar - 1.0))
	                        <= std::numeric_limits<dsp::sample_t>::epsilon()) ? 0.0 : 1.0;

	return true;
}

bool process_stop_message(AudioThreadData& atd) {
	atd.state = AudioThreadState::IDLE;

	return true;
}

std::optional<dsp::Signal<dsp::sample_t>> read_snd_file(const std::string& file_path) {
	SF_INFO sf_info;
	SNDFILE* sf = sf_open(file_path.c_str(), SFM_READ, &sf_info);

	if (sf == nullptr) {
		return std::nullopt;
	}

	std::cout << "sf_info.channels: " << sf_info.channels << "\n";

	dsp::Signal<dsp::sample_t> signal;

	signal.sample_rate = sf_info.samplerate;

	sf_count_t curr_frames_read = 0;
	constexpr sf_count_t NUM_FRAMES_TO_READ = 256;
	std::array<dsp::sample_t, NUM_FRAMES_TO_READ> in_buffer;
	dsp::Frame<dsp::sample_t> curr_frame;

	do {
		curr_frames_read = sf_readf_float(sf, in_buffer.data(), in_buffer.size());
		// insert the read frames into the signal
		for (size_t i = 0; i < in_buffer.size(); i += sf_info.channels) {
			curr_frame.left_sample = in_buffer.at(i);

			if (sf_info.channels == 1) {
				curr_frame.right_sample = in_buffer.at(i);
			} else {
				curr_frame.right_sample = in_buffer.at(i + 1);
			}

			signal.frames.push_back(curr_frame);
		}
	} while (curr_frames_read == NUM_FRAMES_TO_READ);

	return signal;
}

void display_options() {
	std::cout << "Choose one of the following options:\n"
		<< "1. Play Audio From Beginning\n"
		<< "2. Pause/Resume Audio\n"
		<< "3. Toggle Mute\n"
		<< "4. Stop\n"
		<< "5. Configure Effects\n"
		<< "Selected option: ";
}

lfmq::MessageType process_user_input() {
	std::string user_input;
	std::cin >> user_input;

	uint8_t option;
	std::from_chars_result result = std::from_chars(user_input.data(),
		user_input.data() + user_input.size(), option);

	lfmq::Message msg;
	lfmq::MessageMetadata msg_metadata(lfmq::MessageType::UNKNOWN);

	switch (option) {
	case 1:
		msg_metadata.set_type(lfmq::MessageType::PLAY_AT);
		// the cast to uint64_t is necessary so the Message is able to correctly deduce the type
		msg.set_payload(static_cast<dsp::time_ms_t>(0));
		break;
	case 2:
		msg_metadata.set_type(lfmq::MessageType::PAUSE);
		break;
	case 3:
		msg_metadata.set_type(lfmq::MessageType::VOLUME);
		break;
	case 4:
		msg_metadata.set_type(lfmq::MessageType::STOP);
		break;
	default:
		msg_metadata.set_type(lfmq::MessageType::UNKNOWN);
		break;
	}

	if (msg_metadata.get_type() != lfmq::MessageType::UNKNOWN) {
		msg.set_metadata(msg_metadata);

		g_message_queue.push(msg);
	}

	return msg_metadata.get_type();
}
