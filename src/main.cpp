#include <iostream>
#include <string>
#include <windows.h>  

#include "include/parser.h"
#include "include/execute.h"
#include <thread> 
#include "include/process_manager.h"

static HANDLE g_currentProcess = NULL;
static DWORD g_originalConsoleMode = 0;

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType) {
    if ((dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_BREAK_EVENT) && g_currentProcess) {
        TerminateProcess(g_currentProcess, 1);
        std::cout << "\n[Shell] Foreground process terminated by Ctrl-C\n";
        g_currentProcess = NULL;
        // flush input buffer and restore console mode
        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
        FlushConsoleInputBuffer(hStdin);
        SetConsoleMode(hStdin, g_originalConsoleMode);
        std::cout << "myShell> ";
        return TRUE;
    }
    return FALSE;
}

void printPrompt() {
    std::cout << "myShell> ";
}

int main() {
    // Save original console mode
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hStdin, &g_originalConsoleMode);
    
    // Install Ctrl-C/Break handler
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
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
        if (!cmd.argv.empty() && cmd.argv[0] == "exit") break;
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
