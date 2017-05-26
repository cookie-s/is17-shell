#include "parse.h"

typedef struct _job_list {
    job *job;
    struct _job_list *next;
} job_list;

