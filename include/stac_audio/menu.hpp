#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

class MenuOption {
private:
	std::unordered_map<uint64_t, std::function<void()>> on_select_callbacks;

public:
	std::string title;
	std::string description;

	MenuOption() = default;
	MenuOption(const std::string& title);
	MenuOption(const std::string& title, const std::string& description);

	~MenuOption() = default;

	/**
	 * @brief Execute the callback functions added to this menu option
	 */
	void select() const;

	/**
	 * @brief Add a callback function to execute when the option is selected
	 * @param cb Callback function to execute when the option is selected
	 * @return id of the callback function
	 */
	uint64_t add_on_select_callback(const std::function<void()>& cb);
	/**
	 * @brief Remove a callback function
	 * @param id id of the callback function
	 * @return whether a callback function at the given id was found
	 */
	bool remove_on_select_callback(const uint64_t id);

	void display() const;
};

class Menu {
public:
	std::vector<MenuOption> options;

	Menu() = default;

	~Menu() = default;

	void display() const;
};
