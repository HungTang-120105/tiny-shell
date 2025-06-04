#pragma once

#include <string>
#include <vector>

// Adds a command to the history.
void add_to_command_history(const std::string& command);

// Retrieves the command history.
const std::vector<std::string>& get_command_history();

// Clears the command history.
void clear_all_command_history(); 