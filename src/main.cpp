#include <stdio.h>
#include <cstring>

#include "../include/parser.h"

int main(){
    char input[256];

    while (1){
        printf("my_shell> ");
        if (fgets(input, sizeof(input), stdin) == NULL){
            printf("nhap lenh");
        }

        if (strncmp(input, "exit", 4) == 0){
            break;
        }

        parse_and_execute(input);
    }
    return 0;
}

