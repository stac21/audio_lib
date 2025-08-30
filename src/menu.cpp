#include "menu.hpp"

#include <iostream>

/*
 * Start MenuOption class
 */
MenuOption::MenuOption(const std::string& title) :
	title(title) {
}

MenuOption::MenuOption(const std::string& title, const std::string& description) :
	title(title),
	description(description) {
}

void MenuOption::select() const {
	for (decltype(this->on_select_callbacks)::const_iterator it = this->on_select_callbacks.begin(); it != this->on_select_callbacks.end(); it++) {
		// execute the std::function
		it->second();
	}
}

uint64_t MenuOption::add_on_select_callback(const std::function<void()>& cb) {
	static uint64_t id = 1;

	this->on_select_callbacks.insert({ id, cb });

	return id;
}

bool MenuOption::remove_on_select_callback(const uint64_t id) {
	return this->on_select_callbacks.erase(id);
}

void MenuOption::display() const {
	std::cout << this->title;
}
/*
 * End MenuOption class
 */

 /*
  * Start Menu class
  */
void Menu::display() const {
	for (size_t i = 0; i < this->options.size(); i++) {
		std::cout << std::to_string(i + 1) << ".";

		this->options.at(i).display();

		std::cout << "\n";
	}
}
/*
 * End Menu class
 */
