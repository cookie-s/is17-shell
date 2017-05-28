#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "print.h"
#include "parse.h"
#include "jobs.h"
#include "job_list.h"
#include "signals.h"
#include "commands.h"

job_list *running_job_list = NULL;
job_list *freeing_job_list = NULL;
job *job_fg = NULL;

void wait_children() {
    while(1) {
        pid_t pid;
        int wstatus;
        if((pid = waitpid(-1, &wstatus, WNOHANG | WUNTRACED | WCONTINUED)) == -1) {
            if(errno != ECHILD) perror("waitpid");
            return;
        }

        if(!pid) // no changing process
            return;

        job_list *prev = NULL, *cur_job;
        process *cur_proc;
        for(cur_job = running_job_list; cur_job; cur_job = (prev = cur_job)->next) {
            for(cur_proc = cur_job->job->process_list; cur_proc; cur_proc = cur_proc->next)
                if(cur_proc->pid == pid)
                    goto FOUND;
        }

        //fputs("no such process in running_process_list\n", stderr);
        return;

FOUND:
        cur_proc->wstatus = wstatus;
        cur_proc->status  =
            WIFCONTINUED(wstatus) ? RUNNING :
            WIFSTOPPED(wstatus)? STOP : TERMINATED;

        if(cur_proc->status == TERMINATED)
            cur_job->job->running_process_num--;

        if(cur_job->job->running_process_num == 0) { // this job's all processes have terminated
            // unlink from running_job_list
            if(!prev)
                running_job_list = cur_job->next;
            else
                prev->next = cur_job->next;

            // link to freeing_job_list
            cur_job->next = freeing_job_list;
            freeing_job_list = cur_job;
        }
    }
}

void wait_fgjob(void) {
    block_sigchld();
    while(job_fg) {
        sigset_t ss;
        int sig;
        char stop = 0;
        process *cur_proc;
        for(cur_proc = job_fg->process_list; cur_proc; cur_proc = cur_proc->next) {
            if(cur_proc->status == RUNNING) // if running proccess exists, this job hasn't finished yet
                goto CONT;
            if(cur_proc->status == STOP)
                stop = 1;
        }
        // all of foreground procs are not running, so, this job is background rather than foreground job
        if(stop) job_fg = NULL;
        break;
CONT:
        // wait for any signals including SIGCHLD
        sigemptyset(&ss);
        sigaddset(&ss, SIGCHLD);
        if(sigwait(&ss, &sig)) perror("sigwait");
        wait_children();
    }
    unblock_sigchld();
}

void free_freeingjobs(void) {
    int first = 1;

    while(freeing_job_list) {
        if(freeing_job_list->job == job_fg)
            job_fg = NULL;
        else {
            if(first)
                printf("Done: \n"), first = 0;
            print_job_list(freeing_job_list->job);
        }

        free_job(freeing_job_list->job);

        job_list *next = freeing_job_list->next;
        free(freeing_job_list);
        freeing_job_list = next;
    }

}

struct command {
    const char *name;
    void (*func) (char**);
};

int main(int argc, char *argv[]) {
    char s[LINELEN];
    job *curr_job;

    // these funcs are in `commands.c'
    const struct command commands[] = {
        {.name = "jobs"   , .func = command_jobs   },
        {.name = "fg"     , .func = command_fg     },
        {.name = "bg"     , .func = command_bg     },
        {.name = "cd"     , .func = command_cd     },
        {NULL, NULL}, // don't forget this line, which indicates tail.
    };

    ignore_stop_signals();

    char first_exit = 1;

    while(get_line(s, LINELEN)) {

        wait_children();

        if(!strcmp(s, "exit\n")) {
            if(!running_job_list || !first_exit) break;

            puts("Some jobs remaining:");
            command_jobs(NULL);
            puts("");
            puts("If you really want to exit, type once more.");
            first_exit = 0;
            continue;
        }

        first_exit = 1;

        curr_job = parse_line(s);
        if(!curr_job) continue;

        // built-in commands
        for(const struct command *com = commands; com->name; com++) {
            if(!strcmp(curr_job->process_list->program_name, com->name)) {
                com->func(curr_job->process_list->argument_list);
                goto EPILOGUE;
            }
        }

        // link to running_job_list
        job_list *cur_joblist = (job_list*)malloc(sizeof(job_list));
        cur_joblist->job  = curr_job;
        cur_joblist->next = running_job_list;
        running_job_list  = cur_joblist;

        // start processes
        start_job(curr_job);

        if(curr_job->mode == FOREGROUND) {
            job_fg = curr_job;
            // set foreground
            if(tcsetpgrp(0, job_fg->pgid)) perror("tcsetpgrp");
        }

        // give jobid to job, which is unique in running_job_list
        int max = -1;
        for(job_list *cur_job = running_job_list; cur_job; cur_job = cur_job->next) {
            if(max < cur_job->job->jobid)
                max = cur_job->job->jobid;
        }
        curr_job->jobid = max+1;

EPILOGUE:
        // if there, wait for stop/termination of fg job
        wait_fgjob();

        // set myself as foreground
        if(tcsetpgrp(0, getpgid((pid_t)0))) perror("tcsetpgrp");

        puts("");

        wait_children();
        free_freeingjobs();

    }

    return 0;
}
