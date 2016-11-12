#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "include/operations.h"

void process_chmod(char *filepath, char *params)
{
    int i;
    char *savedptr = NULL;
    /* params is of the form: "0444:"   */
    /*  params is ':' separated, get relevant information   */
    char *permissions = strtok_r(params, MESSAGE_SEPARATOR, &savedptr);
    i = strtol(permissions, 0, 8);
    if (chmod (filepath,i) < 0){
        syslog(LOG_ERR, "File permissions could not be changed for %s\n",
               filepath);
    }
}

void process_rm(char *filepath, char *params)
{
	if(unlink(filepath)){
        syslog(LOG_ERR, "%s could not be deleted\n", filepath);
	}
}

void process_truncate(char *filepath, char *params)
{
	if(truncate(filepath, 0)){
        syslog(LOG_ERR, "%s could not be truncated\n", filepath);
	}
}

void process_move(char *filepath, char *params)
{
	char *savedptr = NULL;
	const char *newpath = strtok_r(params, MESSAGE_SEPARATOR, &savedptr);
	if(rename(filepath, newpath)){
        syslog(LOG_ERR, "%s could not be moved\n", filepath);
	}
}
