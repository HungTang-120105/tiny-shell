#pragma once
#include <vector>
#include <string>

// Built-in command handlers
void builtin_cd(const std::vector<std::string>& args);
void builtin_exit(const std::vector<std::string>& args);
void builtin_pwd(const std::vector<std::string>& args);
void builtin_echo(const std::vector<std::string>& args);
void builtin_help(const std::vector<std::string>& args);
void builtin_list(const std::vector<std::string>& args);
void builtin_date(const std::vector<std::string>& args);
void builtin_dir(const std::vector<std::string>& args);
void builtin_path(const std::vector<std::string>& args);
void builtin_addpath(const std::vector<std::string>& args);

// Utility to check and dispatch built-in commands
bool is_builtin(const std::string& cmd);
void run_builtin(const std::vector<std::string>& args);

// process_manager.h
void builtin_pinfo(const std::vector<std::string>& args);
void builtin_kill(const std::vector<std::string>& args);
void builtin_stop(const std::vector<std::string>& args);
void builtin_resume(const std::vector<std::string>& args);
void builtin_mlist(const std::vector<std::string>& args);

void builtin_monitor(const std::vector<std::string>& args);
void builtin_stopmonitor(const std::vector<std::string>& args);
