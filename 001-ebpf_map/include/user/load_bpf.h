#ifndef __USER_LOAD_BPF_H__
#define __USER_LOAD_BPF_H__

#include "user/kernel_bpf.skel.h"

struct kernel_bpf *load_bpf_to_kernel(void);

#endif /* __USER_LOAD_BPF_H__ */