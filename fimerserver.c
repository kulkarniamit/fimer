/**
 * @file fimerserver.c
 * @author Amit Kulkarni
 * @date 3 Nov 2016
 * @brief Accept socket connections from client and add to job queue
 *
 * This file is intended to run as a daemon in it's final form
 * The daemon acts as a server listening on a port for client requests
 * It also periodically checks for expired timers of jobs in the queue
 * If a job has expired, action is taken and job is removed from queue
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define LISTENING_PORT 51515
#define BACKLOG	2
#define MESSAGE_BUFFER_SIZE 1024

void serv_addr_init(struct sockaddr_in *serv_addr_ptr){
    serv_addr_ptr->sin_family = AF_INET;
    serv_addr_ptr->sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr_ptr->sin_port = htons(LISTENING_PORT);
}

int main(int argc, char *argv[]){
	int listen_fd = 0, conn_fd = 0;
	struct sockaddr_in serv_addr;
	char buffer[MESSAGE_BUFFER_SIZE];

	memset(&serv_addr, 0, sizeof(serv_addr));
	memset(buffer, 0, sizeof(buffer));
	serv_addr_init(&serv_addr);
	
	if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stdout, "ERROR: %s\n", strerror(errno));
		return 1;
	}
	if(bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
		fprintf(stdout, "ERROR: %s\n", strerror(errno));
		return 1;
	}
	if(listen(listen_fd, BACKLOG) == -1){
		fprintf(stdout, "ERROR: %s\n", strerror(errno));
		return 1;
	}

	conn_fd = accept(listen_fd, (struct sockaddr*)NULL, NULL);
	if(conn_fd <= 0){
		fprintf(stdout, "ERROR: %s\n", strerror(errno));
		return 1;
	}
	while(1){
		if(read(conn_fd, buffer, MESSAGE_BUFFER_SIZE) == 0){
			puts("Connection terminated by client");
			puts("Exiting..");
			close(conn_fd);
			break;
		}
		fprintf(stdout, buffer);
		memset(buffer, 0, sizeof(buffer));
	}
	close(listen_fd);
	return 0;
}