/**
 * @file fimerd.c
 * @author Amit Kulkarni
 * @date 5 Nov 2016
 * @brief Daemon that runs jobs and accepts socket connections 
 *
 * Daemon creates a worker thread that spawns with child process and
 * goes into an accept() blocking mode for listening to connections
 * Meanwhile, daemon forked process periodically checks for any 
 * jobs that have expired and executes the corresponsing instruction

 * This file is intended to run as a daemon in it's final form
 * The daemon acts as a server listening on a port for client requests
 * It also periodically checks for expired timers of jobs in the queue
 * If a job has expired, action is taken and job is removed from queue
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

#include "include/opcodes.h"
#include "include/job.h"
#include "include/linkedlist.h"
#include "include/utilities.h"
#include "include/parser.h"
#include "include/operations.h"

#define LISTENING_PORT 51515
#define BACKLOG	2
#define MESSAGE_BUFFER_SIZE 1024
/*	Accepted timestamp units: 'h','s','m','d','M','y','w'	*/
//#define DEFAULT_TIME_UNIT 'm'

struct job *head = NULL;
struct timespec current_time;

void create_new_job(struct job_data *data, struct timespec *expiry_time)
{
	struct job *new = malloc(sizeof(struct job));
    new->delete = 0;    /*	Job hasn't expired yet   */
    new->data = data;
    new->job_expiry = *expiry_time;
    new->next = NULL;	/*	Job appended at the end of list	*/
    append_jobs_list(&head, new);
    print_jobs_list(head);
}

void assign_job(char *job_message)
{
	/*	
		Generic function that delegates the job to different
		functions depending on opcode
	*/
	struct timespec job_assign_time, job_expiry_time;
	struct job_data *data = malloc(sizeof(struct job_data));
	struct message *parsed_message = malloc(sizeof(struct message));

	parsed_message = get_parsed_message(job_message);

	/* Remember +1 */
	data->filepath = malloc(strlen(parsed_message->received_filepath)+1);
	strcpy(data->filepath, parsed_message->received_filepath);
	/* Remember +1 */
	data->params= malloc(strlen(parsed_message->received_parameters)+1);
	strcpy(data->params, parsed_message->received_parameters);
	
	switch(parsed_message->opcode){
		case OCHMOD:
			data->opcode = OCHMOD;
			data->job_worker = process_chmod;
			break;
		case OREMOVE:
			data->opcode = OREMOVE;
			data->job_worker = process_rm;
			break;
		default:	exit(EXIT_FAILURE);	
	}
	
	clock_gettime(CLOCK_MONOTONIC, &job_assign_time);
	job_expiry_time = job_assign_time;
	job_expiry_time.tv_sec += parsed_message->expiry_time;

	create_new_job(data,&job_expiry_time);
	free(parsed_message);
}

void display_error_exit()
{
    syslog(
        LOG_INFO,
        "\n%s: %s\n\n",
        __FILE__,
        strerror(errno));
    exit(EXIT_FAILURE);
}

void serv_addr_init(struct sockaddr_in *serv_addr_ptr)
{
    serv_addr_ptr->sin_family = AF_INET;
    serv_addr_ptr->sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr_ptr->sin_port = htons(LISTENING_PORT);
}

void *thread_job(void *ptr)
{
	/*	Socket listening code	*/
	int listen_fd = 0, conn_fd = 0;
	struct sockaddr_in serv_addr;
	char buffer[MESSAGE_BUFFER_SIZE];

	memset(&serv_addr, 0, sizeof(serv_addr));
	memset(buffer, 0, sizeof(buffer));
	serv_addr_init(&serv_addr);
	
	if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		display_error_exit();
	}
	if(bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
		display_error_exit();
	}
	if(listen(listen_fd, BACKLOG) == -1){
		display_error_exit();
	}

	while(1){
		conn_fd = accept(listen_fd, (struct sockaddr*)NULL, NULL);
		if(conn_fd <= 0){
			display_error_exit();
		}
		if(read(conn_fd, buffer, MESSAGE_BUFFER_SIZE) == 0){
			/*	No bytes read	*/
			puts("Exiting..");
			close(conn_fd);
			break;
		}
		syslog(LOG_INFO, "Received a message on socket\n");
		clock_gettime(CLOCK_MONOTONIC, &current_time);
	    syslog(LOG_INFO, "Current time: %ld\n",current_time.tv_sec);
	
		/*	Function to parse and add job to queue	*/	
		assign_job(buffer);
		memset(buffer, 0, sizeof(buffer));
	}
	close(listen_fd);
	pthread_exit(NULL);
}

void execute_eligible_jobs(){
	/*	
		1.	Run through jobs list 
		2.	Perform jobs that have expired
		3.	Unlink expired job from list
	*/
	clock_gettime(CLOCK_MONOTONIC, &current_time);
//	syslog(LOG_INFO, "Current time: %d\n",current_time.tv_sec);
	struct job *current = head;
	struct job *next_job;
	while(current != NULL){
		next_job = current->next;
		if(current->job_expiry.tv_sec <= current_time.tv_sec){
			syslog(LOG_INFO,"Time to delete job!\n");
			/*	Eligible job to be executed	*/
			current->data->job_worker(current->data->filepath, 
									  current->data->params);
		
			/* Job done. Remove the job	*/
			delete_job(&head, current);
			current = next_job;
			continue;
		}
		current = current->next;
	}

}

int main(int argc, char *argv[])
{
	pid_t pid, sid;
	
	pthread_t server_thread;
	
	/* Clone ourselves to make a child */
	pid = fork();
	
	/* If the pid is less than zero,
	 *    something went wrong when forking */
	if (pid < 0) {
	    exit(EXIT_FAILURE);
	}
	
	/* If the pid we got back was greater
	 *    than zero, then the clone was
	 *       successful and we are the parent. */
	if (pid > 0) {
	    exit(EXIT_SUCCESS);
	}
	
	/* If execution reaches this point we are the child */
	/* Set the umask to zero */
	umask(0);
	
	/* Open a connection to the syslog server */
	openlog(argv[0],LOG_NOWAIT|LOG_PID,LOG_USER);
	
	/* Sends a message to the syslog daemon */
	syslog(LOG_NOTICE, "Successfully started daemon\n");

	/* Try to create our own process group */
	sid = setsid();
	if (sid < 0) {
	    syslog(LOG_ERR, "Could not create process group\n");
	    exit(EXIT_FAILURE);
	}
	
	/* Change the current working directory */
	if ((chdir("/")) < 0) {
	    syslog(LOG_ERR, "Could not change working directory to /\n");
	    exit(EXIT_FAILURE);
	}
	
	/* Close the standard file descriptors */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	
	/*  Create a thread that updates global guy */
	pthread_create(&server_thread, NULL, thread_job, NULL);
	
	while (1) {
	    sleep(1);
		if(head != NULL){
//			syslog(LOG_INFO, "head is not NULL, continuing check..\n");
			/*	There are some jobs in the linkedlist queue	*/
			execute_eligible_jobs();
		}
	}
	
	/* this is optional and only needs to be done when your daemon exits */
	closelog();
	return 0;
}
