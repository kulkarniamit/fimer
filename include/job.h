#ifndef _JOB_H_
#define _JOB_H_

/*	Well, this surely looks cute, but I don't think
	we actually need this info going further.
	All the information that is requires is the 
	expiry time	*/
struct job_duration
{
    char unit;
    unsigned int time;
};

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
//    struct job_duration *duration;
	struct timespec job_expiry;
    struct job *next;
};

#endif
