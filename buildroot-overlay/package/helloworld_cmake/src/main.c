#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h> // for backtrace
#include <signal.h>   // for signal handling
#include "list_file.h"

#define STACK_SIZE 100

// 打印调用栈
void print_stack_trace() {
    void *buffer[STACK_SIZE];
    int size = backtrace(buffer, STACK_SIZE);
    char **strings = backtrace_symbols(buffer, size);
    if (strings == NULL) {
        perror("backtrace_symbols");
        exit(EXIT_FAILURE);
    }

    printf("Stack trace:\n");
    for (int i = 0; i < size; i++) {
        printf("%s\n", strings[i]);
    }

    free(strings);
}

// 信号处理函数
void signal_handler(int sig) {
    printf("Received signal %d\n", sig);
    print_stack_trace();
    exit(1);
}

void func_c() {
    // 触发段错误
    int *p = NULL;
    *p = 42;
}

void func_b() {
    func_c();
}

void func_a() {
    func_b();
}

int main(int argc, char *argv[]) {
    printf("hellowold cmake\n");
    // 注册信号处理函数
    signal(SIGSEGV, signal_handler); // 捕获段错误
    signal(SIGABRT, signal_handler); // 捕获 abort
    list_files(argc, argv);

    func_a();

    printf("hellowold cmake end\n");

    return 0;
}
