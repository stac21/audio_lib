#include "dsp_utils.hpp"

namespace dsp::utils {
 
sample_t amp_to_db(sample_t sample) {
    return static_cast<sample_t>(20 * std::log10(sample));
}

sample_t db_to_amp(sample_t sample) {
    return static_cast<sample_t>(std::pow(10, sample / 20));
}
} // namespace dsp::utils
