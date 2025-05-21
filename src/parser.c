#include <stdio.h>
#include <string.h>

#include "../include/parser.h"
#include "../include/builtins.h"

void parse_and_execute(const char *input){
    char command[256];
    char *args[MAX_ARGS+1];

    strcpy(command, input);
    command[strcspn(command, "\n")] = 0;
    char *token = strtok(command, " ");

    int i = 0;
    while (token != NULL) {
        printf(">> Command: %s\n", token);
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    //for (int j=0; j<=i; ++j) printf("args[%d]= %s \n", j, args[j]);

    //printf(">> Command: %s\n", command);
    if (handle_bultin(args)){
        printf("COMMAND OK\n");
    }
}


