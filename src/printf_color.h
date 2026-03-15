#pragma once
#include <stdio.h>

#define printf_red(fmt, ...)     printf("\033[1;31m" fmt "\033[0m", ##__VA_ARGS__)
#define printf_green(fmt, ...)   printf("\033[1;32m" fmt "\033[0m", ##__VA_ARGS__)
#define printf_yellow(fmt, ...)  printf("\033[1;33m" fmt "\033[0m", ##__VA_ARGS__)
#define printf_blue(fmt, ...)    printf("\033[1;34m" fmt "\033[0m", ##__VA_ARGS__)
#define printf_magenta(fmt, ...) printf("\033[1;35m" fmt "\033[0m", ##__VA_ARGS__)
#define printf_cyan(fmt, ...)    printf("\033[1;36m" fmt "\033[0m", ##__VA_ARGS__)
#define printf_white(fmt, ...)   printf("\033[1;37m" fmt "\033[0m", ##__VA_ARGS__)
