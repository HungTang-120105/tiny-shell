#pragma once
#include "parser.h"

// Execute a parsed command: handle built-in vs external, redirect, background
void executeCommand(const Command &cmd);