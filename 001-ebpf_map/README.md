# 简介

## 功能

该`BPF`程序主要是在程序的进入和退出阶段采取数据，主要采集的数据定义在`include/common/bootstrap.h`中，包括：

- pid
- ppid
- exit_code
- 执行时间
- 任务命令字符串
- 任务的绝对路径

## 目录结构

```tex
.
├── include
│   ├── common						/* 内核和用户共用的头文件 */
│   │   └── bootstrap.h				/* 包含内核与用户交互的结构体 */
│   ├── kernel						/* 在编译时，会在该目录生成包含所有内核结构体的vmlinux.h文件 */
│   │   └── bpf_kernel.h			/* 内核独有的头文件，主要用于包含内核和用户的共用头文件和导出要使用的映射 */
│   └── user						/* 在编译后，会在该目录依据BPF程序自动生成的骨架代码 */
│       ├── load_bpf.h				/* 用户独有的头文件 */
│       └── parse_arg.h				/* 解析参数的头文件，定义了参数结构体 */
├── Makefile						/* 编译脚本，无论bpf文件名如何，每次都生成kernel_bpf.skel.h骨架代码，且内部结构体名称也不变，
|									 * 达到用户程序代码(src/user/load_bpf.c)复用的目的 */
├── README.md						/* 自述文件 */
└── src
    ├── kernel
    │   ├── handle_exec.bpf.c		/* 实现在任务执行时，向用户发送数据 */
    │   ├── handle_exit.bpf.c       /* 实现在任务退出时，向用户发送数据 */
    │   └── main.bpf.c              /* 定义其他BPF程序要使用的映射和LICENSE */
    └── user
        ├── load_bpf.c				/* 调用骨架代码，完成bpf程序的加载以及参数设置 */
        ├── main.c					/* 主要的处理逻辑 */
        └── parse_arg.c				/* 用于参数解析 */
```

后续的开发也可沿用该目录结构、部分文件以及`Makefile`。

当替换到新的项目时，修改`include/common/bootstrap.h`、`include/common/parse_arg.h`、`src/kernel`下的文件、`src\user\`下的文件即可。

# 使用

## 编译

```shell
make
```

编译成功后，会在当前目录创建`build`目录，`build`目录包含`kernel_bpf.bpf`(bpf 二进制文件),`kernel_bpf.ll`(bpf 链接文件),`user`(用户态可执行文件)三个文件。

## 运行

```shell
./build/user [-b] [-d <min-duration-ms>]
```

其中`-b`表示打开`libbpf`的调式输出信息，`-d <min-duration-ms>`表示过滤掉执行时间少于`min-duration-ms`的任务，命令的其余信息可通过`--help`查看。

成功执行后，会在中断输出如下内容：

```shell
TIME     EVENT COMM             PID     PPID    FILENAM/EXIT CODE
```

随后在另一个中断执行任何命令，例如`ls`、`cat`等，内核`bpf`程序就会执行相应逻辑，将命令的相关数据返回到用户态：

```shell
TIME     EVENT COMM             PID     PPID    FILENAM/EXIT CODE
14:43:50 EXEC  ls               9762    2369    /usr/bin/ls
14:43:50 EXIT  ls               9762    2369    [0] (2ms)
```

