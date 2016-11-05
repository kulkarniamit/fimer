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
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

#define LISTENING_PORT 51515
#define BACKLOG	2
#define MESSAGE_BUFFER_SIZE 1024

int global_guy=1;

void display_error_exit()
{
    fprintf(
        stdout,
        "\n%s: %s\n\n",
        __FILE__,
        strerror(errno));
    exit(1);
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

	conn_fd = accept(listen_fd, (struct sockaddr*)NULL, NULL);
	if(conn_fd <= 0){
		display_error_exit();
	}
	while(1){
		if(read(conn_fd, buffer, MESSAGE_BUFFER_SIZE) == 0){
			/*	No bytes read	*/
			puts("Exiting..");
			close(conn_fd);
			break;
		}
		syslog(LOG_INFO, "Received a message on socket\n");

		fprintf(stdout, "Received message: %s\n",buffer);
		clock_gettime(CLOCK_MONOTONIC, &job_assign_time);
		now = job_assign_time;
		now.tv_sec += 5;
		fprintf(stdout, "Start time: %lu\n",job_assign_time.tv_sec);
		fprintf(stdout, "End time: %lu\n",now.tv_sec);
		sleep(5);
		struct timespec ts_current;
		struct timespec ts_remaining;
		clock_gettime(CLOCK_MONOTONIC, &ts_current);
		fprintf(stdout, "New time: %lu\n",ts_current.tv_sec);
		
		ts_remaining.tv_sec = now.tv_sec - ts_current.tv_sec;

		if(ts_remaining.tv_sec <= 0){
			fprintf(stdout, "Alright, time's up!\n");
			fprintf(stdout, "Time to perform operation..\n");
			syslog(LOG_INFO, "Time elapsed, happy times now\n");
			break;
		}
		memset(buffer, 0, sizeof(buffer));
	}
	syslog(LOG_INFO, "Updating global guy now..\n");
	global_guy++;
	close(listen_fd);
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
	
	/* A useless payload to demonstrate */
	while (1) {
	    sleep(2);
	    if(global_guy % 2 == 0){
	        syslog(LOG_INFO, "Found an even number!\n");
	    }
	}
	
	/* this is optional and only needs to be done when your daemon exits */
	closelog();
}
