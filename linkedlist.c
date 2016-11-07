#include <stdlib.h>
#include <syslog.h>
#include "include/job.h"
#include "include/linkedlist.h"

void print_jobs_list(struct job *head)
{
    struct job *current = head;
    while(current != NULL){
        syslog(LOG_INFO, "Printing job params\n");
        syslog(LOG_INFO, "delete flag: %d\n", current->delete);
        syslog(
            LOG_INFO,
            "Duration: %d%c\n",
            current->duration->time,
            current->duration->unit);
        syslog(
            LOG_INFO,
            "Data: 0x%04x:%s:%s\n",
            current->data->opcode,
            current->data->filepath,
            current->data->params);
        current->data->job_worker(current->data->params);
        current = current->next;
    }
}

void append_jobs_list(struct job **head_ref, struct job *new)
{
    /*  Given a reference (pointer to pointer) to the head
        of a list and an int, appends a new node at the end  */
    struct job *last = *head_ref;

    /*  New node is going to be the last node, so make next 
        of it as NULL*/
    new->next = NULL;

    /*  If the Linked List is empty, then make the new job as head */
    if(*head_ref == NULL){
        *head_ref = malloc(sizeof(struct job));
        *head_ref = new;
        return;
    }

    /*  Else traverse till the last node */
    while (last->next != NULL)
        last = last->next;

    /*  Change the next of last node */
    last->next = new;
}
