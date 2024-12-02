#include "message.hpp"

/*
 * Start MessageMetadata class definitions
 */
void MessageMetadata::set_type(const MessageType type) noexcept {
	this->m_type = type;
}
/*
 * End MessageMetadata class definitions
 */

 /*
  * Start Message class definitions
  */
Message::Message(const MessageMetadata& metadata, const void* const data, const size_t size) :
	m_metadata(metadata),
	m_data(),
	m_data_size(size) {

	if (this->m_data_size > Message::MAX_MESSAGE_SIZE) {
		throw std::out_of_range(MAX_MESSAGE_SIZE_EXCEEDED_MSG + this->m_data_size);
	}

	memcpy(m_data, data, size);
}

MessageMetadata Message::get_metadata() const noexcept {
	return this->m_metadata;
}

const uint8_t* Message::get_data() const noexcept {
	return this->m_data;
}

size_t Message::get_data_size() const noexcept {
	return this->m_data_size;
}

void Message::set_metadata(const MessageMetadata& metadata) noexcept {
	this->m_metadata = metadata;
}

void Message::set_data(const void* const data, const size_t size) {
	if (this->m_data_size > Message::MAX_MESSAGE_SIZE) {
		throw std::out_of_range(MAX_MESSAGE_SIZE_EXCEEDED_MSG + this->m_data_size);
	}

	memcpy(m_data, data, size);
	this->m_data_size = size;
}
/*
 * End Message class definitions
 */
