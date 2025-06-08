#ifndef __COMMON_BOOTSTARP_H__
#define __COMMON_BOOTSTARP_H__

#define TASK_COMM_LEN       16
#define MAX_FILENAME_LEN    127

/* 和用户交互的数据结构 */
struct event {
    int pid;                            /* 任务pid */
    int ppid;                           /* 任务的父id */
    unsigned int exit_code;             /* 任务退出的值 */
    unsigned long long duration_ns;     /* 任务执行和退出所用的时间 */
    char comm[TASK_COMM_LEN];           /* 存放任务的命令 */
    char filename[MAX_FILENAME_LEN];    /* 存放任务的绝对路径 */
    bool exit_event;                    /* 任务是否执行到退出 */
};

#endif /* __COMMON_BOOTSTARP_H__ */