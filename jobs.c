#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "parse.h"
#include "signals.h"

extern char** environ;

void start_process(process *proc, int rfd, int wfd) {
    pid_t pid;
    if((pid = fork()) == -1) {
        perror("ish: fork");
        proc->status = TERMINATED;
        return;
    }

    if(pid) {
        // parent
        if(rfd != 0) close(rfd);
        if(wfd != 1) close(wfd);

        if(!proc->parent_job->pgid)
            proc->parent_job->pgid = pid;

        proc->parent_job->running_process_num++;
        proc->status = RUNNING;
        proc->pid    = pid;

        // make program group in one job
        int stat;
        if((stat = setpgid(pid, proc->parent_job->pgid ? proc->parent_job->pgid : pid))) {
           if(stat == ESRCH) {
               proc->parent_job->pgid = pid;
               if(setpgid(pid, proc->parent_job->pgid))
                   perror("setpgid");
           } else
               perror("setpgid");
        }
    } else {
        // child

        if(dup2(rfd, 0) == -1) perror("ish: dup2");
        if(dup2(wfd, 1) == -1) perror("ish: dup2");
        if(rfd != 0) close(rfd);
        if(wfd != 1) close(wfd);

        // SIG_IGN would be inherited without this, which prevents child process from termitating with ^C
        dfl_stop_signals();

        if(execve(proc->program_name, proc->argument_list, environ)) {
            perror("ish: execve");
            exit(127);
        }
    }
}

void start_job(job *jo) {
    process *cur_proc;
    int nextrfd = 0;
    for(cur_proc = jo->process_list; cur_proc; cur_proc = cur_proc->next) {
        int rfd = nextrfd, wfd = 1;
        nextrfd=0;
        if(cur_proc->input_redirection) {
            if(rfd != 0) if(close(rfd)) perror("ish: close");
            if((rfd = open(cur_proc->input_redirection, O_RDONLY)) == -1) {
                perror("ish: open");
                continue;
            }
        }
        if(cur_proc->output_redirection) {
            if(wfd != 1) if(close(wfd)) perror("ish: close");
            if((wfd = open(cur_proc->output_redirection, O_WRONLY | (cur_proc->output_option == TRUNC ? O_TRUNC | O_CREAT : O_APPEND), 0644)) == -1) {
                perror("ish: open");
                continue;
            }
        }
        if(cur_proc->next) {
            int fds[2];
            if(pipe(fds) == -1) {
                perror("ish: pipe");
            } else {
                if(nextrfd != 0) if(close(nextrfd)) perror("ish: close");
                if(wfd != 1) if(close(wfd)) perror("ish: close");

                nextrfd = fds[0];
                wfd = fds[1];
            }
        }
        start_process(cur_proc, rfd, wfd);
    }
}
