#include <stdio.h>
#include <stdlib.h>
#include "parse.h"

void print_job_list(job* job_list) {
    int      index;
    job*     jb;
    process* pr;

    for(index = 0, jb = job_list; jb != NULL; jb = jb->next, ++index) {
        pr = jb->process_list;
        printf(" [%d]  %s % 6d %s%s\n", jb->jobid, pr->status == RUNNING ? " RUN" : pr->status == STOP ? "STOP" : "TERM",
                pr->pid, pr->program_name, (pr->next ? " |" : ""));
        for(pr = pr->next; pr != NULL; pr = pr->next) {
            printf("      %s % 6d %s%s\n", pr->status == RUNNING ? " RUN" : pr->status == STOP ? "STOP" : "TERM",
                    pr->pid, pr->program_name, (pr->next ? " |" : ""));
        }
    }
}
