#include "kernel/bpf_kernel.h"

/**
 * 在任务退出点打桩
 */
SEC("tp/sched/sched_process_exit")
int handle_exit(struct trace_event_raw_sched_process_exec *ctx)
{
    struct task_struct *task;
    struct event *e;
    pid_t pid, tid;
    u64 id, *start_ts, duration_ns = 0;

    /* 获取要退出任务的pid和tid */
    id = bpf_get_current_pid_tgid();
    pid = id >> 32;
    tid = (u32)id;

    if (pid != tid) {
        return 0;
    }

    /* 根据PID找到该任务的开始时间 */
    start_ts = bpf_map_lookup_elem(&exec_start, &pid);
    if (start_ts) {
        duration_ns = bpf_ktime_get_ns() - *start_ts;
    } else if (min_duration_ns) {
        return 0;
    }

    /* 从HASH映射中删除该键值对 */
    bpf_map_delete_elem(&exec_start, &pid);

    /* 当命令的持续时间小于设定的时间，则直接返回 */
    bpf_printk("%lld\n", min_duration_ns);
    if (min_duration_ns && duration_ns < min_duration_ns) {
        return 0;
    }

    /* 在环形队列中填充一个与用户交互的结构体 */
    e = bpf_ringbuf_reserve(&rb, sizeof(*e), 0);
    if (!e) {
        return 0;
    }

    /* 填充相应的数据到与用户交互的结构体 */
    task = (struct task_struct *)bpf_get_current_task();

    e->exit_event = true;
    e->pid = pid;
    e->ppid = BPF_CORE_READ(task, real_parent, tgid);

    /* 将任务执行时间存放在结构体中 */
    e->duration_ns = duration_ns;

    /* 获取任务的执行时间 */
    e->exit_code = (BPF_CORE_READ(task, exit_code)) & 0xff;

    /* 获取任务的命令字符串 */
    bpf_get_current_comm(&e->comm, sizeof(e->comm));

    /* 填充完毕后，将数据发送给用户 */
    bpf_ringbuf_submit(e, 0);

    return 0;
}