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

#include <ctype.h>
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

#define LISTENING_PORT 51515
#define BACKLOG	2
#define MESSAGE_BUFFER_SIZE 1024
#define MESSAGE_SEPARATOR ":"
/*	Accepted timestamp units: 'h','s','m','d','M','y','w'	*/
#define DEFAULT_TIME_UNIT 'm'

struct job *head = NULL;
struct timespec current_time;
/**********************************************************************/
/*	Operation methods to be moved to a header file					  */
/**********************************************************************/
void process_chmod(char *filepath, char *params)
{
	int i;
	char *savedptr = NULL;
	/* params is of the form: "0444:"	*/
	/*	params is ':' separated, get relevant information	*/
	char *permissions = strtok_r(params, MESSAGE_SEPARATOR, &savedptr);
	i = strtol(permissions, 0, 8);
	if (chmod (filepath,i) < 0){
		syslog(LOG_ERR, "File permissions could not be changed for %s",
			   filepath);
	}
}

char get_duration_unit(char *received_duration)
{
	while(!isalpha(*received_duration) && *received_duration != '\0'){
		received_duration++;
	}
	return (*received_duration == '\0')?
			DEFAULT_TIME_UNIT:
			*received_duration;
}
/**********************************************************************/
unsigned int get_duration_seconds(unsigned int duration, char unit)
{
	unsigned int duration_in_seconds;
	switch(unit){
		case 's':	duration_in_seconds = duration;
					break;
		case 'm':	duration_in_seconds = MINUTES_TO_SECONDS(duration);
                    break;
		case 'h':	duration_in_seconds = HOURS_TO_SECONDS(duration);
                    break;
		case 'd':	duration_in_seconds = DAYS_TO_SECONDS(duration);
                    break;
		case 'M':	duration_in_seconds = MONTHS_TO_SECONDS(duration);
                    break;
		case 'w':	duration_in_seconds = WEEKS_TO_SECONDS(duration);
                    break;
		case 'y':	duration_in_seconds = YEARS_TO_SECONDS(duration);
                    break;
		default:	exit(EXIT_FAILURE);
	}
	return duration_in_seconds;
}

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
	unsigned int opcode, duration, expiry_time;
	char duration_unit;

	/*
		Well, strtok_r() has been generous enough to do this job.
		All we have to do it just pass char * pointer and it will
		take care of the rest of the allocation job	*/
	char *received_filepath, *received_duration, *received_opcode;
	char *received_parameters;
	char *saveptr = NULL;

	/*	strtok_r() modifies its first argument, save a backup	*/
	char *received_message = strdup(job_message);
	
	received_filepath = strtok_r(received_message, MESSAGE_SEPARATOR, &saveptr);
	received_duration = strtok_r(NULL, MESSAGE_SEPARATOR, &saveptr);
	received_opcode   = strtok_r(NULL, MESSAGE_SEPARATOR, &saveptr);
	received_parameters = saveptr;

	opcode   = (unsigned int)strtol(received_opcode, NULL, 10);
	duration = (unsigned int)strtol(received_duration, NULL, 10);
	duration_unit = get_duration_unit(received_duration);
	expiry_time = get_duration_seconds(duration, duration_unit);

	/*  Initialize job_data struct  */
	data->filepath = malloc(strlen(received_filepath)+1);   /* Remember +1 */
	strcpy(data->filepath, received_filepath);
	data->params= malloc(strlen(received_parameters)+1);    /* Remember +1 */
	strcpy(data->params, received_parameters);
	
	switch(opcode){
		case OCHMOD:
			data->opcode = OCHMOD;
			data->job_worker = process_chmod;
			break;
		default:	exit(EXIT_FAILURE);	
	}
	
	clock_gettime(CLOCK_MONOTONIC, &job_assign_time);
	job_expiry_time = job_assign_time;
	job_expiry_time.tv_sec += expiry_time;

	create_new_job(data,&job_expiry_time);
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

void *thread_job(void *ptr){
	/*	Socket listening code	*/
	int listen_fd = 0, conn_fd = 0;
	struct sockaddr_in serv_addr;
	char buffer[MESSAGE_BUFFER_SIZE];
	struct timespec job_assign_time, now;

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
	    syslog(LOG_INFO, "Current time: %d\n",current_time.tv_sec);
	
		/*	Function to parse and add job to queue	*/	
		assign_job(buffer);
		memset(buffer, 0, sizeof(buffer));
	}
	close(listen_fd);
}

void execute_eligible_jobs(){
	/*	
		1.	Run through jobs list 
		2.	Perform jobs that have expired
		3.	Unlink expired job from list
	*/
	clock_gettime(CLOCK_MONOTONIC, &current_time);
	syslog(LOG_INFO, "Current time: %d\n",current_time.tv_sec);
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

int main(int argc, char *argv[]) {
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
	    sleep(2);
		if(head != NULL){
			syslog(LOG_INFO, "head is not NULL, continuing check..\n");
			/*	There are some jobs in the linkedlist queue	*/
			execute_eligible_jobs();
		}
	}
	
	/* this is optional and only needs to be done when your daemon exits */
	closelog();
}
