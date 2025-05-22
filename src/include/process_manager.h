#pragma once
#include <string>
#include <vector>
#include <windows.h>

// Struct để lưu thông tin process
struct ProcessInfo {
    DWORD pid;
    std::string name;
    std::string status;
    bool is_background; 
};

// Liệt kê danh sách tiến trình
void list_processes();

// Dừng một tiến trình theo PID
bool stop_process(DWORD pid);

// Tiếp tục một tiến trình theo PID
bool resume_process(DWORD pid);

// Kết thúc một tiến trình theo PID
bool kill_process(DWORD pid);

// Thêm một tiến trình vào danh sách
void addProcess(DWORD pid, const std::wstring &cmdline, HANDLE hProcess, bool is_background);

// in ra danh sách các tiến trình đang quản lý (Myshell)
void print_managed_processes();

// in ra thông tin của một tiến trình theo PID
void print_process_info(DWORD pid);