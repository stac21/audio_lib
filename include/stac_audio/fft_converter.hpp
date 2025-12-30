#pragma once

#include <fftw3.h>
#include <type_traits>
#include <concepts>

// Idea: could make this take in a dimension parameter in the future
template<typename _sample_t> requires std::convertible_to<_sample_t, double>
class R2CConverter {
private:
	fftw_plan plan;

public:
	void execute() {
		fftw_execute(plan);
	}
};

/*
 * TODO
 * Have the user be able to pass in the buffer of real samples
 * as well as a buffer of complex samples
 * so that the audio thread's wave may be passed in as an argument.
 * A container of complex numbers also must be added to the AudioThreadData struct,
 * and neither of these containers will be owned by the FFTConverter.
 * TODO
 * Need to check whether the Frame structure can be casted into a double[2]
 * for the purposes of using it within fftw
 */
template<typename _sample_t> requires std::convertible_to<_sample_t, double>
class FFTConverter {
public:
	enum class AllocationStrategy : bool {
		PATIENT,
		IMPATIENT
	};

private:
	AllocationStrategy allocation_strategy ;
	_sample_t*         left_samples_real = nullptr;
	_sample_t*         right_samples_real = nullptr;
	fftw_complex*      left_samples_complex = nullptr;
	fftw_complex*      right_samples_complex = nullptr;
	fftw_plan          left_real_to_complex_plan;
	fftw_plan          left_complex_to_real_plan;
	fftw_plan          right_real_to_complex_plan;
	fftw_plan          right_complex_to_real_plan;
	size_t             num_real_samples = 0;
	// Equivalent to num_real_samples / 2 + 1
	size_t             num_complex_samples = 0;
	// Whether the resources have been allocated
	bool               are_resources_allocated = false;

public:

	FFTConverter(const AllocationStrategy allocation_strategy, const size_t num_real_samples, const bool alloc_immediately = false) :
			allocation_strategy(allocation_strategy),
			num_real_samples(num_real_samples),
			num_complex_samples(num_real_samples / 2 + 1) {
		if (alloc_immediately) {
			this->allocate_resources();
		}
	}

	// delete the copy constructors for now until they are needed
	FFTConverter(const FFTConverter& rhs) = delete;
	FFTConverter& operator=(const FFTConverter& rhs) = delete;

	~FFTConverter() {
		if (this->are_resources_allocated) {
			delete[] this->left_samples_real;
			delete[] this->right_samples_real;
			fftw_free(this->left_samples_complex);
			fftw_free(this->right_samples_complex);
			fftw_destroy_plan(this->left_real_to_complex_plan);
			fftw_destroy_plan(this->left_complex_to_real_plan);
			fftw_destroy_plan(this->right_real_to_complex_plan);
			fftw_destroy_plan(this->right_complex_to_real_plan);
		}
	}

	/**
	 * @brief  Allocate the resources necessary for the FFT conversion
	 * @note   This may take several seconds, so it is a good idea to allocate the resources at program startup
	 * @return True if resources were allocated at the time of this call, false if they were previously allocated
	 */
	bool allocate_resources() {
		if (this->are_resources_allocated) {
			return false;
		}

		this->are_resources_allocated = true;
		uint32_t plan_flags = (this->allocation_strategy == AllocationStrategy::PATIENT) ? FFTW_PATIENT : 0;

		this->left_samples_real = new _sample_t[this->num_real_samples];
		this->right_samples_real = new _sample_t[this->num_real_samples];
		this->left_samples_complex = fftw_alloc_complex(this->num_complex_samples);
		this->right_samples_complex = fftw_alloc_complex(this->num_complex_samples);
		this->left_real_to_complex_plan = fftw_plan_dft_r2c_1d(this->num_real_samples, this->left_samples_real,
		                                                       this->left_samples_complex, plan_flags);
		this->right_real_to_complex_plan = fftw_plan_dft_r2c_1d(this->num_real_samples, this->right_samples_real,
		                                                        this->right_samples_complex, plan_flags);
		this->left_complex_to_real_plan = fftw_plan_dft_c2r_1d(this->num_complex_samples, this->left_samples_complex,
		                                                       this->left_samples_real, plan_flags);
		this->right_complex_to_real_plan = fftw_plan_dft_c2r_1d(this->num_complex_samples, this->right_samples_complex,
		                                                        this->right_samples_real, plan_flags);

		return true;
	}

	r2c()
};
