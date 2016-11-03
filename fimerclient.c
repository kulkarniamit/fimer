/**
 * @file fimerclient.c
 * @author Amit Kulkarni
 * @date 3 Nov 2016
 * @brief Despatch jobs to server using sockets
 *
 * Sanitize the inputs, validate the input, construct
 * a server readable message and despatch the job to 
 * server
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>

#define SERVER_PORT 51515
#define BACKLOG	2
#define MESSAGE_BUFFER_SIZE 1024
#define SERVER_ADDRESS "127.0.0.1"

void serv_addr_init(struct sockaddr_in *serv_addr_ptr){
	serv_addr_ptr->sin_family = AF_INET;
	if(inet_pton(AF_INET, SERVER_ADDRESS, &serv_addr_ptr->sin_addr) <= 0){
		fprintf(stdout, "%s: %s\n", __FUNCTION__, strerror(errno));
		exit(1);
	}
	serv_addr_ptr->sin_port = htons(SERVER_PORT);
}

int main(int argc, char *argv[]){
	int sock_fd = 0;
	struct sockaddr_in serv_addr;
	char buffer[MESSAGE_BUFFER_SIZE];
	
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
	return 0;
}
