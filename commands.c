#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include "job_list.h"
#include "print.h"

extern job_list *running_job_list;
extern job_list *freeing_job_list;
extern job *job_fg;

void command_jobs(const char **arg) {
    for(job_list *cur = running_job_list; cur; cur = cur->next)
        print_job_list(cur->job);
}

void command_fg(const char **arg) {
    if(!arg[1]) {
        if(!running_job_list) {
            fputs("there is no running job~\n", stderr);
            return;
        }
        job_fg = running_job_list->job;
    } else {
        int jobid = atoi(arg[1]);

        job_list *jo;
        for(jo = running_job_list; jo; jo = jo->next) {
            if(jo->job->jobid == jobid)
                goto FOUND;
        }
        fputs("no such job~\n", stderr);
        return;
FOUND:
        job_fg = jo->job;
    }

    for(process *pr = job_fg->process_list; pr; pr = pr->next)
        if(pr->status == STOP)
            kill(pr->pid, SIGCONT), pr->status = RUNNING;

    if(tcsetpgrp(0, job_fg->pgid)) perror("tcsetpgrp");
}

void command_bg(const char **arg) {
    job *jo;
    if(!arg[1]) {
        if(!running_job_list) {
            fputs("there is no running job~\n", stderr);
            return;
        }
        jo = running_job_list->job;
    } else {
        int jobid = atoi(arg[1]);

        job_list *jl;
        for(jl = running_job_list; jl; jl = jl->next) {
            if(jl->job->jobid == jobid)
                goto FOUND;
        }
        fputs("no such job~\n", stderr);
        return;
FOUND:
        jo = jl->job;
    }

    for(process *pr = jo->process_list; pr; pr = pr->next)
        if(pr->status == STOP)
            kill(pr->pid, SIGCONT), pr->status = RUNNING;
}

void command_cd(const char **arg) {
    if(!arg[1]) {
        fputs("sorry, I can't search home dir!\n", stderr);
        return;
    }
    
    if(chdir(arg[1]))
        perror("ish: chdir");
}
