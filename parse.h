#ifndef __PARSE_H__
#define __PARSE_H__

#include <unistd.h>

#define PROMPT "ish$ " /* 入力ライン冒頭の文字列 */
#define NAMELEN 32    /* 各種名前の長さ */
#define ARGLSTLEN 16  /* 1つのプロセスがとる実行時引数の数 */
#define LINELEN 256   /* 入力コマンドの長さ */

typedef enum write_option_ {
    TRUNC,
    APPEND,
} write_option;

typedef enum proc_stat_ {
    INITIALIZED, // 初期状態
    RUNNING,     // 実行中 // *注意* RUNNINGだからといって本当に実行中とは限らない。まだ終了を捕捉されていないだけかもしれない。
    STOP,
    TERMINATED,  // 終了済み waitは不必要
} proc_stat;

// wait/waitpidの返り値
typedef int wstatus_t;

struct job_;
typedef struct process_ {
    struct job_* parent_job;

    char*        program_name;
    char**       argument_list;

    char*        input_redirection;

    write_option output_option;
    char*        output_redirection;

    pid_t        pid;
    proc_stat    status;
    wstatus_t    wstatus; // status == TERMINATEDのときのみ有効

    struct process_* next;
} process;

typedef enum job_mode_ {
    FOREGROUND,
    BACKGROUND,
} job_mode;

typedef struct job_ {
    int          running_process_num;
    int          jobid;
    pid_t        pgid;
    job_mode     mode;
    process*     process_list;
    struct job_* next;
} job;

typedef enum parse_state_ {
    ARGUMENT,
    IN_REDIRCT,
    OUT_REDIRCT_TRUNC,
    OUT_REDIRCT_APPEND,
} parse_state;

char* get_line(char *, int);
job* parse_line(char *);
void free_job(job *);

#endif
