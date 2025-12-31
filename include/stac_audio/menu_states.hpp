#pragma once

#include <string>

// TODO switch this over to using the menu option stuff from the cpp-sandbox folder

/**
 * Which menu state the user is on
 */
enum class MenuState {
	// Main menu where the user may select a project
	MAIN,
	// General project view
	PROJECT_VIEW,
	// Effects Overview page
	EFFECTS_CONFIGURATION,
	// TODO add more states specific to the effects in order to configure them
};

/**
 * Menu options for the main menu state
 */
class MainMenuOption {
public:
	enum Value {
		CREATE_PROJECT,
		LOAD_PROJECT,
		QUIT
	};

private:
	Value m_val;

public:
	constexpr MainMenuOption() :
		m_val(Value::CREATE_PROJECT)
	{}

	constexpr MainMenuOption(const Value val) :
		m_val(val)
	{}

	constexpr operator Value() const {
		return this->m_val;
	}

	constexpr operator bool() = delete;
	constexpr operator bool() const = delete;

	constexpr bool operator==(const Value val) {
		return this->m_val == val;
	}

	constexpr bool operator!=(const Value val) {
		return this->m_val != val;
	}

	std::string to_string(const Value val) const {
		std::string str;

		switch (val) {
		case Value::CREATE_PROJECT: str = "Create New Project"; break;
		case Value::LOAD_PROJECT:   str = "Load Existing Project"; break;
		case Value::QUIT:           str = "Quit Program"; break;
		}

		return str;
	}
};

/**
 * Menu options for the project menu state
 */
class ProjectMenuOption {
public:
	enum Value {
		PLAY_AUDIO,
		PAUSE_AUDIO,
		STOP_AUDIO,
		VOLUME_SELECT,
		CONFIGURE_EFFECTS,
		SAVE,
		SAVE_AS,
	};

private:
	Value m_val;

public:
	constexpr ProjectMenuOption() :
		m_val(Value::PLAY_AUDIO)
	{}

	constexpr ProjectMenuOption(const Value val) :
		m_val(val)
	{}

	constexpr operator Value() const {
		return this->m_val;
	}

	constexpr bool operator==(const Value val) {
		return this->m_val == val;
	}

	constexpr bool operator!=(const Value val) {
		return this->m_val != val;
	}

	constexpr operator bool() = delete;
	constexpr operator bool() const = delete;

	std::string to_string(const Value val) const {
		std::string str;

		switch (val) {
		case Value::PLAY_AUDIO:        str = "Play Audio"; break;
		case Value::PAUSE_AUDIO:       str = "Pause Audio"; break;
		case Value::STOP_AUDIO:        str = "Stop Audio"; break;
		case Value::VOLUME_SELECT:     str = "Configure Volume"; break;
		case Value::CONFIGURE_EFFECTS: str = "Configure Effects"; break;
		case Value::SAVE:              str = "Save Project"; break;
		case Value::SAVE_AS:           str = "Save Project As"; break;
		}

		return str;
	}
};

/**
 * Menu options for the effects configuration menu state
 */
class EffectsConfigurationMenuOption {
public:
	enum Value {
		HIGH_PASS_FILTER,
		LOW_PASS_FILTER,
		EQUALIZATION,
		NORMALIZATION
	};

private:
	Value m_val;

public:
	constexpr EffectsConfigurationMenuOption() :
		m_val(Value::HIGH_PASS_FILTER)
	{}

	constexpr EffectsConfigurationMenuOption(const Value val) :
		m_val(val)
	{}

	constexpr operator Value() const {
		return this->m_val;
	}

	constexpr bool operator==(const Value val) {
		return this->m_val == val;
	}

	constexpr bool operator!=(const Value val) {
		return this->m_val != val;
	}

	constexpr operator bool() = delete;
	constexpr operator bool() const = delete;

	std::string to_string(const Value val) const {
		std::string str;

		switch (val) {
		case Value::HIGH_PASS_FILTER: str = "High Pass Filter"; break;
		case Value::LOW_PASS_FILTER:  str = "Low Pass Filter"; break;
		case Value::EQUALIZATION:     str = "Equalization"; break;
		case Value::NORMALIZATION:    str = "Normalization"; break;
		}

		return str;
	}
};
