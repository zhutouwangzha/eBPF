#include <stdlib.h>

#include "user/parse_arg.h"

struct parse_arg user_args = {0};

static const struct argp_option opts[] = {
    /* 长参数       短参    跟的参数            参数的详细解释 */
    {"debug",       'b',    NULL,           0,  "Debug output"},
    {"duration",    'd',    "DURATION_MS",  0,  "Minimum process duration (ms) to report"},
    {}
};

const char *argp_program_version = "0.1";   /* 版本号 */
const char *argp_program_bug_address = "";  /* 错误地址 */

static char doc[] = "该命令可使能调式输出与屏蔽时间较短的命令";
static char argp_program_doc[] = "user [-d <min-duration-ms>] [-b]";

static error_t parse_arg(int key, char *arg, struct argp_state *state)
{
    switch (key) {
    case 'b':
        user_args.debug_log = true;
        break;
    case 'd':
        user_args.min_duration_ms = strtol(arg, NULL, 10);
        if (user_args.min_duration_ms < 0) {
            fprintf(stderr, "Invalid duration: %s\n", arg);
            argp_usage(state);
        }
        break;
    case ARGP_KEY_ARG:
        argp_usage(state);
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

const struct argp arg_assemble = {
    .options = opts,
    .parser = parse_arg,
    .args_doc = argp_program_doc,
    .doc = doc,
};
