#pragma once
#include <string>
#include <vector>

// Structure to hold parsed command
struct Command {
    std::vector<std::string> argv;  // Tokens: first element is command name
    bool background = false;        // True if ends with '&'
    std::string infile;             // Input redirection file
    std::string outfile;            // Output redirection file
};

// Parse the input line into a single Command (no pipe support)
Command parseCommand(const std::string &line);