#include "../include/process_manager.h"
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>
#include <mutex>

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

bool stop_process(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, pid);
    if (hProcess == NULL) {
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
    return true;
}

bool resume_process(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, pid);
    if (hProcess == NULL) {
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
    return true;
}

bool kill_process(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == NULL) {
        std::cerr << "Failed to open process.\n";
        return false;
    }

    BOOL result = TerminateProcess(hProcess, 0);
    CloseHandle(hProcess);
    return result;
}


static std::vector<ProcessInfo> process_list;
static std::mutex list_mutex;
void addProcess(DWORD pid, const std::wstring &cmdline, HANDLE hProcess) {
    std::lock_guard<std::mutex> lock(list_mutex);

    // Chuyển cmdline từ wstring -> string để lưu trong ProcessInfo
    int len = WideCharToMultiByte(CP_UTF8, 0, cmdline.c_str(), -1, NULL, 0, NULL, NULL);
    std::string name(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, cmdline.c_str(), -1, &name[0], len, NULL, NULL);

    process_list.push_back({ pid, name, "Running" });
}
