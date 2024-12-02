#pragma once

#include <cstring>
#include <cstdint>
#include <memory>
#include <stdexcept>

// This should probably be in a separate library

enum class MessageType : uint32_t {
	UNKNOWN = 0,
	TEST = 1
};

class MessageMetadata {
private:
	MessageType m_type;

public:
	constexpr MessageMetadata() noexcept :
		m_type(MessageType::UNKNOWN)
	{ }

	MessageMetadata(const MessageMetadata& rhs) = default;
	MessageMetadata(MessageMetadata&& rhs) = default;
	MessageMetadata& operator=(const MessageMetadata& rhs) = default;
	MessageMetadata& operator=(MessageMetadata&& rhs) = default;

	~MessageMetadata() = default;

	constexpr MessageType get_type() const noexcept {
		return this->m_type;
	}

	void set_type(const MessageType type) noexcept;
};

class Message {
public:
	/// 61,440 bytes. 60 Kb. Literally just chose this at random
	static constexpr size_t MAX_MESSAGE_SIZE                = 60 * 1024;
	static constexpr char   MAX_MESSAGE_SIZE_EXCEEDED_MSG[] = "Max Message Size Exceeded. Message Size: ";

private:
	MessageMetadata m_metadata;
	uint8_t         m_data[MAX_MESSAGE_SIZE] = { 0 };
	size_t          m_data_size;

public:
	template<typename T>
	Message(const MessageMetadata& metadata, const T& data) :
		m_metadata(metadata),
		m_data(),
		m_data_size(sizeof(data)) {

		if (this->m_data_size > Message::MAX_MESSAGE_SIZE) {
			throw std::out_of_range(MAX_MESSAGE_SIZE_EXCEEDED_MSG + this->m_data_size);
		}

		memcpy(this->m_data, &data, this->m_data_size);
	}

	Message(const MessageMetadata& metadata, const void* const data, const size_t size);

	Message(const Message& rhs) noexcept = default;
	Message(Message&& rhs) noexcept = default;
	Message& operator=(const Message& rhs) noexcept = default;
	Message& operator=(Message&& rhs) noexcept = default;

	~Message() = default;

	MessageMetadata get_metadata() const noexcept;
	const uint8_t*  get_data() const noexcept;

	template<typename T>
	constexpr const T& get_data() const noexcept {
		return *reinterpret_cast<const T*>(m_data);
	}

	size_t get_data_size() const noexcept;

	void set_metadata(const MessageMetadata& metadata) noexcept;
	void set_data(const void* const data, const size_t size);

	template<typename T>
	void set_data(const T& data) {
		this->set_data(&data, sizeof(T));
	}

	friend void swap(Message& lhs, Message& rhs) noexcept {
		using std::swap;

		swap(lhs.m_metadata, lhs.m_metadata);
		swap(lhs.m_data, rhs.m_data);
		swap(lhs.m_data_size, rhs.m_data_size);
	}
};
