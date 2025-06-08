#include "kernel/bpf_kernel.h"

/* 所有bpf文件只需要一个LICENSE字段就可以了 */
char LICENSE[] SEC("license") = "GPL";

/**
 * 定义一个hash类型的映射，pid为键 时间为值
 * 在进入时保存，在退出时取出
 * 用于计算任务的执行时间
 */
struct bpf_map_hash {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, 8192);
    __type(key, pid_t);
    __type(value, u64);
} exec_start SEC(".maps");

/**
 * 定义一个和用户交互的环形队列映射
 * 可以将自定义的数据发送给用户
 */
struct bpf_map_ringbuf {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 256 * 1024);
} rb SEC(".maps");

/**
 * 该变量用于控制是否向用户传输自定义的数据
 * 在用户端进行初始化
 */
const volatile unsigned long long min_duration_ns = 0;
