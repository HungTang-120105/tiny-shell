#include <stdio.h>

#include "parser.h"

int main(){
    char input[MAX_INPUT_LENG];

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


