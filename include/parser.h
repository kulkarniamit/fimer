#ifndef _PARSER_H_
#define _PARSER_H_

#include "job.h"
#define DEFAULT_TIME_UNIT 'm'
#define MESSAGE_SEPARATOR ":"

char get_duration_unit(char *received_duration);
unsigned int get_duration_seconds(unsigned int duration, char unit);
struct message* get_parsed_message(char *received_message);
#endif
