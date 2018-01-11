#include <stdio.h>

#define pr_build_info(fmt, args...)    do { \
    printf(fmt "\n\n\t\033[32mCompile at %s %s \033[0m\n\n", __DATE__, __TIME__, ##args); \
} while(0)

#define pr(fmt, args...)    do { \
    printf("[%s ,%d] : " fmt "\n", __FUNCTION__, __LINE__, ##args); \
} while(0)

#define pr_warn(fmt, args...)    do { \
    printf("[%s ,%d] : \033[33m" fmt "\033[0m\n", __FUNCTION__, __LINE__, ##args); \
} while(0)

#define pr_err(fmt, args...)    do { \
    printf("[%s, %s ,%d] : \033[31mError -> " fmt "\033[0m\n", __FILE__, __FUNCTION__, __LINE__, ##args); \
} while(0)
