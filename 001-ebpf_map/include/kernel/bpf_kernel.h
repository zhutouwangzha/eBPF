#ifndef __KERNEL_BPF_KERNEL_H__
#define __KERNEL_BPF_KERNEL_H__

#include "kernel/vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>

#include "common/bootstrap.h"

extern const volatile unsigned long long min_duration_ns;
extern struct bpf_map_hash exec_start;
extern struct bpf_map_ringbuf rb;

#endif /* __KERNEL_BPF_KERNEL_H__ */