#include "kernel/bpf_kernel.h"

/**
 * 在任务执行点打桩
 */
SEC("tp/sched/sched_process_exec")
int handle_exec(struct trace_event_raw_sched_process_exec *ctx)
{
    struct task_struct *task;
    unsigned fname_off;
    struct event *e;
    pid_t pid;
    u64 ts;

    /* 将要执行的任务的PID和开始时间记录到HASH映射中 */
    pid = bpf_get_current_pid_tgid() >> 32;
    ts = bpf_ktime_get_ns();
    bpf_map_update_elem(&exec_start, &pid, &ts, BPF_ANY);

    /* 当设置了该参数时，不向用户发送数据 */
    if (min_duration_ns) {
        return 0;
    }

    /* 在环形队列中填充一个与用户交互的结构体 */
    e = bpf_ringbuf_reserve(&rb, sizeof(*e), 0);
    if (!e) {
        return 0;
    }

    /* 填充相应的数据到与用户交互的结构体 */
    task = (struct task_struct *)bpf_get_current_task();

    e->exit_event = false;
    e->pid = pid;
    e->ppid = BPF_CORE_READ(task, real_parent, tgid);

    /* 获取执行的命令字符串 */
    bpf_get_current_comm(&e->comm, sizeof(e->comm));

    /* 获取执行的命令的绝对路径 */
    fname_off = ctx->__data_loc_filename & 0xFFFF;
    bpf_probe_read_str(&e->filename, sizeof(e->filename), (void *)ctx + fname_off);

    /* 填充完毕后，将数据发送给用户 */
    bpf_ringbuf_submit(e, 0);

    return 0;
}
