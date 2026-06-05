#include <iostream>

#include <stac_audio/dsp_declarations.hpp>
#include <stac_audio/signals.hpp>
#include <stac_audio/audio_thread_data.hpp>
#include <stac_audio/dsp_utils.hpp>
#include <stac_audio/tone.hpp>

#include <portaudio.h>
#include <sndfile.h>
#include <charconv>
#include <future>
#include <limits>
#include <optional>
#include <numbers>
#include <stac_audio/message.hpp>
#include <stac_audio/lock_free_queue.hpp>

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
bool process_effect_added(AudioThreadData& atd, const dsp::Signal<dsp::sample_t>* signal_ptr);
std::optional<dsp::Signal<dsp::sample_t>> read_snd_file(const std::string& file_path);
std::optional<dsp::Signal<dsp::sample_t>> generate_sin_wave(dsp::frequency_t frequency, dsp::sample_rate_t sample_rate, dsp::time_ms_t duration);
void display_options();
lfmq::MessageType process_user_input();
lfmq::SpscQueue<lfmq::Message, 10> g_message_queue;

int main() {
	static constexpr char FILE_PATH[] = "C:/Users/MyNam/source/repos/audio_lib/test/file.wav";
	//static constexpr char FILE_PATH[] = "/home/grant/projects/git/audio_lib/test/file.wav";
	std::optional<dsp::Signal<dsp::sample_t>> signal;
	signal = read_snd_file(&FILE_PATH[0]);

	if (!signal.has_value()) {
		std::cout << "Unable to open file for reading: " << &FILE_PATH[0] << "\n";
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
	if (device_info == nullptr) {
		std::cout << "Error fetching device_info\n";
		return -1;
	}
	stream_params.suggestedLatency = device_info->defaultLowOutputLatency;
	stream_params.channelCount = dsp::NUM_CHANNELS;
	stream_params.sampleFormat = paFloat32;
	stream_params.hostApiSpecificStreamInfo = nullptr;

	std::cout << "device_name: " << device_info->name << "\n";

	AudioThreadData atd;

	atd.state = AudioThreadState::PAUSED;

	atd.signal = &signal;

	PaStream* stream = nullptr;

	err = Pa_OpenStream(&stream, nullptr, &stream_params, atd.signal->sample_rate, dsp::FRAMES_PER_BUFFER,
		paClipOff, audio_thread_callback, &atd);
	CHECK_PA_ERROR(err);

	err = Pa_StartStream(stream);
	CHECK_PA_ERROR(err);

	while (Pa_IsStreamActive(stream) != 0) {
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
	if (user_data == nullptr) {
		std::cout << "user_data is null\n";
		return paComplete;
	}
	if (output_buffer == nullptr) {
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
				out_buf[i * dsp::NUM_CHANNELS]     = curr_frame.left_sample;
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
		memset(out_buf, static_cast<int>(dsp::SAMPLE_SILENCE), out_buf_size);
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
	case lfmq::MessageType::EFFECT_ADDED:
	{
		const dsp::Signal<dsp::sample_t>* const signal_ptr = msg.get_payload<const dsp::Signal<dsp::sample_t>* const>();
		successfully_processed = process_effect_added(atd, signal_ptr);
		break;
	}
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

bool process_effect_added(AudioThreadData& atd, const dsp::Signal<dsp::sample_t>* const signal_ptr) {
	atd.state        = AudioThreadState::PLAYING;
	atd.sample_index = 0;
	atd.signal       = signal_ptr;

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
	std::array<dsp::sample_t, NUM_FRAMES_TO_READ> in_buffer = {};
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

std::optional<dsp::Signal<dsp::sample_t>> generate_sin_wave(const dsp::frequency_t frequency, dsp::sample_rate_t sample_rate, const dsp::time_ms_t duration) {
	// important to note that the phase of the end resulting signal
	// does not matter, but the relative phase of the harmonic signals
	// does matter as they can constructively or destructively combine
	// to make the signal louder or quieter. Also could change the
	// timbre of the resulting signal
	// https://pudding.cool/2018/02/waveforms/
	static constexpr float ms_to_seconds = 1000.0f;
	const size_t num_samples_in_signal = static_cast<size_t>((duration / ms_to_seconds) * sample_rate);
	std::cout << "num_samples_in_signal: " << num_samples_in_signal << "\n";
	std::cout << "frequency: " << frequency << "\n";

	dsp::Signal<dsp::sample_t> signal(sample_rate);
	try {
		signal.frames.reserve(num_samples_in_signal);
	} catch (std::bad_alloc& e) {
		// the requested buffer size was too large to allocate
		return std::nullopt;
	}

	dsp::sample_t curr_sample = 0.0;

	for (size_t sample_index = 0; sample_index < num_samples_in_signal; sample_index++) {
		curr_sample = static_cast<dsp::sample_t>(std::sin(frequency * 2 * std::numbers::pi * (static_cast<double>(sample_index) / sample_rate)));

		signal.frames.emplace_back(curr_sample, curr_sample);
	}

	return signal;
}

enum class UserChoices : uint8_t {
	PLAY_AUDIO_FROM_BEGINNING = 1,
	PAUSE_OR_RESUME_AUDIO     = 2,
	TOGGLE_MUTE               = 3,
	STOP_PLAYBACK             = 4,
	PLAY_NOTE                 = 5
};

void display_options() {
	std::cout << "Choose one of the following options:\n"
		<< static_cast<int32_t>(UserChoices::PLAY_AUDIO_FROM_BEGINNING) << ". Play Audio From Beginning\n"
		<< static_cast<int32_t>(UserChoices::PAUSE_OR_RESUME_AUDIO)     << ". Pause/Resume Audio\n"
		<< static_cast<int32_t>(UserChoices::TOGGLE_MUTE)               << ". Toggle Mute\n"
		<< static_cast<int32_t>(UserChoices::STOP_PLAYBACK)             << ". Stop\n"
		<< static_cast<int32_t>(UserChoices::PLAY_NOTE)                 << ". Play Note\n"
		<< "Selected option: ";
}

lfmq::MessageType process_user_input() {
	std::string user_input;
	std::cin >> user_input;

	uint8_t option{};
	std::from_chars_result result = std::from_chars(user_input.data(),
		user_input.data() + user_input.size(), option);

	// error while parsing the input
	if (result.ec != std::errc()) {
		return lfmq::MessageType::UNKNOWN;
	}

	lfmq::Message msg;
	lfmq::MessageMetadata msg_metadata(lfmq::MessageType::UNKNOWN);

	switch (static_cast<UserChoices>(option)) {
	case UserChoices::PLAY_AUDIO_FROM_BEGINNING:
		msg_metadata.set_type(lfmq::MessageType::PLAY_AT);
		// the cast to uint64_t is necessary so the Message is able to correctly deduce the type
		msg.set_payload(static_cast<dsp::time_ms_t>(0));
		break;
	case UserChoices::PAUSE_OR_RESUME_AUDIO:
		msg_metadata.set_type(lfmq::MessageType::PAUSE);
		break;
	case UserChoices::TOGGLE_MUTE:
		msg_metadata.set_type(lfmq::MessageType::VOLUME);
		break;
	case UserChoices::STOP_PLAYBACK:
		msg_metadata.set_type(lfmq::MessageType::STOP);
		break;
	case UserChoices::PLAY_NOTE:
		// TODO remove the message types from lfmq and place them in
		// the controller layer
		if (std::optional<stac::Tone> toneOpt = stac::Tone::create(stac::Note::A, stac::Octave::TWO); toneOpt.has_value()) {
			const stac::Tone& tone = toneOpt.value();

			msg_metadata.set_type(lfmq::MessageType::EFFECT_ADDED);

			static constexpr dsp::time_ms_t signal_duration = 3000;

			// TODO this should really go into a controller-type class
			// but this will do for now
			//
			// It's fine to not check has_value here because I know that the operation will succeed
			// but the result of a signal generation should be checked generally
			static dsp::Signal<dsp::sample_t> signal = generate_sin_wave(tone.frequency(), dsp::SAMPLE_RATE, signal_duration).value();
			const dsp::utils::WriteResult write_result = dsp::utils::write_signal_to_file(signal, "/home/grant/projects/git/audio_lib/plots/signal.sig");

			std::cout << "result of writing signal to file: " << static_cast<int32_t>(write_result) << "\n";

			msg.set_payload(&signal);

			std::cout << "Made effect added message\n";
		} else {
			std::cout << "Unable to create tone\n";

			msg_metadata.set_type(lfmq::MessageType::UNKNOWN);
		}
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
