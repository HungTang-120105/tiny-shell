#include "../include/execute.h"
#include "../include/builtin.h"          // hỗ trợ built-in commands
#include "../include/process_manager.h" // hỗ trợ kill/stop/resume
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

// Convert UTF-8 std::string args to a Windows Unicode command line
static std::wstring joinArgs(const std::vector<std::string>& args) {
    std::wstring result;
    for (const auto& arg : args) {
        if (!result.empty()) result += L' ';
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, arg.c_str(), (int)arg.size(), NULL, 0);
        std::wstring warg(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, arg.c_str(), (int)arg.size(), &warg[0], size_needed);
        if (warg.find(L' ') != std::wstring::npos) {
            result += L'"' + warg + L'"';
        } else {
            result += warg;
        }
    }
    return result;
}

void executeCommand(const Command &cmd) {
    if (cmd.argv.empty()) return;

    // Xử lý built-in commands trước
    if (is_builtin(cmd.argv[0])) {
        std::cout << "[DEBUG] run_builtin called, returning early\n";
        run_builtin(cmd.argv);
        return;
    }

    // Chuẩn bị STARTUPINFO và PROCESS_INFORMATION
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;    // cho phép override stdin/stdout/stderr
    ZeroMemory(&pi, sizeof(pi));

    HANDLE hIn  = INVALID_HANDLE_VALUE;
    HANDLE hOut = INVALID_HANDLE_VALUE;

    // SECURITY_ATTRIBUTES để tạo handle có thể kế thừa
    SECURITY_ATTRIBUTES sa;
    ZeroMemory(&sa, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE; // cho phép con thừa kế handle này

    // --- Input redirection ---
    if (!cmd.infile.empty()) {
        std::wstring winIn;
        int len = MultiByteToWideChar(CP_UTF8, 0, cmd.infile.c_str(), -1, NULL, 0);
        winIn.resize(len);
        MultiByteToWideChar(CP_UTF8, 0, cmd.infile.c_str(), -1, &winIn[0], len);

        hIn = CreateFileW(
            winIn.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            &sa,               // truyền sa để handle có thể kế thừa
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        if (hIn == INVALID_HANDLE_VALUE) {
            std::wcerr << L"Error opening input file: " << winIn << L"\n";
            return;
        }

        // Đảm bảo flag kế thừa
        SetHandleInformation(hIn, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
        si.hStdInput = hIn;
    } else {
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    }

    // --- Output redirection ---
    if (!cmd.outfile.empty()) {
        std::wstring winOut;
        int len = MultiByteToWideChar(CP_UTF8, 0, cmd.outfile.c_str(), -1, NULL, 0);
        winOut.resize(len);
        MultiByteToWideChar(CP_UTF8, 0, cmd.outfile.c_str(), -1, &winOut[0], len);

        hOut = CreateFileW(
            winOut.c_str(),
            GENERIC_WRITE,
            0,
            &sa,               // truyền sa để handle có thể kế thừa
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        if (hOut == INVALID_HANDLE_VALUE) {
            std::wcerr << L"Error opening output file: " << winOut << L"\n";
            if (hIn != INVALID_HANDLE_VALUE) CloseHandle(hIn);
            return;
        }

        // Đảm bảo flag kế thừa
        SetHandleInformation(hOut, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
        si.hStdOutput = hOut;
        si.hStdError  = hOut;
    } else {
        si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);
    }

    // Chuẩn bị command line và khởi tạo tiến trình
    std::wstring cmdline = joinArgs(cmd.argv);
    BOOL ok = CreateProcessW(
        NULL,
        cmdline.data(),   // lưu ý: data() cho phép sửa đổi, phù hợp với CreateProcessW
        NULL,             // process security attributes
        NULL,             // thread security attributes
        TRUE,             // thừa kế handles
        0,                // creation flags
        NULL,             // environment
        NULL,             // current directory
        &si,
        &pi
    );

    // Đóng các handle redirect ở bên parent
    if (hIn  != INVALID_HANDLE_VALUE) CloseHandle(hIn);
    if (hOut != INVALID_HANDLE_VALUE) CloseHandle(hOut);

    if (!ok) {
        std::wcerr << L"Failed to start process: " << cmdline << L"\n";
        return;
    }

    // Quản lý tiến trình (background/foreground)
    if (cmd.background) {
        std::wcout << L"[bg] PID=" << pi.dwProcessId << L"\n";
        addProcess(pi.dwProcessId, cmdline, pi.hProcess, cmd.background);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    } else {
        addProcess(pi.dwProcessId, cmdline, pi.hProcess, cmd.background);
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
}
