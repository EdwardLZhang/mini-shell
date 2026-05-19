#include <stdlib.h>
#include <stdio.h>

#include "execute.h"

#ifndef __JOB_HANDLER_H__
#define __JOB_HANDLER_H__

#define MAX_JOBS 50

typedef enum {
  FOREGROUND,
  BACKGROUND,
  STOPPED,
  INACTIVE
} State;

typedef struct {
  pid_t pid;
  State state;
  int order;
} Job;

/**
 * Clears all jobs
 * 
 * @param jobs array of Job structs
 * @param n size of job list
 */
void cleanup_jobs(Job* jobs, size_t n);

/**
 * Initializes the job list
 * 
 * @param jobs the list of jobs
 * @param n the length of the job list
 */
void initialize_jobs(Job* jobs, size_t n);

/**
 * Runs an input that requires job handles.
 * 
 * @param jobs the list of jobs
 * @param lexemes list of lexemes
 * @param n the number of lexemes
 */
// void handle_jobs(Job* jobs, char** lexemes, size_t n);

/**
 * Finds the index of the first vacant job slot
 * 
 * @param jobs the jobs
 * @param n length of the jobs list
 * @return index of the first vacand job, or n 
 */
size_t first_vacant_job(Job* jobs, size_t n);

// void print_jobs();

/**
 * Finds the PID of the foreground process
 * 
 * @param jobs the jobs
 * @return the PID
 */
pid_t get_fg_pid(Job* jobs);

/**
 * Returns a pointer to the job running in the foreground
 * 
 * @param jobs all the jobs
 * @return pointer to the foreground Job struct, or NULL if not found
 */
Job* get_fg_job(Job* jobs);

/**
 * Returns a pointer to the job either backgrounded or stopped with the
 * highest order, i.e. most recent.
 * Also resets orders for foreground/inactive jobs
 * 
 * @param jobs all the jobs
 * @return pointer to the most recently backgrounded/stopped job, or NULL if none
 */
Job* get_highest_order_job(Job* jobs);

/**
 * Returns a pointer to the stopped job with the highest order.
 * Resets orders for foreground/inactive jobs
 * 
 * @param jobs all the jobs
 * @return pointer to the most recently stopped job, or NULL if none
 */
Job* get_highest_stopped_job(Job* jobs);

#endif
