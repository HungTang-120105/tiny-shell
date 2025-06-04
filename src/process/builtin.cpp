#include "../include/builtin.h"
#include "../include/process_manager.h"
#include "../include/execute.h"
#include "../include/animations.h"
#include "../include/snake_game.h"
#include "../include/system_utils.h"
#include "../include/history.h"
#include "../include/calculator.h"
#include "../include/converter.h"
#include "../include/location_service.h"
#include "../include/weather_service.h"
#include "../include/minesweeper_game.h"
#include "../include/hangman_game.h"
#include "../include/cat_animation.h"
#include <iostream>
#include <cstdlib>
#include <direct.h>
#include <windows.h>
#include <vector>
#include <string>
#include <thread>
#include <fstream>
#include <iomanip> // For formatting output (setw, fixed, setprecision)
#include <sstream> // For string streams (diskinfo)
#include <numeric> // For std::accumulate if joining args
#include <regex>
#define WIN64_LEAN_AND_MEAN
#define _WIN64_WINNT 0x0A00
// For WMI (cpuinfo)
#include <comdef.h> 
#include <Wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")

void colored(const std::string& text, WORD color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
    std::cout << text;
    SetConsoleTextAttribute(hConsole, 7); // Reset to default
}

void blink(const std::string& title, int blink_times = 6, int delay_ms = 300) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    COORD pos;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    pos.X = 12; // vị trí căn giữa tương đối cho dòng tiêu đề
    pos.Y = csbi.dwCursorPosition.Y;

    for (int i = 0; i < blink_times; ++i) {
        // Di chuyển con trỏ đến vị trí cần in
        SetConsoleCursorPosition(hConsole, pos);
        if (i % 2 == 0) {
            colored(title, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Vàng
        } else {
            colored("                            ", 0); // In khoảng trắng để "ẩn" chữ
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }

    // In lại tiêu đề lần cuối
    SetConsoleCursorPosition(hConsole, pos);
    colored(title, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::cout << std::endl;
}

void builtin_fireworks(const std::vector<std::string>& args) {
    animateFirework(); 
}

void builtin_snake(const std::vector<std::string>& args) {
    playSnakeGame();
}

void builtin_history(const std::vector<std::string>& args) {
    const auto& history = get_command_history();
    if (history.empty()) {
        std::cout << "No commands in history." << std::endl;
    } else {
        for (size_t i = 0; i < history.size(); ++i) {
            std::cout << std::setw(4) << i + 1 << "  " << history[i] << std::endl;
        }
    }
}

void builtin_clear_history(const std::vector<std::string>& args) {
    clear_all_command_history();
    // Message is printed by clear_all_command_history() in main.cpp
}

void builtin_calculate(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << "Usage: calculate <expression>" << std::endl;
        std::cerr << "Example: calculate \"3 + 4 * (2 - 1)\"" << std::endl;
        return;
    }

    // Concatenate all parts of the expression after "calculate"
    std::string expression_str;
    for (size_t i = 1; i < args.size(); ++i) {
        if (i > 1) expression_str += " "; // Add space between arguments if they were split by shell parser
        expression_str += args[i];
    }
    // A common way to input expressions with spaces is to quote them:
    // calculate "5 * (10 + 2)"
    // If the shell parser already gives the full quoted string as one arg (args[1]),
    // then the loop is not strictly necessary but harmless. 
    // If args are already "calculate", "5", "*", "(10", "+", "2)", this joins them.
    // Let's assume for now args[1] might be the whole expression if quoted,
    // or it might be split if not quoted. The safest is to join them all.
    // However, the parser likely handles quotes and gives "5 * (10+2)" as args[1].
    // Let's simplify and assume the expression is args[1], if quoted, or needs careful input.
    // For robustness, joining all subsequent args is better.

    if (expression_str.empty()) {
         std::cerr << "Expression cannot be empty." << std::endl;
         return;
    }

    try {
        double result = TinyCalculator::calculate_expression(expression_str);
        std::cout << std::fixed << std::setprecision(6); // Adjust precision as needed
        std::cout << expression_str << " = " << result << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void builtin_convert(const std::vector<std::string>& args) {
    if (args.size() != 6 || args[2] != "from" || args[4] != "to") {
        std::cerr << "Usage: convert <value> from <base_from> to <base_to>" << std::endl;
        std::cerr << "Example: convert 101 from 2 to 10" << std::endl;
        std::cerr << "Example: convert FF from 16 to 10" << std::endl;
        std::cerr << "Note: Bases must be between 2 and 36." << std::endl;
        return;
    }

    const std::string& value_str = args[1];
    int base_from = 0;
    int base_to = 0;

    try {
        size_t pos_from, pos_to;
        base_from = std::stoi(args[3], &pos_from);
        base_to = std::stoi(args[5], &pos_to);

        if (pos_from != args[3].length() || pos_to != args[5].length()) {
            throw std::invalid_argument("Bases must be valid integers.");
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: Invalid base number provided. " << e.what() << std::endl;
        std::cerr << "Bases must be integers between 2 and 36." << std::endl;
        return;
    }

    try {
        std::string result = BaseConverter::convert_base(value_str, base_from, base_to);
        std::cout << value_str << " (base " << base_from << ") = " 
                  << result << " (base " << base_to << ")" << std::endl;
    } catch (const std::exception& e) { // Catches std::invalid_argument or std::out_of_range
        std::cerr << "Conversion Error: " << e.what() << std::endl;
    }
}

void builtin_location(const std::vector<std::string>& args) {
    // Potentially add argument parsing here if you want `location <city>` in the future
    // For now, it just gets the current (placeholder) location.
    try {
        std::string location_info = LocationService::get_current_location_placeholder();
        std::cout << location_info << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error getting location: " << e.what() << std::endl;
    }
}

void builtin_weather(const std::vector<std::string>& args) {
    std::string query_location_for_api;
    std::string display_location_for_user; // For the "Fetching weather for..." message

    if (args.size() < 2) {
        std::cout << "No city specified. Attempting to use current location..." << std::endl;
        display_location_for_user = "current location";
        try {
            std::string full_location_info = LocationService::get_current_location_placeholder();
            // Attempt to parse City from the detailed string
            // Example: "  City: Hanoi\n"
            size_t city_label_pos = full_location_info.find("City: ");
            if (city_label_pos != std::string::npos) {
                size_t city_start_pos = city_label_pos + 6; // Length of "City: "
                size_t city_end_pos = full_location_info.find('\n', city_start_pos);
                if (city_end_pos != std::string::npos) {
                    query_location_for_api = full_location_info.substr(city_start_pos, city_end_pos - city_start_pos);
                    // Trim trailing carriage return if present from Windows newlines \r\n
                    if (!query_location_for_api.empty() && query_location_for_api.back() == '\r') {
                        query_location_for_api.pop_back();
                    }
                }
            }

            if (!query_location_for_api.empty()) {
                 std::cout << "(Determined city: " << query_location_for_api << " from location service)" << std::endl;
                 display_location_for_user = query_location_for_api; // Update for user message
            } else {
                std::cout << "(Could not automatically determine city. Defaulting to Hanoi for weather query.)" << std::endl;
                query_location_for_api = "Hanoi"; 
                display_location_for_user = "Hanoi (default)";
            }
            // If LocationService itself returned an error, print it and fallback.
            if (full_location_info.rfind("Error:", 0) == 0 || full_location_info.rfind("Geolocation API Error:",0) == 0) {
                std::cerr << "Location service info: " << full_location_info << std::endl;
                if (query_location_for_api.empty()) { // If parsing also failed or wasn't attempted due to error string
                    std::cout << "(Location service error. Defaulting to Hanoi for weather query.)" << std::endl;
                    query_location_for_api = "Hanoi";
                    display_location_for_user = "Hanoi (default)";
                }
            }

        } catch (const std::exception& e) {
            std::cerr << "Could not retrieve current location: " << e.what() << std::endl;
            std::cout << "(Defaulting to Hanoi for weather query due to location error.)" << std::endl;
            query_location_for_api = "Hanoi";
            display_location_for_user = "Hanoi (default)";
        }
    } else {
        // Concatenate arguments to form the city name (e.g., weather New York)
        query_location_for_api = args[1];
        for (size_t i = 2; i < args.size(); ++i) {
            query_location_for_api += " " + args[i];
        }
        display_location_for_user = query_location_for_api;
    }

    if (query_location_for_api.empty()) {
        std::cerr << "Error: Location for weather query is empty after processing." << std::endl;
        std::cerr << "Usage: weather <city_name> or weather (uses current location if available)" << std::endl;    
        return;
    }

    try {
        std::cout << "Fetching weather for: '" << display_location_for_user << "'..." << std::endl;
        std::string weather_info = WeatherService::get_weather_placeholder(query_location_for_api);
        std::cout << weather_info << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error getting weather: " << e.what() << std::endl;
    }
}

void builtin_mines(const std::vector<std::string>& args) {
    // Args could be used in the future to set difficulty (e.g., "mines easy", "mines 15 15 30")
    // For now, play_minesweeper_game uses default settings.
    std::cout << "Launching Minesweeper..." << std::endl;
    Minesweeper::play_minesweeper_game();
    std::cout << "Returned from Minesweeper." << std::endl; 
    // The game itself handles clearing screen and "press any key to continue"
}

void builtin_hangman(const std::vector<std::string>& args) {
    std::cout << "Launching Hangman..." << std::endl;
    Hangman::play_hangman_game();
    std::cout << "Returned from Hangman." << std::endl;
}

void builtin_nyancat(const std::vector<std::string>& args) {
    std::cout << "Starting Nyan Cat animation..." << std::endl;
    CatAnimation::play_nyancat_animation();
    // play_nyancat_animation handles its own screen clearing and messages
    std::cout << "Nyan Cat animation finished." << std::endl; 
}

bool is_builtin(const std::string& cmd) {
    return cmd == "cd" || cmd == "exit" || cmd == "pwd" || cmd == "echo" ||
           cmd == "help" || cmd == "list" || cmd == "kill" || cmd == "stop" ||
           cmd == "resume" || cmd == "date" || cmd == "dir" || cmd == "cls" ||
           cmd == "path" || cmd == "addpath" || cmd == "mlist" || cmd == "pinfo" || 
           cmd == "monitor" || cmd == "stopmonitor" || cmd == "monitor_silent" ||
           cmd == "mkdir" || cmd == "rmdir" || cmd == "touch" || cmd == "rm" || cmd == "cat" || cmd == "REM" ||
           cmd == "fireworks" || cmd == "snake" ||
           cmd == "worktime" || cmd == "cpuinfo" || cmd == "meminfo" || cmd == "diskinfo" ||
           cmd == "history" || cmd == "clear_history" ||
           cmd == "calculate" || cmd == "convert" || 
           cmd == "location" || cmd == "weather" ||
           cmd == "mines" || cmd == "hangman" || cmd == "nyancat";
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
    else if (cmd == "fireworks") builtin_fireworks(args);
    else if (cmd == "snake") builtin_snake(args);
    else if (cmd == "worktime") showWorkTime(args);   // Added worktime
    else if (cmd == "cpuinfo") showCPUInfo(args);    // Added cpuinfo
    else if (cmd == "meminfo") showMemoryInfo(args);    // Added meminfo
    else if (cmd == "diskinfo") showDiskInfo(args);  // Added diskinfo
    else if (cmd == "history") builtin_history(args);         // Added history
    else if (cmd == "clear_history") builtin_clear_history(args); // Added clear_history
    else if (cmd == "calculate") builtin_calculate(args); // Added calculate
    else if (cmd == "convert") builtin_convert(args); // Added convert
    else if (cmd == "location") builtin_location(args); // Added location
    else if (cmd == "weather") builtin_weather(args); // Added weather
    else if (cmd == "mines") builtin_mines(args); // Added Minesweeper
    else if (cmd == "hangman") builtin_hangman(args); // Added Hangman
    else if (cmd == "nyancat") builtin_nyancat(args); // Added nyancat
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
    std::cout << "\n";
    blink("TINY-SHELL INSTRUCTIONS:");
    std::cout << "\n";

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
    std::cout << "help              : Display this help message.\n";
    std::cout << "nyancat           : Display a jumping cat animation.\n";
    std::cout << "fireworks         : Display an ASCII fireworks animation.\n";
    std::cout << "snake             : Play the classic Snake game.\n";
    std::cout << "mines             : Play a game of Minesweeper.\n";
    std::cout << "hangman           : Play a game of Hangman (guess the word).\n";
    std::cout << "\n";

    std::cout << "=== System Information Commands ===\n";
    std::cout << "worktime          : Display system uptime.\n";
    std::cout << "cpuinfo           : Display CPU information.\n";
    std::cout << "meminfo           : Display memory usage information.\n";
    std::cout << "diskinfo          : Display disk usage information for all drives.\n\n";

    std::cout << "=== Command History ===\n";
    std::cout << "history           : Show the command history.\n";
    std::cout << "clear_history     : Clear the command history.\n\n";

    std::cout << "=== Calculator ===\n";
    std::cout << "calculate <expr>  : Evaluate a mathematical expression. Use quotes for expressions with spaces.\n";
    std::cout << "  Operators:      +, -, *, /, % (modulo), ^ (exponentiation), ! (factorial)\n";
    std::cout << "  Functions:      sqrt(x), sin(x), cos(x), tan(x), cot(x)\n";
    std::cout << "                  ln(x), log10(x), log2(x), log8(x), log16(x)\n";
    std::cout << "  Constants:      pi, e\n";
    std::cout << "  Unary +/-:      Supported, e.g., -5, -(2+3)\n";
    std::cout << "  Example:        calculate \"sin(pi/2) + (5! - 100)^2 % 9 - -sqrt(16)\"\n";
    std::cout << "convert <val> from <b1> to <b2>: Convert number <val> from base <b1> to base <b2>.\n";
    std::cout << "  Example:        convert 1A from 16 to 10\n";
    std::cout << "                  convert 255 from 10 to 16\n";
    std::cout << "                  convert 1011 from 2 to 10\n";
    std::cout << "  Bases <b1>, <b2> must be integers between 2 and 36.\n";
    std::cout << "\n";

    std::cout << "=== General Purpose Utilities ===\n";
    std::cout << "location           : Display the current geographical location (placeholder).\n";
    std::cout << "weather <city_name>: Displays current weather. \n";
    std::cout << "\n";

    std::cout << "=== Notes ===\n";
    std::cout << "- Commands like 'kill', 'stop', and 'resume' require the PID of the target process.\n";
    std::cout << "- Use 'list' to view all system processes and 'mlist' to view processes managed by this shell.\n";
    std::cout << "- The 'monitor_silent' command suppresses output while monitoring processes.\n";
    std::cout << "- Use 'addpath' to temporarily modify the PATH variable for this shell session.\n";

    std::cout << "\n--- Process Management ---\n";
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



