#include "../include/parser.h"
#include <sstream>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

// Split string by whitespace
static std::vector<std::string> splitTokens(const std::string &s) {
    std::istringstream iss(s);
    std::vector<std::string> tokens;
    std::string tok;
    while (iss >> tok) tokens.push_back(tok);
    return tokens;
}

Command parseCommand(const std::string &line) {
    Command cmd;
    auto tokens = splitTokens(line);
    for (size_t i = 0; i < tokens.size(); ++i) {
        const std::string &t = tokens[i];
        if (t == "&" && i == tokens.size() - 1) {
            cmd.background = true;
        } else if (t == ">" && i + 1 < tokens.size()) {
            cmd.outfile = tokens[++i];
        } else if (t == "<" && i + 1 < tokens.size()) {
            cmd.infile = tokens[++i];
        } else {
            cmd.argv.push_back(t);
        }
    }
    return cmd;
}