#ifndef _JOB_H_
#define _JOB_H_

struct job_data
{
	/* A struct that represents every kind of job */
	/* All members are applicable for every kind of job */
    unsigned int opcode;
    char *filepath;
    void (*job_worker)(char *, char *);
    char *params;	/* Should be processed individually */
};

struct job
{
    unsigned int delete:1;	/* Flag to keep track of expiry */
    struct job_data *data;
	struct timespec job_expiry;
    struct job *next;
};

#endif
