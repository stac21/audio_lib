#include <iostream>

#include <thread>
#include <chrono>

#include <sndfile.h>
#include <portaudio.h>
#include <fstream>

#include "dsp_declarations.hpp"
#include "message.hpp"
#include "lock_free_queue.hpp"
#include "signals.hpp"
#include "audio_thread_data.hpp"

#include <complex>
#include <fftw3.h>

#define CHECK_PA_ERROR(err)\
	if (err != paNoError) {\
		std::cout << "PaError #: " << err << ", Message: " << Pa_GetErrorText(err) << "\n";\
	}\

inline std::ostream& operator<<(std::ostream& ostream, const SF_INFO& sf_info) {
	ostream
		<< "frames: " << sf_info.frames
		<< ", samplerate: " << sf_info.samplerate
		<< ", channels: " << sf_info.channels
		<< ", format: " << sf_info.format
		<< ", sections: " << sf_info.sections
		<< ", seekable: " << sf_info.seekable;

	return ostream;
}

int32_t callback_func(const void* input_buffer, void* output_buffer, unsigned long frames_per_buffer,
	const PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags status_flags, void* user_data) {

	int32_t ret = paContinue;
	float* out_buf = static_cast<float*>(output_buffer);
	sf_count_t num_frames_to_read = frames_per_buffer;
	SNDFILE* sf = static_cast<SNDFILE*>(user_data);

	int32_t num_frames_read = sf_readf_float(sf, out_buf, num_frames_to_read);

	if (num_frames_read != num_frames_to_read) {
		ret = paComplete;
	}

	return ret;
}

void write_fft_file(const std::string& file_path, const fftw_complex* const buffer, size_t num_elems) {
	std::fstream out_file(file_path, std::ios_base::out);

	if (!out_file.is_open()) {
		std::cout << "Could not open outfile for writing\n";
		return;
	}

	static constexpr bool REAL_COMPONENT_INDEX = 0;
	static constexpr bool IMAGINARY_COMPONENT_INDEX = 1;

	for (size_t i = 0; i < num_elems; i++) {
		const fftw_complex* const curr_elem = &buffer[i];


		out_file << (*curr_elem)[REAL_COMPONENT_INDEX] << "," << (*curr_elem)[IMAGINARY_COMPONENT_INDEX] << "\n";
	}

	out_file.flush();

	if (out_file.fail()) {
		std::cout << "Failed to write outfile\n";
		return;
	}

	std::cout << "Finished writing to: " << file_path << "\n";
}

template<typename T>
void write_real_file(const std::string& file_path, const T* const buffer, size_t num_elems) {
	std::fstream out_file(file_path, std::ios_base::out);

	if (!out_file.is_open()) {
		std::cout << "Could not open outfile for writing\n";
		return;
	}

	for (size_t i = 0; i < num_elems; i++) {
		const T* const curr_elem = &buffer[i];

		out_file << *curr_elem << "\n";
	}

	out_file.flush();

	if (out_file.fail()) {
		std::cout << "Failed to write outfile\n";

		return;
	}

	std::cout << "Finished writing to: " << file_path << "\n";
}

