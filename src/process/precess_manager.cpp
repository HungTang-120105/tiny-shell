#include "../include/process_manager.h"
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>

static std::vector<ProcessInfo> process_list;

void list_processes() {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create snapshot.\n";
        return;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe)) {
        std::cout << "PID\tName\n";
        do {
            std::wcout << pe.th32ProcessID << "\t" << pe.szExeFile << "\n";
        } while (Process32Next(hSnapshot, &pe));
    } else {
        std::cerr << "Failed to retrieve process list.\n";
    }

    CloseHandle(hSnapshot);
}

int find_process_index(DWORD pid) {
    for (size_t i = 0; i < process_list.size(); ++i) {
        if (process_list[i].pid == pid) return static_cast<int>(i);
    }
    return -1;
}

bool stop_process(DWORD pid) {
    if (pid == GetCurrentProcessId()) {
        std::cerr << "Cannot suspend the shell itself!\n";
        return false;
    }

    HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, pid);
    if (!hProcess) {
        std::cerr << "Failed to open process.\n";
        return false;
    }

    typedef LONG (NTAPI *NtSuspendProcess)(HANDLE);
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (!ntdll) return false;
    NtSuspendProcess suspend = (NtSuspendProcess)GetProcAddress(ntdll, "NtSuspendProcess");
    if (!suspend) return false;

    suspend(hProcess);
    CloseHandle(hProcess);

    int index = find_process_index(pid);
    if (index != -1) {
        process_list[index].status = "Suspended";
    }

    return true;
}

bool resume_process(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, pid);
    if (!hProcess) {
        std::cerr << "Failed to open process.\n";
        return false;
    }

    typedef LONG (NTAPI *NtResumeProcess)(HANDLE);
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (!ntdll) return false;
    NtResumeProcess resume = (NtResumeProcess)GetProcAddress(ntdll, "NtResumeProcess");
    if (!resume) return false;

    resume(hProcess);
    CloseHandle(hProcess);

    int index = find_process_index(pid);
    if (index != -1) {
        process_list[index].status = "Running";
    }

    return true;
}

bool kill_process(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (!hProcess) {
        std::cerr << "Failed to open process.\n";
        return false;
    }

    BOOL result = TerminateProcess(hProcess, 0);
    CloseHandle(hProcess);

    if (result) {
        int index = find_process_index(pid);
        if (index != -1) {
            process_list.erase(process_list.begin() + index);
        }
    }

    return result;
}

void addProcess(DWORD pid, const std::wstring &cmdline, HANDLE hProcess, bool is_background) {
    int len = WideCharToMultiByte(CP_UTF8, 0, cmdline.c_str(), -1, NULL, 0, NULL, NULL);
    std::string name(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, cmdline.c_str(), -1, &name[0], len, NULL, NULL);
    process_list.push_back({ pid, name, "Running", is_background });
}

void print_managed_processes() {
    std::cout << "PID\tStatus\t\tType\t\tName\n";
    for (const auto& p : process_list) {
        std::cout << p.pid << "\t" << p.status << "\t"
                  << (p.is_background ? "Background" : "Foreground") << "\t"
                  << p.name << "\n";
    }
}

void print_process_info(DWORD pid) {
    int index = find_process_index(pid);
    if (index == -1) {
        std::cout << "Process with PID " << pid << " not found in managed list.\n";
        return;
    }
    const auto& p = process_list[index];
    std::cout << "PID:        " << p.pid << "\n"
              << "Status:     " << p.status << "\n"
              << "Type:       " << (p.is_background ? "Background" : "Foreground") << "\n"
              << "Name:       " << p.name << "\n";
}
