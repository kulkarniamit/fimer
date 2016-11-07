#ifndef _JOB_H_
#define _JOB_H_

struct job_duration
{
    char unit;
    unsigned int time;
};

struct job_data
{
    unsigned int opcode;
    char *filepath;
    char *params;
    void (*job_worker)(char *);
};

struct job
{
    unsigned int delete:1;
    struct job_duration *duration;
    struct job_data *data;
    struct job *next;
};

#endif
