#include <stdio.h>
#include <string.h>
#include "parser.h"

void parse_and_execute(const char *input){
    char command[256];
    strcpy(command, input);
    command[strcspn(command, "\n")] = 0;

    printf(">> Command: %s\n", command);
}

