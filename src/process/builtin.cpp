#include "../include/builtin.h"
#include "../include/process_manager.h"
#include "../include/execute.h"
#include <iostream>
#include <cstdlib>
#include <direct.h>
#include <windows.h>
#include <vector>
#include <string>
#include <thread>
#include <fstream>



bool is_builtin(const std::string& cmd) {
    return cmd == "cd" || cmd == "exit" || cmd == "pwd" || cmd == "echo" ||
           cmd == "help" || cmd == "list" || cmd == "kill" || cmd == "stop" ||
           cmd == "resume" || cmd == "date" || cmd == "dir" || cmd == "cls" ||
           cmd == "path" || cmd == "addpath" || cmd == "mlist" || cmd == "pinfo" || 
           cmd == "monitor" || cmd == "stopmonitor" || cmd == "monitor_silent"
           || cmd == "mkdir" || cmd == "rmdir" || cmd == "touch" || cmd == "rm" || cmd == "cat" || cmd == "REM";
}

void run_builtin(const std::vector<std::string>& args) {
    if (args.empty()) return;
    const std::string& cmd = args[0];
    if (cmd == "cd") builtin_cd(args);
    else if (cmd == "exit") builtin_exit(args);
    else if (cmd == "pwd") builtin_pwd(args);
    else if (cmd == "echo") builtin_echo(args);
    else if (cmd == "help") builtin_help(args);
    else if (cmd == "list") builtin_list(args);
    else if (cmd == "kill") builtin_kill(args);
    else if (cmd == "stop") builtin_stop(args);
    else if (cmd == "resume") builtin_resume(args);
    else if (cmd == "date") builtin_date(args);
    else if (cmd == "dir") builtin_dir(args);
    else if (cmd == "path") builtin_path(args);
    else if (cmd == "addpath") builtin_addpath(args);
    else if (cmd == "mlist") builtin_mlist(args);
    else if (cmd == "pinfo") builtin_pinfo(args);
    else if (cmd == "monitor") builtin_monitor(args);
    else if (cmd == "stopmonitor") builtin_stopmonitor(args);
    else if (cmd == "monitor_silent") builtin_monitor_silent(args);
    else if (cmd == "mkdir") builtin_mkdir(args); 
    else if (cmd == "rmdir") builtin_rmdir(args); 
    else if (cmd == "touch") builtin_touch(args); 
    else if (cmd == "rm") builtin_rm(args);      
    else if (cmd == "cat") builtin_cat(args);     
    else if (cmd == "REM") builtin_rem(args);
    else if (cmd == "cls") builtin_cls(args); 
    else std::cerr << "Unknown command: " << cmd << "\n";

}

void builtin_cd(const std::vector<std::string>& args) {
    if (args.size() == 1) {
        char cwd[MAX_PATH];
        if (_getcwd(cwd, sizeof(cwd))) {
            std::cout << cwd << "\n";
        } else {
            perror("cd");
        }
        return;
    }

    std::string target = args[1];

    if (target == "\\") {
        char drive[MAX_PATH];
        if (_getcwd(drive, sizeof(drive))) {
            drive[2] = '\0'; 
            target = std::string(drive) + "\\";
        }
    }

    if (target.size() == 2 && std::isalpha(target[0]) && target[1] == ':') {
        std::cout << "Note: Changing drives like 'cd D:' has no effect in this shell.\n";
        return;
    }

    if (_chdir(target.c_str()) != 0) {
        perror("cd");
    } else {
        char cwd[MAX_PATH];
        if (_getcwd(cwd, sizeof(cwd))) {
            std::cout << cwd << "\n";
        }
    }
}

void builtin_exit(const std::vector<std::string>& args) {
    std::exit(0);
}

void builtin_pwd(const std::vector<std::string>& args) {
    char cwd[1024];
    if (_getcwd(cwd, sizeof(cwd))) {
        std::cout << cwd << std::endl;
    } else {
        perror("pwd");
    }
}

