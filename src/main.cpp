#include <iostream>
#include <string>

#include "include/parser.h"
#include "include/execute.h"

// Forward declaration (sẽ code sau)
void executeCommand(const Command& cmd);

void printPrompt() {
    std::cout << "myShell> ";
}

int main() {
    std::string line;

    while (true) {
        printPrompt();

        // Đọc dòng lệnh từ người dùng
        if (!std::getline(std::cin, line)) {
            std::cout << "\n";
            break; // EOF (Ctrl+D / Ctrl+Z)
        }

        // Bỏ qua dòng trống
        if (line.empty()) continue;

        // Phân tích dòng lệnh
        Command cmd = parseCommand(line);

        // Xử lý lệnh đặc biệt (exit)
        if (!cmd.argv.empty() && cmd.argv[0] == "exit") {
            break;
        }

        // Gọi hàm thực thi
        executeCommand(cmd);
    }

    return 0;
}
