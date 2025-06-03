#include <iostream>
#include <string>
#include <windows.h> 

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

#include "include/parser.h"
#include "include/execute.h"
#include <thread> 
#include <chrono>
#include "include/process_manager.h"

void print_colored(const std::string& text, WORD color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
    std::cout << text;
    SetConsoleTextAttribute(hConsole, 7); // Reset to default
}

void blinking_title(const std::string& title, int blink_times = 6, int delay_ms = 300) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    COORD pos;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    pos.X = 12; // vị trí căn giữa tương đối cho dòng tiêu đề
    pos.Y = csbi.dwCursorPosition.Y;

    for (int i = 0; i < blink_times; ++i) {
        // Di chuyển con trỏ đến vị trí cần in
        SetConsoleCursorPosition(hConsole, pos);
        if (i % 2 == 0) {
            print_colored(title, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Vàng
        } else {
            print_colored("          ", 0); // In khoảng trắng để "ẩn" chữ
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }

    // In lại tiêu đề lần cuối
    SetConsoleCursorPosition(hConsole, pos);
    print_colored(title, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::cout << std::endl;
}

void animated_print(const std::string& line, WORD color, int delay_ms = 30) {
    for (char c : line) {
        print_colored(std::string(1, c), color);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }
    std::cout << "\n";
}


void typewriter_effect(const std::string& message, WORD color = 7, int delay_ms = 30) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
    for (char c : message) {
        std::cout << c << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }
    SetConsoleTextAttribute(hConsole, 7); // reset color
}

HANDLE g_currentProcess = NULL;
static DWORD g_originalConsoleMode = 0;

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType) {
    if ((dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_BREAK_EVENT) && g_currentProcess) {
        // Ngắt tiến trình foreground
        TerminateProcess(g_currentProcess, 1);
        std::cout << "\n[Shell] Foreground process terminated by Ctrl-C\n";
        g_currentProcess = NULL;

        // Khôi phục trạng thái console
        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
        FlushConsoleInputBuffer(hStdin);
        SetConsoleMode(hStdin, g_originalConsoleMode);

        // Hiển thị lại prompt
        std::cout << "\033[33mmyShell> \033[0m";
        return TRUE; // Xử lý tín hiệu thành công, không thoát shell
    }

    // Nếu không phải tín hiệu liên quan đến Ctrl+C hoặc không có tiến trình foreground
    return FALSE; // Trả về FALSE để shell không bị thoát
}

void printPrompt() {
    std::cout << "\033[33mmyShell> \033[0m"; // 33 = yellow, 0 = res
}

void printWelcomeMessage() {
    DWORD pid = GetCurrentProcessId();
    print_colored("========================================\n", FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN); // Yellow
    blinking_title("Tiny Shell");
    print_colored("========================================\n", FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);

    animated_print("Welcome to Tiny Shell!", FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    animated_print("This is a simple shell to interact with the Windows OS.", FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    
    std::string pid_msg = "PID of Tiny Shell: " + std::to_string(pid);
    animated_print(pid_msg, FOREGROUND_RED | FOREGROUND_INTENSITY);

    animated_print("Type 'help' to see available commands.", FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);

    print_colored("========================================\n", FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
}

void printExitMessage() {
    system("cls");

    typewriter_effect("========================================\n", FOREGROUND_RED | FOREGROUND_INTENSITY);
    typewriter_effect("  Thank you for using ", FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    typewriter_effect("Tiny Shell!\n", FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    typewriter_effect("  Shutting down", FOREGROUND_BLUE | FOREGROUND_INTENSITY);

    // Thêm hiệu ứng dấu chấm động
    for (int i = 0; i < 3; ++i) {
        std::cout << "." << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << std::endl;
    typewriter_effect("  Goodbye and have a great day! \n", FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    typewriter_effect("========================================\n", FOREGROUND_RED | FOREGROUND_INTENSITY);
}

int main() {
    // Bật chế độ xử lý ANSI escape sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    // Save original console mode
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hStdin, &g_originalConsoleMode);
    
    // Install Ctrl-C/Break handler
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

    // Print the welcome message first
    printWelcomeMessage();

    // Chạy monitor trong background
    std::thread monitorThread(MonitorProcessCreation);
    monitorThread.detach(); // không chặn main thread
    

    std::string line;

    while (true) {
        printPrompt();

        // Read input line
        if (!std::getline(std::cin, line)) {
            std::cout << "\n";
            if (std::cin.eof()) break;
            // Restore cin state
            std::cin.clear();
            // Clear and restore console
            FlushConsoleInputBuffer(hStdin);
            SetConsoleMode(hStdin, g_originalConsoleMode);
            continue;
        }

        if (line.empty()) continue;

        Command cmd = parseCommand(line);
        if (!cmd.argv.empty() && cmd.argv[0] == "exit") {
            if (monitor_running) { // Check if monitor_running is accessible
                monitor_running = false;
            }
            if (monitorThread.joinable()) {
                monitorThread.join(); // Wait for monitor thread to finish
            }
            printExitMessage(); // No longer printing message on exit here
            break;
        }
        // std::cout << cmd.argv.empty();

        // Execute command
        // Set current process handle before launching, so handler knows
        g_currentProcess = NULL; // reset
        executeCommand(cmd);

        // If foreground, executeCommand should set g_currentProcess to child handle
        // Wait finishes, so clear it
        g_currentProcess = NULL;

        // After execution, flush and restore console
        FlushConsoleInputBuffer(hStdin);
        SetConsoleMode(hStdin, g_originalConsoleMode);
    }

    return 0;
}