void builtin_echo(const std::vector<std::string>& args) {
    // Bắt đầu từ i=1 để bỏ qua "echo"
    for (size_t i = 1; i < args.size(); ++i) {

        // Nếu muốn bỏ dấu "": 
        std::string out = args[i];
        if (!out.empty() && out.front() == '"' && out.back() == '"') {
            out = out.substr(1, out.size() - 2);
        }

        std::cout << out;
        if (i + 1 < args.size()) {
            std::cout << " ";
        }
    }
    std::cout << std::endl;
}

void builtin_help(const std::vector<std::string>& args) {
    std::cout << "Tiny-Shell Help:\n\n";

    std::cout << "=== File and Directory Commands ===\n";
    std::cout << "cd <dir>          : Change the current directory to <dir>.\n";
    std::cout << "pwd               : Print the current working directory.\n";
    std::cout << "dir               : List the contents of the current directory.\n";
    std::cout << "mkdir <dir>       : Create a new directory.\n";
    std::cout << "rmdir <dir>       : Remove an empty directory.\n";
    std::cout << "touch <file>      : Create or update a file.\n";
    std::cout << "rm <file>         : Remove a file.\n";
    std::cout << "cat <file>        : Display the contents of a file.\n";
    std::cout << "path              : Display the current PATH environment variable.\n";
    std::cout << "addpath <dir>     : Add <dir> to the PATH environment variable.\n\n";

    std::cout << "=== Process Management Commands ===\n";
    std::cout << "list              : List all processes currently running on the system.\n";
    std::cout << "mlist             : List all processes managed by this shell.\n";
    std::cout << "pinfo <pid>       : Display detailed information about the process with PID <pid>.\n";
    std::cout << "kill <pid>        : Terminate the process with PID <pid>.\n";
    std::cout << "stop <pid>        : Suspend the process with PID <pid>.\n";
    std::cout << "resume <pid>      : Resume the process with PID <pid>.\n\n";

    std::cout << "=== Monitoring Commands ===\n";
    std::cout << "monitor           : Start monitoring process creation and deletion (normal mode).\n";
    std::cout << "monitor_silent    : Start monitoring process creation and deletion (silent mode).\n";
    std::cout << "stopmonitor       : Stop the process monitoring.\n\n";

    std::cout << "=== Shell Utility Commands ===\n";
    std::cout << "cls               : Clear the console screen.\n"; 
    std::cout << "echo <text>       : Print <text> to the console.\n";
    std::cout << "date              : Display the current date and time.\n";
    std::cout << "exit              : Exit the shell.\n";
    std::cout << "help              : Display this help message.\n\n";

    std::cout << "=== Notes ===\n";
    std::cout << "- Commands like 'kill', 'stop', and 'resume' require the PID of the target process.\n";
    std::cout << "- Use 'list' to view all system processes and 'mlist' to view processes managed by this shell.\n";
    std::cout << "- The 'monitor_silent' command suppresses output while monitoring processes.\n";
    std::cout << "- Use 'addpath' to temporarily modify the PATH variable for this shell session.\n";
}

void builtin_list(const std::vector<std::string>& args) {
    list_processes();
}

void builtin_kill(const std::vector<std::string>& args) {
    if (args.size() < 2) { std::cerr << "kill: missing pid\n"; return; }
    DWORD pid = std::stoul(args[1]);
    if (!kill_process(pid)) std::cerr << "kill: failed on pid " << pid << "\n";
}

void builtin_stop(const std::vector<std::string>& args) {
    if (args.size() < 2) { std::cerr << "stop: missing pid\n"; return; }
    DWORD pid = std::stoul(args[1]);
    if (!stop_process(pid)) std::cerr << "stop: failed on pid " << pid << "\n";
}

void builtin_resume(const std::vector<std::string>& args) {
    if (args.size() < 2) { std::cerr << "resume: missing pid\n"; return; }
    DWORD pid = std::stoul(args[1]);
    if (!resume_process(pid)) std::cerr << "resume: failed on pid " << pid << "\n";
}

void builtin_date(const std::vector<std::string>& args) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    std::cout << st.wDay << "/" << st.wMonth << "/" << st.wYear
              << " " << st.wHour << ":" << st.wMinute
              << ":" << st.wSecond << std::endl;
}

