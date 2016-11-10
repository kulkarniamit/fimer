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
            current->job_expiry.tv_sec);
        syslog(
            LOG_INFO,
            "Data: 0x%04x:%s:%s\n",
            current->data->opcode,
            current->data->filepath,
            current->data->params);
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

void free_job_data(struct job *job_to_free)
{	
	free(job_to_free->data->params);
	free(job_to_free->data->filepath);
	free(job_to_free->data);
}

void delete_job(struct job **head_ref, struct job *job_to_delete)
{
	struct job *head = *head_ref;
	struct job *prev = head;
	if(head == job_to_delete){
		/*	head is supposed to be deleted	*/
		if(head->next == NULL){
			/*	There is only 1 node	*/
			free_job_data(head);
			free(head);
			*head_ref = NULL;
			return;
		}
		else{
			/* There are multiple nodes	*/
			*head_ref = head->next;
			free_job_data(head);
			free(head);
			return;
		}
	}
	
	while(prev->next != NULL && prev->next != job_to_delete){
		prev = prev->next;
	}
	if(prev->next == NULL){
		syslog(LOG_INFO,"Job not found in the list\n");
		return;
	}

	/* Remove the given job */
	prev->next = prev->next->next;
	free_job_data(job_to_delete);
	free(job_to_delete);
}
