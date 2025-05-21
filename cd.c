#include<stdio.h>
#include <unistd.h> // chdir
#include "builtins.h"

int pwd(char **args){
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL){
        printf("pwd: %s\n", cwd);
    } else {
        perror("pwd error: ");
        return 0;
    }
    return 1;
}

int cd(char **args){
    //for (int j=0; args[j]!=NULL; ++j) printf("args[%d]= %s \n", j, args[j]);
    if (args[0] == NULL) {
        fprintf(stderr, "cd: expected argument to \"cd\"\n");
        return 1;
    }
    if (chdir(args[0]) != 0) {
        perror("cd error: ");
    }
    return 1;
}

int dir(char **args){

}
