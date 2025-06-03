#pragma once
#include <string>
#include <vector>
#include <windows.h>

extern bool monitor_running;
extern bool monitor_silent; 
struct ProcessInfo {
    DWORD pid;
    std::string name;
    std::string status;
    bool is_background; 
};

void list_processes();

bool stop_process(DWORD pid);

bool resume_process(DWORD pid);

bool kill_process(DWORD pid);

void addProcess(DWORD pid, const std::wstring &cmdline, HANDLE hProcess, bool is_background);

void print_managed_processes();

void print_process_info(DWORD pid);

void MonitorProcessCreation();