#include <ctype.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include "include/job.h"
#include "include/parser.h"
#include "include/utilities.h"

char get_duration_unit(char *received_duration)
{
    while(!isalpha(*received_duration) && *received_duration != '\0'){
        received_duration++;
    }
    return (*received_duration == '\0')?
            DEFAULT_TIME_UNIT:
            *received_duration;
}

unsigned int get_duration_seconds(unsigned int duration, char unit)
{
    unsigned int duration_in_seconds;
    switch(unit){
        case 's':   duration_in_seconds = duration;
                    break;
        case 'm':   duration_in_seconds = MINUTES_TO_SECONDS(duration);
                    break;
        case 'h':   duration_in_seconds = HOURS_TO_SECONDS(duration);
                    break;
        case 'd':   duration_in_seconds = DAYS_TO_SECONDS(duration);
                    break;
        case 'M':   duration_in_seconds = MONTHS_TO_SECONDS(duration);
                    break;
        case 'w':   duration_in_seconds = WEEKS_TO_SECONDS(duration);
                    break;
        case 'y':   duration_in_seconds = YEARS_TO_SECONDS(duration);
                    break;
        default:    exit(EXIT_FAILURE);
    }
    return duration_in_seconds;
}

struct message* get_parsed_message(char *received_message)
{
	struct message* parsed_message = malloc(sizeof(struct message));
	char *received_duration, *received_opcode;
	char *saveptr = NULL;
	unsigned int duration;
	char duration_unit;

    /*
        Well, strtok_r() has been generous enough.
        All we have to do it just pass char * pointer and it will
        take care of the rest of the allocation job 
	*/

    parsed_message->received_filepath = strtok_r(received_message, 
												 MESSAGE_SEPARATOR, 
												 &saveptr);
    received_duration = strtok_r(NULL, 
												 MESSAGE_SEPARATOR, 
												 &saveptr);
    received_opcode   = strtok_r(NULL,
												 MESSAGE_SEPARATOR, 
												 &saveptr);
    parsed_message->received_parameters = saveptr;

    parsed_message->opcode   = (unsigned int)strtol(
												received_opcode, 
												NULL, 
												10);
    duration = (unsigned int)strtol(received_duration, 
									NULL, 
									10);
    duration_unit = get_duration_unit(received_duration);
    parsed_message->expiry_time = get_duration_seconds(duration, duration_unit);
	return parsed_message;
}
