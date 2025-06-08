#ifndef __USER_PARSE_ARG_H__
#define __USER_PARSE_ARG_H__

#include <argp.h>
#include <stdbool.h>

struct parse_arg {
    bool debug_log;                         /* 是否输出调式信息 */
    unsigned long long min_duration_ms;     /* 最小执行时间，如果执行的命令时间小于该值，则忽略采集它的数据 */
};

extern struct parse_arg user_args;
extern const struct argp arg_assemble;

#endif /* __USER_PARSE_ARG_H__ */