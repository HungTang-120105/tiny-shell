#include "../include/parser.h"
#include <sstream>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

// Split string by whitespace
static std::vector<std::string> splitTokens(const std::string &s) {
    std::vector<std::string> tokens;
    bool inQuotes = false;
    std::string token;

    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '"') {
            // Đảo trạng thái inQuotes
            inQuotes = !inQuotes;
        } else if (std::isspace(static_cast<unsigned char>(c)) && !inQuotes) {
            // Kết thúc một token nếu gặp khoảng trắng bên ngoài dấu "
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else {
            // Thêm ký tự vào token
            token.push_back(c);
        }
    }

    // Thêm token còn sót lại
    if (!token.empty()) {
        tokens.push_back(token);
    }

    return tokens;
}

// ...existing code...

Command parseCommand(const std::string &line) {
    Command cmd;
    auto tokens = splitTokens(line);
    for (size_t i = 0; i < tokens.size(); ++i) {
        const std::string &t = tokens[i];

        if (t == "&" && i == tokens.size() - 1) {
            cmd.background = true;
        }
        else if (t == ">>" && i + 1 < tokens.size()) {
            cmd.outfile = tokens[++i];
            cmd.appendMode = true;
        }
        else if (t == ">" && i + 1 < tokens.size()) {
            cmd.outfile = tokens[++i];
            cmd.appendMode = false;
        }
        else if (t == "<" && i + 1 < tokens.size()) {
            cmd.infile = tokens[++i];
        }
        else {
            cmd.argv.push_back(t);
        }
    }
    return cmd;
}