int main() {
	static constexpr char INPUT_FILE_PATH[] = "C:/Users/MyNam/source/repos/audio_lib/test/file.wav";
	SF_INFO sf_info;
	SNDFILE* sf = sf_open(INPUT_FILE_PATH, SFM_READ, &sf_info);

	if (sf == nullptr) {
		std::cout << "Sound file could not be read";
		return 1;
	}

	sf_count_t curr_frames_read = 0;
	constexpr sf_count_t NUM_FRAMES_TO_READ = 256;
	std::array<dsp::sample_t, NUM_FRAMES_TO_READ> buffer;
	// these are doubles for the purposes of the fftw demo, but in "production" I need
	// to figure out the best way to convert between dsp::sample_t and the double
	// which fftw expects for their functions. Maybe just make dsp::sample_t a double
	std::vector<double> left_samples_in;
	std::vector<double> right_samples_in;

	do {
		curr_frames_read = sf_readf_float(sf, buffer.data(), NUM_FRAMES_TO_READ);
		// insert the read frames into the samples vector
		for (size_t i = 0; i < buffer.size(); i += 2) {
			left_samples_in.push_back(buffer.at(i));
			right_samples_in.push_back(buffer.at(i + 1));
		}
	} while (curr_frames_read == NUM_FRAMES_TO_READ);

	std::cout << "Left samples read: " << left_samples_in.size() << "\n";
	std::cout << "Right samples read: " << right_samples_in.size() << "\n";

	/*
	 * the plan must be created before the input array is filled with data,
	 * because fftw clears out the input array when the plan is created.
	 * In this case, I made a copy of the data then did a memcpy of the copy
	 * to the original input buffer. This is not the ideal way to do things, obviously
	 *
	 * When fftw_execute(plan) is called, the value of the DFT computed will be stored in the
	 * out buffer, in this case left_samples_out. When done with the plan,
	 * it must be destroyed with fftw_destroy_plan(plan). The out buffer can
	 * be destroyed with fftw_free(out). The 0 index of the out buffer will
	 * store the zero-frequency (DC) component.
	 *
	 * A fftw_complex is just a typedef for a double[2] where index 0 is the real
	 * part of the number, and index 1 is the imaginary part of the number
	 */
	std::vector<double> left_samples_copy = left_samples_in;
	const size_t left_samples_num_elems = left_samples_in.size() / 2 + 1;
	fftw_complex* left_samples_out = fftw_alloc_complex(left_samples_num_elems);
	size_t left_samples_out_size = left_samples_num_elems * sizeof(fftw_complex);
	memset(left_samples_out, 0, left_samples_out_size);
	fftw_plan real_to_complex_plan = fftw_plan_dft_r2c_1d(left_samples_in.size(), left_samples_in.data(), left_samples_out, FFTW_PATIENT);
	std::vector<double> left_samples_r2c;
	left_samples_r2c.assign(left_samples_in.size(), 0.0);
	fftw_plan complex_to_real_plan = fftw_plan_dft_c2r_1d(left_samples_r2c.size(), left_samples_out, left_samples_r2c.data(), FFTW_PATIENT);
	memcpy(left_samples_in.data(), left_samples_copy.data(), left_samples_copy.size() * sizeof(double));

	write_real_file("C:/Users/MyNam/source/repos/audio_lib/test/original_real.real", left_samples_in.data(), left_samples_in.size());

	write_fft_file("C:/Users/Mynam/source/repos/audio_lib/test/before_left_fft_data.complex", left_samples_out, left_samples_num_elems);

	// this will write to left_samples_out
	fftw_execute(real_to_complex_plan);

	write_fft_file("C:/Users/Mynam/source/repos/audio_lib/test/after_left_fft_data.complex", left_samples_out, left_samples_num_elems);

	// this will write to left_samples_r2c
	fftw_execute(complex_to_real_plan);
	// normalize the samples to convert back to the original "real" samples
	for (auto it = left_samples_r2c.begin(); it != left_samples_r2c.end(); it++) {
		*it /= left_samples_r2c.size();
	}

	write_real_file("C:/Users/MyNam/source/repos/audio_lib/test/complex_to_real.real", left_samples_r2c.data(), left_samples_r2c.size());

	// TODO I should probably make a Complex class that can be casted to a fftw_complex. Or just have a double* data() function for it.
	// From the fftw documentation, apparently the C++ std::complex type might work via a typecast

	// TODO make an octave script that can parse and graph the data from write_fft_file. DONE

	fftw_free(left_samples_out);
	fftw_destroy_plan(real_to_complex_plan);
	fftw_destroy_plan(complex_to_real_plan);

	return 0;

	/*constexpr size_t NUM_DIMENSIONS = 1;

	const pocketfft::shape_t left_shape(NUM_DIMENSIONS);
	const pocketfft::shape_t right_shape(NUM_DIMENSIONS);

	const pocketfft::stride_t left_stride_in(NUM_DIMENSIONS, sizeof(dsp::sample_t));
	const pocketfft::stride_t right_stride_in(NUM_DIMENSIONS, sizeof(dsp::sample_t));

	std::vector<std::complex<dsp::sample_t>> left_samples_out(left_samples_in.size(), 0);
	std::vector<std::complex<dsp::sample_t>> right_samples_out(right_samples_in.size(), 0);

	pocketfft::stride_t left_stride_out(sizeof(std::complex<dsp::sample_t>));
	const pocketfft::stride_t left_stride_out_copy = left_stride_out;
	pocketfft::stride_t right_stride_out(sizeof(std::complex<dsp::sample_t>));

	std::cout << "left_shape.size(): " << left_shape.size() << ", left_stride_in.size(): "
		<< left_stride_in.size() << ", left_stride_out.size(): " << left_stride_out.size() << "\n";

	std::cout << "Attempting to r2c\n";
	pocketfft::r2c(left_shape, left_stride_in, left_stride_out, 0, pocketfft::FORWARD,
		left_samples_in.data(), left_samples_out.data(), 1.f, 1);
	std::cout << "Finished r2c\n";

	std::cout << "input samples size: " << left_samples_in.size() << ", output samples size: "
		<< left_samples_out.size() << "\n";

	std::fstream out_file("C:/Users/Mynam/source/repos/audio_lib/test/left_fft_data.bin",
		std::ios_base::binary | std::ios_base::out);
	if (!out_file.is_open()) {
		std::cout << "Could not open outfile for writing\n";
		return 1;
	}

	const size_t output_size = left_samples_out.size() * sizeof(std::vector<std::complex<dsp::sample_t>>::value_type);
	std::cout << "expected sizeof output data: " << output_size << "\n";

	const bool left_stride_changed = left_stride_out != left_stride_out_copy;
	std::cout << "left stride changed: " << left_stride_changed << "\n";
	if (!left_stride_changed) {
		return 1;
	}

	out_file.write(reinterpret_cast<const char*>(left_samples_out.data()), left_samples_out.size() * sizeof(std::vector<std::complex<dsp::sample_t>>::value_type));
	out_file.flush();
	if (out_file.fail()) {
		std::cout << "Failed to write outfile\n";

		return 1;
	}
	
	return 0;*/
}

