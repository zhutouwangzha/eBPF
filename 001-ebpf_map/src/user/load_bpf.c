#include <stdio.h>

#include "user/load_bpf.h"
#include "user/parse_arg.h"

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
    /* 完成调式信息控制 */
    if (!user_args.debug_log) {
        return 0;
    }

    return vfprintf(stderr, format, args);
}

struct kernel_bpf *load_bpf_to_kernel(void)
{
    struct kernel_bpf *skel;
    int err;

    libbpf_set_print(libbpf_print_fn);

    skel = kernel_bpf__open();
    if (!skel) {
        fprintf(stderr, "Failed to open and load BPF skeleton\n");
        return NULL;
    }

    /* 设置最小持续时间参数，内核通过该参数判断是否要向用户发送消息 */
    skel->rodata->min_duration_ns = user_args.min_duration_ms * 1000000ULL;

    err = kernel_bpf__load(skel);
    if (err) {
        fprintf(stderr, "Failed to load and verify BPF skeleton\n");
        kernel_bpf__destroy(skel);
        return NULL;
    }

    err = kernel_bpf__attach(skel);
    if (err) {
        fprintf(stderr, "Failed to attach BPF skeleton\n");
        kernel_bpf__destroy(skel);
        return NULL;
    }

    return skel;
}