void builtin_dir(const std::vector<std::string>& args) {
    std::string cmd = "dir";
    system(cmd.c_str());
}

void builtin_path(const std::vector<std::string>& args) {
    char *p = std::getenv("PATH");
    if (p) std::cout << p << std::endl;
}

void builtin_addpath(const std::vector<std::string>& args) {
    if (args.size() < 2) { std::cerr << "addpath: missing dir\n"; return; }
    std::string p = std::getenv("PATH");
    p += ";" + args[1];
    _putenv_s("PATH", p.c_str());
}

void builtin_mkdir(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << "mkdir: missing directory name\n";
        return;
    }

    const std::string& dirName = args[1];
    if (_mkdir(dirName.c_str()) == 0) {
        std::cout << "Directory created: " << dirName << "\n";
    } else {
        perror("mkdir");
    }
}

void builtin_rmdir(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << "rmdir: missing directory name\n";
        return;
    }

    const std::string& dirName = args[1];
    if (_rmdir(dirName.c_str()) == 0) {
        std::cout << "Directory removed: " << dirName << "\n";
    } else {
        perror("rmdir");
    }
}

void builtin_touch(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << "touch: missing file name\n";
        return;
    }

    const std::string& fileName = args[1];
    std::ofstream file(fileName);
    if (file.is_open()) {
        std::cout << "File created or updated: " << fileName << "\n";
        file.close();
    } else {
        std::cerr << "touch: unable to create file " << fileName << "\n";
    }
}

void builtin_rm(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << "rm: missing file name\n";
        return;
    }

    const std::string& fileName = args[1];
    std::wstring wideFileName(fileName.begin(), fileName.end()); 

    if (DeleteFileW(wideFileName.c_str())) {
        std::cout << "File removed: " << fileName << "\n";
    } else {
        DWORD error = GetLastError();
        std::cerr << "rm: failed to remove file " << fileName << " (Error code: " << error << ")\n";
    }
}

void builtin_cat(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << "cat: missing file name\n";
        return;
    }

    const std::string& fileName = args[1];
    std::ifstream file(fileName);
    if (!file.is_open()) {
        std::cerr << "cat: unable to open file " << fileName << "\n";
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::cout << line << "\n";
    }
    file.close();
}

void builtin_cls(const std::vector<std::string>& args) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        std::cerr << "cls: Unable to get console handle\n";
        return;
    }

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        std::cerr << "cls: Unable to get console buffer info\n";
        return;
    }

    DWORD consoleSize = csbi.dwSize.X * csbi.dwSize.Y;
    COORD topLeft = {0, 0};
    DWORD charsWritten;

    FillConsoleOutputCharacter(hConsole, ' ', consoleSize, topLeft, &charsWritten);
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, consoleSize, topLeft, &charsWritten);

    SetConsoleCursorPosition(hConsole, topLeft);
}

void builtin_rem(const std::vector<std::string>& args) {
}

void builtin_mlist(const std::vector<std::string>& args) {
    print_managed_processes();
}

void builtin_pinfo(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: pinfo <pid>\n";
        return;
    }
    DWORD pid = std::stoul(args[1]);
    print_process_info(pid);
}


void builtin_monitor(const std::vector<std::string>& args) {
    if (monitor_running) {
        std::cerr << "[WARNING] Monitor is already running.\n";
        return;
    }
    monitor_running = true; 
    monitor_silent = false; 
    std::cout << "[INFO] Starting process creation monitor...\n";
    std::thread monitorThread(MonitorProcessCreation);
    monitorThread.detach(); 
}

void builtin_stopmonitor(const std::vector<std::string>& args) {
    monitor_running = false;
    monitor_silent = false; 
    std::cout << "Stopping process monitor...\n";
}

void builtin_monitor_silent(const std::vector<std::string>& args) {
    if (monitor_running) {
        std::cerr << "[WARNING] Monitor is already running.\n";
        return;
    }
    monitor_running = true; 
    monitor_silent = true;  
    std::cout << "[INFO] Starting process creation monitor in silent mode...\n";
    std::thread monitorThread(MonitorProcessCreation);
    monitorThread.detach(); 
}