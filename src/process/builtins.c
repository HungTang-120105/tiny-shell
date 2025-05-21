#include<stdio.h>
#include<string.h>
#include "../include/builtins.h"

// có hàm mới thì thêm vào đây
//extern int cd(char **args);
//extern int pwd(char **args);

// Kiểu hàm built-in
typedef int (*builtin_func)(char **args);

// Cấu trúc ánh xạ tên command → hàm xử lý
typedef struct {
    const char *name;
    builtin_func func;
} builtin_entry;

// thêm ánh xạ cho hàm mới ở đây
static builtin_entry builtins[] = {
    { "cd", cd},
    { "pwd", pwd}
};

static int num_builtins(){
    return sizeof(builtins)/sizeof(builtin_entry);
}

builtin_func get_builtin_func(char* cmd){
    if (cmd == NULL) return NULL;
    for (int i=0; i<num_builtins(); ++i){
        if (strcmp(cmd, builtins[i].name) == 0){
            return builtins[i].func;
        }
    }
    return NULL;
}

int handle_bultin(char **args){
    builtin_func f = get_builtin_func(args[0]);
    if (f){
        return f(args+1);
    }
    return 0;
}
