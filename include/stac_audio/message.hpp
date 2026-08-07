#pragma once

#include <algorithm>
#include <cstring>
#include <type_traits>

namespace stac::lfmq
{
enum class MessageType {
	UNKNOWN,         // Unknown message type
	RESUME,          // Resume the audio stream
	PAUSE,           // Pause the audio stream
	STOP,            // Stop audio playback and shut down the audio thread
	VOLUME,          // Adjust the volume of the audio stream
	RESIZE,          // Inform the audio thread that one of its dynamic buffers has been resized on the controller thread
	EFFECT_ADDED,    // A new effect has been added by the user
	EFFECT_REMOVED,  // An effect has been removed by the user
	EFFECT_ENABLED,  // Enable an effect
	EFFECT_DISABLED, // Disable an effect
	PLAY_AT          // begin playing at specific time or frame index
};

struct MessageMetadata {
	MessageType type = MessageType::UNKNOWN;
};

template<size_t _max_message_size = 64>
class Message {
public:
	MessageMetadata metadata = {};

	Message() :
			payload{},
			payload_size(0) {}

	template<typename _T> requires (sizeof(_T) <= _max_message_size)
	Message(MessageMetadata metadata, const _T& data) :
			metadata(std::move(metadata)) {
		this->set_payload(data);
	}

	constexpr const char* get_payload() const noexcept {
		return &this->payload[0];
	}

	template<typename _T> requires (sizeof(_T) <= _max_message_size)
	constexpr const _T& get_payload() const noexcept {
		/*
		 * in the case of T deducing to a pointer type, this will treat
		 * the message payload as a T**
		 */
		return *reinterpret_cast<const _T*>(&this->payload[0]);
	}

	constexpr size_t get_payload_size() const noexcept {
		return this->payload_size;
	}

	/**
	 * @brief Set the payload to the passed in data
	 * @param data Data to copy into the payload
	 * @eturn True if copy was successful, false if data was nullptr
	 */
	template<typename _T> requires (sizeof(_T) <= _max_message_size)
	bool set_payload(const _T& data) {
		if constexpr (std::is_pointer_v<_T>) {
			if (data == nullptr) {
				return false;
			}
		}

		/*
		 * in the case of T deducing to a pointer type, this will treat
		 * the message payload as a T**
		 */
		this->payload_size = sizeof(data);
		memcpy(&this->payload[0], &data, this->payload_size);

		return true;
	}

private:
	char   payload[_max_message_size] = {};
	/// Size of the payload in bytes
	size_t payload_size = 0;
};
}
