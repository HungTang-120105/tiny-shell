#include "../include/builtin.h"
#include "../include/process_manager.h"
#include <iostream>
#include <cstdlib>
#include <direct.h>
#include <windows.h>
#include <vector>
#include <string>



bool is_builtin(const std::string& cmd) {
    return cmd == "cd" || cmd == "exit" || cmd == "pwd" || cmd == "echo" ||
           cmd == "help" || cmd == "list" || cmd == "kill" || cmd == "stop" ||
           cmd == "resume" || cmd == "date" || cmd == "dir" ||
           cmd == "path" || cmd == "addpath" || cmd == "mlist" || cmd == "pinfo" || 
           cmd == "monitor" || cmd == "stopmonitor";
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

}

void builtin_cd(const std::vector<std::string>& args) {
    if (args.size() == 1) {
        // Hiển thị thư mục hiện tại
        char cwd[MAX_PATH];
        if (_getcwd(cwd, sizeof(cwd))) {
            std::cout << cwd << "\n";
        } else {
            perror("cd");
        }
        return;
    }

    std::string target = args[1];

    // Xử lý đặc biệt: cd 
    if (target == "\\") {
        char drive[MAX_PATH];
        if (_getcwd(drive, sizeof(drive))) {
            drive[2] = '\0'; // Cắt thành "C:"
            target = std::string(drive) + "\\";
        }
    }

    // Xử lý đặc biệt: cd D: (không chuyển thư mục nếu không phải foreground shell)
    if (target.size() == 2 && std::isalpha(target[0]) && target[1] == ':') {
        // Với shell tự viết, không nên can thiệp chuyển ổ đĩa toàn hệ thống
        // Ta có thể in ra cảnh báo:
        std::cout << "Note: Changing drives like 'cd D:' has no effect in this shell.\n";
        return;
    }

    // Dùng _chdir để chuyển thư mục
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
    for (size_t i = 1; i < args.size(); ++i) {
        std::cout << args[i] << (i + 1 < args.size() ? " " : "");
    }
    std::cout << std::endl;
}

void builtin_help(const std::vector<std::string>& args) {
    std::cout << "Built-in commands:\n"
              << "cd <dir>\nexit\npwd\necho\nhelp\n"
              << "list\nkill <pid>\nstop <pid>\nresume <pid>\n"
              << "date\ndir\npath\naddpath <dir>\n";
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


//process_manager.h
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
    std::cout << "[INFO] Starting process creation monitor...\n";
    MonitorProcessCreation(); // Gọi hàm giám sát tiến trình
}

void builtin_stopmonitor(const std::vector<std::string>& args) {
    monitor_running = false;
    std::cout << "Stopping process monitor...\n";
}