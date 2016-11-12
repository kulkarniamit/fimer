#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
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
        syslog(LOG_ERR, "File permissions could not be changed for %s",
               filepath);
    }
}

