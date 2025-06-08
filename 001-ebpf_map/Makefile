CLANG 		?= clang
LLVM_STRIP 	?= llvm-strip
LLVM_LINK	?= llvm-link
BPFTOOL 	?= bpftool
ARCH 		?= x86
CC			?= gcc

TOPDIR 		= $(shell pwd)
BUILD_PATH	= $(TOPDIR)/build

BPF_SRC 	= $(wildcard $(TOPDIR)/src/kernel/*.bpf.c)
USER_SRC 	= $(wildcard $(TOPDIR)/src/user/*.c)
BPF_OBJS 	= $(BPF_SRC:.c=.ll)
USER_OBJS 	= $(USER_SRC:.c=.o)

# 固定生成的骨架代码中结构体的名称和文件名，达到不同项目共用相同函数接口和文件名的目的
MODE_NAME 		= kernel_bpf
SKELETON_FILE 	= $(TOPDIR)/include/user/$(MODE_NAME).skel.h
VMLINUX_FILE 	= $(TOPDIR)/include/kernel/vmlinux.h
BPF_LINK		= $(BUILD_PATH)/$(MODE_NAME).ll
BPF_ELF			= $(BUILD_PATH)/$(MODE_NAME).bpf
USER_ELF		= $(BUILD_PATH)/user

INCLUDE 		= -I$(TOPDIR)/include
LD				= -lbpf
DEBUG			= -g
WALL 			= -Wall
OPTIMIZE		= -O2

CLANG_FLAGS		= -target bpf -D__TARGET_ARCH_$(ARCH) $(DEBUG) $(WALL) $(OPTIMIZE) $(INCLUDE)
C_FLAGS			= $(DEBUG) $(WALL) $(OPTIMIZE) $(INCLUDE)

.PHONY: all bpf vmlinux skeleton

all: $(USER_ELF)

bpf: $(BPF_ELF)

vmlinux: $(VMLINUX_FILE)

skeleton: $(SKELETON_FILE)

%.bpf.ll: %.bpf.c
	$(CLANG) $(CLANG_FLAGS) -emit-llvm -S $< -o $@

%.o: %.c
	$(CC) $(C_FLAGS) -c $< -o $@

$(VMLINUX_FILE):
	$(BPFTOOL) btf dump file /sys/kernel/btf/vmlinux format c > $@

$(BPF_LINK): $(BPF_OBJS)
	$(LLVM_LINK) $^ -S -o $@

$(BPF_ELF): $(BUILD_PATH) $(VMLINUX_FILE) $(BPF_LINK)
	$(CLANG) -target bpf -O2 -c $(BPF_LINK) -o $@
	$(LLVM_STRIP) -g $@

$(SKELETON_FILE): $(BPF_ELF)
	$(BPFTOOL) gen skeleton -p $< > $@ name $(MODE_NAME)

$(USER_ELF): $(SKELETON_FILE) $(USER_OBJS)
	$(CC) $(C_FLAGS) $^ -o $@ $(LD)

$(BUILD_PATH):
	mkdir -p $(BUILD_PATH)

clean:
	rm -rf $(BPF_OBJS) $(USER_OBJS) $(SKELETON_FILE) $(VMLINUX_FILE) $(BUILD_PATH)
