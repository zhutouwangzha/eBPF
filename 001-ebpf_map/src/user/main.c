#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <bpf/libbpf.h>

#include "common/bootstrap.h"
#include "user/load_bpf.h"
#include "user/parse_arg.h"

/**
 * 标识是否按下了 ctrl + c
 */
static volatile bool exiting = false;

/**
 * ctrl + c 的回调函数
 */
static void sig_handler(int sig)
{
    exiting = true;
}

/**
 * 消息队列的回调函数
 */
static int handle_event(void *ctx, void *data, size_t data_sz)
{
    const struct event *e = data;
    struct tm *tm;
    char ts[32];
    time_t t;

    time(&t);
    tm = localtime(&t);
    strftime(ts, sizeof(ts), "%H:%M:%S", tm);

    if (e->exit_event) {
        printf("%-8s %-5s %-16s %-7d %-7d [%u]", ts, "EXIT", e->comm,
            e->pid, e->ppid, e->exit_code);
        
        if (e->duration_ns) {
            printf(" (%llums)", e->duration_ns / 1000000);
        }

        printf("\n");
    } else {
        printf("%-8s %-5s %-16s %-7d %-7d %s\n", ts, "EXEC", e->comm,
                e->pid, e->ppid, e->filename);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    struct ring_buffer *rb = NULL;
    struct kernel_bpf *skel;
    int err = -1;

    /* 处理输出参数，实际会调用到 src/user/parse_arg.c 中的 parse_arg 函数 */
    err = argp_parse(&arg_assemble, argc, argv, 0, NULL, NULL);
    if (err) {
        return err;
    }

    /* 为 ctrl + c 注册回调函数 */
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    /* 加载bpf到内核中 */
    skel = load_bpf_to_kernel();
    if (!skel) {
        fprintf(stderr, "Failed to load_bpf_to_kernel\n");
        goto cleanup_skel;
    }

    /* 创建循环消息队列 */
    rb = ring_buffer__new(bpf_map__fd(skel->maps.rb), handle_event, NULL, NULL);
    if (!rb) {
        fprintf(stderr, "Failed to create ring buffer\n");
        goto cleanup;
    }

    printf("%-8s %-5s %-16s %-7s %-7s %s\n", "TIME", "EVENT", "COMM", "PID", "PPID", "FILENAM/EXIT CODE");
    while (!exiting) {
        /* 100ms 的超时时间 */
        err = ring_buffer__poll(rb, 100);

        /* ctrl + c 会返回这个值，在这里提前处理 */
        if (err == -EINTR) {
            err = 0;
            break;
        }

        if (err < 0) {
            printf("Error polling perf buffer: %d\n", err);
            break;
        }
    }

cleanup:
    ring_buffer__free(rb);
cleanup_skel:
    kernel_bpf__destroy(skel);

    return err < 0 ? -err : 0;
}