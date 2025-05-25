#include "../include/execute.h"
#include "../include/builtin.h"          // hỗ trợ built-in commands
#include "../include/process_manager.h" // hỗ trợ kill/stop/resume
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream> // Thêm dòng này để sử dụng std::ifstream
#include <fcntl.h>     // for _O_APPEND, _O_BINARY, _O_RDWR
#include <stdio.h>     // for FILE*, stdout, stderr, _fdopen


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
    // --- Prepare STARTUPINFO and handle inheritance for redirection ---
    STARTUPINFOW si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;

    HANDLE hIn  = INVALID_HANDLE_VALUE;
    HANDLE hOut = INVALID_HANDLE_VALUE;

    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    // --- Input redirection ---
    if (!cmd.infile.empty()) {
        std::wstring winIn;
        int len = MultiByteToWideChar(CP_UTF8, 0, cmd.infile.c_str(), -1, nullptr, 0);
        winIn.resize(len);
        MultiByteToWideChar(CP_UTF8, 0, cmd.infile.c_str(), -1, &winIn[0], len);
        hIn = CreateFileW(winIn.c_str(), GENERIC_READ, FILE_SHARE_READ, &sa,
                          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hIn == INVALID_HANDLE_VALUE) {
            std::wcerr << L"Error opening input file: " << winIn << L"\n";
            return;
        }
        SetHandleInformation(hIn, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
        si.hStdInput = hIn;
    } else {
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    }

    // --- Output redirection ---
    if (!cmd.outfile.empty()) {
        std::wstring winOut;
        int len = MultiByteToWideChar(CP_UTF8, 0, cmd.outfile.c_str(), -1, nullptr, 0);
        winOut.resize(len);
        MultiByteToWideChar(CP_UTF8, 0, cmd.outfile.c_str(), -1, &winOut[0], len);

        hOut = CreateFileW(
            winOut.c_str(),
            GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            &sa,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if (hOut == INVALID_HANDLE_VALUE) {
            std::wcerr << L"Error opening output file: " << winOut << L"\n";
            if (hIn != INVALID_HANDLE_VALUE) CloseHandle(hIn);
            return;
        }

        SetHandleInformation(hOut, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
        si.hStdOutput = hOut;
        si.hStdError  = hOut;
    } else {
        si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);
    }

    // --- Handle empty commands (inline script) ---
    if (cmd.argv.empty()) {
        FILE* originalOut = nullptr;
        FILE* redirectedOut = nullptr;
        int fdOut = -1;

        if (hOut != INVALID_HANDLE_VALUE) {
            fdOut = _open_osfhandle((intptr_t)hOut, _O_WRONLY | _O_BINARY);
            if (fdOut != -1) {
                redirectedOut = _fdopen(fdOut, "w");
                if (redirectedOut) {
                    fflush(stdout);
                    originalOut = stdout;
                    *stdout = *redirectedOut;
                    setvbuf(stdout, NULL, _IONBF, 0);
                }
            }
        }

        // --- Run inline script from input file ---
        if (!cmd.infile.empty()) {
            std::ifstream infile(cmd.infile);
            std::string line;
            while (std::getline(infile, line)) {
                if (line.empty()) continue;
                Command subCmd = parseCommand(line);
                if (!subCmd.argv.empty()) executeCommand(subCmd);
            }
        }

        // --- Restore stdout ---
        if (redirectedOut) {
            fflush(stdout);
            if (originalOut) *stdout = *originalOut;
            fclose(redirectedOut);
        } else {
            if (hOut != INVALID_HANDLE_VALUE) CloseHandle(hOut);
        }

        if (hIn != INVALID_HANDLE_VALUE) CloseHandle(hIn);
        return;
    }



    // --- Built-in commands: redirect std::cout/std::cerr using C++ streams ---
    if (is_builtin(cmd.argv[0])) {
        std::streambuf* oldCout = nullptr;
        std::streambuf* oldCerr = nullptr;
        std::ofstream ofs;
        if (!cmd.outfile.empty()) {
            ofs.open(cmd.outfile, std::ios::out | std::ios::app);
            if (ofs.is_open()) {
                oldCout = std::cout.rdbuf(ofs.rdbuf());
                oldCerr = std::cerr.rdbuf(ofs.rdbuf());
            } else {
                std::cerr << "Error: Unable to open output file: " << cmd.outfile << "\n";
            }
        }
        run_builtin(cmd.argv);
        std::cout.flush();
        std::cerr.flush();
        if (oldCout) std::cout.rdbuf(oldCout);
        if (oldCerr) std::cerr.rdbuf(oldCerr);
        if (ofs.is_open()) ofs.close();

        if (hIn  != INVALID_HANDLE_VALUE) CloseHandle(hIn);
        if (hOut != INVALID_HANDLE_VALUE) CloseHandle(hOut);
        return;
    }

    // --- Launch external process ---
    std::wstring cmdline = joinArgs(cmd.argv);
    BOOL ok = CreateProcessW(nullptr, cmdline.data(), nullptr, nullptr,
                              TRUE, 0, nullptr, nullptr, &si, &pi);

    if (hIn  != INVALID_HANDLE_VALUE) CloseHandle(hIn);
    if (hOut != INVALID_HANDLE_VALUE) CloseHandle(hOut);

    if (!ok) {
        std::wcerr << L"Failed to start process: " << cmdline << L"\n";
        return;
    }

    if (cmd.background) {
        std::wcout << L"[bg] PID=" << pi.dwProcessId << L"\n";
        addProcess(pi.dwProcessId, cmdline, pi.hProcess, true);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    } else {
        addProcess(pi.dwProcessId, cmdline, pi.hProcess, false);
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
}

