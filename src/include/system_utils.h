#ifndef SYSTEM_UTILS_H
#define SYSTEM_UTILS_H

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
#include <unordered_set>

// Danh sách các lệnh hệ thống hỗ trợ
static const std::unordered_set<std::string> supportedSystemCommands = {
    "worktime",
    "cpuinfo",
    "meminfo",
    "diskinfo"
};

// --- worktime ---
inline void showWorkTime(const std::vector<std::string>& args)
{
    

    DWORDLONG uptime = GetTickCount();
    DWORD seconds = uptime/1000;
    DWORD minutes = (seconds / 60) % 60;
    DWORD hours = (seconds / 3600) % 24;
    DWORD days = seconds / 86400;

    std::cout << "System uptime: "
              << days << " days, "
              << hours << " hours, "
              << minutes << " minutes, "
              << seconds << " seconds"
              << std::endl;
}

// --- cpuinfo ---
inline void showCPUInfo(const std::vector<std::string>& args)
{
    

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    std::cout << "CPU Information:" << std::endl;
    std::cout << "Number of processors: " << sysInfo.dwNumberOfProcessors << std::endl;
    std::cout << "Processor type: " << sysInfo.dwProcessorType << std::endl;
}

// --- meminfo ---
inline void showMemoryInfo(const std::vector<std::string>& args)
{
   

    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);

    std::cout << "Memory Information:" << std::endl;
    std::cout << "Total physical memory: " << statex.ullTotalPhys / (1024 * 1024) << " MB" << std::endl;
    std::cout << "Available physical memory: " << statex.ullAvailPhys / (1024 * 1024) << " MB" << std::endl;
    std::cout << "Total virtual memory: " << statex.ullTotalPageFile / (1024 * 1024) << " MB" << std::endl;
    std::cout << "Available virtual memory: " << statex.ullAvailPageFile / (1024 * 1024) << " MB" << std::endl;
}

// --- diskinfo ---
inline void showDiskInfo(const std::vector<std::string>& args)
{
    

    std::string drive = args[0] + ":\\";

    ULARGE_INTEGER freeBytesAvailableToCaller, totalNumberOfBytes, totalNumberOfFreeBytes;

    std::cout << "Checking disk information for drive " << drive << std::endl;
    

    if (GetDiskFreeSpaceExA(drive.c_str(), &freeBytesAvailableToCaller, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
        std::cout << "Disk Information for drive " << drive << ":" << std::endl;
        std::cout << "Total space: " << totalNumberOfBytes.QuadPart / (1024 * 1024 * 1024) << " GB" << std::endl;
        std::cout << "Free space: " << totalNumberOfFreeBytes.QuadPart / (1024 * 1024 * 1024) << " GB" << std::endl;
    } else {
        std::cerr << "Failed to get disk information: " << GetLastError() << std::endl;
    }
}


#endif // SYSTEM_UTILS_H