//struct Foo {
//	int i;
//	char str[10];
//
//	friend std::ostream& operator<<(std::ofstream& os, const Foo& f) {
//		os << "i: " << f.i << ", str: " << f.str << "\n";
//	}
//};
//
//int main() {
//	lfmq::MessageMetadata metadata;
//	metadata.set_type(lfmq::MessageType::UNKNOWN);
//
//	Foo f = {
//		.i = 1,
//		.str = "Hello"
//	};
//
//	const lfmq::Message message(metadata, f);
//
//	Foo f_copy = message.get_payload<Foo>();
//
//	//std::cout << f_copy << "\n";
//
//	static constexpr char FILE_PATH[] = "C:/Users/MyNam/source/repos/audio_lib/test/file.wav";
//	SF_INFO sf_info;
//	SNDFILE* sf = sf_open(FILE_PATH, SFM_READ, &sf_info);
//	if (sf == nullptr) {
//		std::cout << "File could not be read";
//		return 1;
//	}
//
//	PaError err = Pa_Initialize();
//
//	std::cout << "sf_info: {" << sf_info << "}" << "\n";
//
//	PaStream* stream = nullptr;
//
//	PaDeviceIndex default_device_index = Pa_GetDefaultOutputDevice();
//	const PaDeviceInfo* default_device_info = Pa_GetDeviceInfo(default_device_index);
//	if (default_device_info == nullptr) {
//		std::cout << "default_device_index: " << default_device_index << "\n";
//
//		return 1;
//	}
//
//	PaStreamParameters output_params;
//	output_params.device = default_device_index;
//	output_params.channelCount = sf_info.channels;
//	output_params.hostApiSpecificStreamInfo = nullptr;
//	output_params.sampleFormat = paFloat32;
//	output_params.suggestedLatency = default_device_info->defaultLowOutputLatency;
//
//	err = Pa_OpenStream(&stream, nullptr, &output_params, sf_info.samplerate, 256, paClipOff,
//		callback_func, sf);
// 
//	CHECK_PA_ERROR(err);
//
//	err = Pa_StartStream(stream);
//	CHECK_PA_ERROR(err);
//
//	while (Pa_IsStreamActive(stream)) {
//		std::this_thread::sleep_for(std::chrono::seconds(1));
//	}
//
//	err = Pa_StopStream(stream);
//	CHECK_PA_ERROR(err);
//
//	err = Pa_CloseStream(stream);
//	CHECK_PA_ERROR(err);
//
//	Pa_Terminate();
//
//	int32_t sf_err = sf_close(sf);
//	if (sf_err != 0) {
//		std::cout << "Error closing soundfile\n";
//	}
//
//	return 0;
//}
