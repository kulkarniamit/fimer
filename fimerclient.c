/**
 * @file fimerclient.c
 * @author Amit Kulkarni
 * @date 3 Nov 2016
 * @brief Despatch jobs to server using sockets
 *
 * Sanitize the inputs, validate the input, construct
 * a server readable message and dispatch the job to 
 * server
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>

/* Support safe and consistent functions like strlcat() */
#include <bsd/string.h>

#include "include/opcodes.h"

#define SERVER_PORT 51515
#define BACKLOG	2
#define MESSAGE_BUFFER_SIZE 1024
#define SERVER_ADDRESS "127.0.0.1"

void display_error_exit()
{
	fprintf(
		stdout, 
		"\n%s: %s\n\n", 
		__FILE__, 
		strerror(errno));
	exit(1);
}

void usage(char *prog_name)
{
	fprintf(
		stdout, 
		"usage: %s %s %s %s %s %s\n",
		prog_name,
		"opcode",
		"operation",
		"permissions",
		"filepath",
		"timer_in_seconds");
	fprintf(stdout, "\tchmod a file after some time\n");
}
 	
void serv_addr_init(struct sockaddr_in *serv_addr_ptr)
{
	serv_addr_ptr->sin_family = AF_INET;
	if(inet_pton(AF_INET, SERVER_ADDRESS, &serv_addr_ptr->sin_addr) <= 0){
		fprintf(stdout, "%s: %s\n", __FUNCTION__, strerror(errno));
		exit(1);
	}
	serv_addr_ptr->sin_port = htons(SERVER_PORT);
}

void file_path_validation(char *filepath, char *absolute_filepath)
{
	if(realpath(filepath, absolute_filepath) == NULL){
		/*
		We could use perror() but I need some more info during 
		debug to find out where exactly it failed
		perror(filepath);
		*/
		display_error_exit();
	}
}

void process_ochmod(char *permissions, 	char *filepath, char *timer, 
					char *dispatch_message)
{
	char absolute_filepath [PATH_MAX+1];
	struct stat *filestat = malloc(sizeof(struct stat));

	file_path_validation(filepath, absolute_filepath);	
	printf("\nAbsolute path: %s\n\n", absolute_filepath);

	if((stat(absolute_filepath, filestat)!= 0) ||
		(filestat->st_uid != geteuid())){
		display_error_exit();
	}
	free(filestat);
	fprintf(stdout, "We are all set to do chmod\n");
	/* 	This job is eligible to be put in the queue	*/
	/*	Despatch the job to the server	*/
	strlcat(dispatch_message, "chmod", MESSAGE_BUFFER_SIZE);
	strlcat(dispatch_message, " ",MESSAGE_BUFFER_SIZE);
	strlcat(dispatch_message, permissions,MESSAGE_BUFFER_SIZE);
	strlcat(dispatch_message, " ",MESSAGE_BUFFER_SIZE);
	strlcat(dispatch_message, absolute_filepath, MESSAGE_BUFFER_SIZE);
	strlcat(dispatch_message, " ",MESSAGE_BUFFER_SIZE);
	strlcat(dispatch_message, timer, MESSAGE_BUFFER_SIZE);
	fprintf(stdout, "Final message: %s\n",dispatch_message);
}

void send_message(char *message, int sock_fd)
{
	/* 
		Ideally there has to be a while(1) which waits
		for an acknowledgement from server about job
		addition to the queue.
		For now, let's assume the message safely reached
		the server and server has added it to the queue
	*/
	if(write(sock_fd, message, MESSAGE_BUFFER_SIZE) <= 0){
		display_error_exit();
	}
	else{
		fprintf(stdout, "Job successfully added to the queue\n");
	}
	close(sock_fd);
}

int main(int argc, char *argv[])
{
	/*
		Expected arglist:
		argv[1] = OCHMOD (indicating the operation is chmod)
		argv[2] = chmod
		argv[3] = Permission bits (either with '0' or without)
		argv[4] = File path
		argv[5] = Time duration (let's say in seconds for now)
	*/
	int sock_fd = 0;
	struct sockaddr_in serv_addr;
	char buffer[MESSAGE_BUFFER_SIZE];

	/*	 Probably this if would be replaced with a switch() */
	if(argc < 2){
		usage(argv[0]);
		exit(1);
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	memset(buffer, 0, sizeof(buffer));
	serv_addr_init(&serv_addr);

	if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stdout, "ERROR: %s\n", strerror(errno));
		return 1;

	}
	if(connect(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))){
		fprintf(stdout, "%s: %s\n", __FUNCTION__, strerror(errno));
		return 1;
	}

	if(!strncmp(argv[1], "OCHMOD", 6)){
		// Make sure it is a chmod operation
		process_ochmod(argv[3], argv[4], argv[5], buffer);
		send_message(buffer, sock_fd);
	}
/*
	while(1){
		puts("Please enter the message for server [EOC to terminate]:");
		fgets(buffer, MESSAGE_BUFFER_SIZE, stdin);
		if(strncmp(buffer, "EOC",3) == 0){
			break;
		}
		if(write(sock_fd, buffer, MESSAGE_BUFFER_SIZE) <= 0){
			fprintf(stdout, "ERROR: %s\n", strerror(errno));
		}
	}
	close(sock_fd);
*/
	return 0;
}
