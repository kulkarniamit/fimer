#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

#include "job.h"

void print_jobs_list(struct job *head);
void append_jobs_list(struct job **head_ref, struct job *new);

#endif
