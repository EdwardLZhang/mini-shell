#include <signal.h>
#include <string.h>
#include <sys/types.h>

#include "job_handler.h"

void cleanup_jobs(Job* jobs, size_t n)
{
  size_t i;
  for (i = 0; i < n; i++)
  {
    if (jobs[i].state != INACTIVE)
    {
      // killpg(jobs[i].pid, SIGINT);
      killpg(jobs[i].pid, SIGINT);
      jobs[i].state = INACTIVE;
    }
  }
}

void initialize_jobs(Job* jobs, size_t n)
{
  size_t i;
  for (i = 0; i < n; i++)
  {
    jobs[i].state = INACTIVE;
    jobs[i].pid = 0;
    jobs[i].order = -1;
  }
}

size_t first_vacant_job(Job* jobs, size_t n)
{
  size_t i;
  for (i = 0; i < n; i++)
  {
    if (jobs[i].state == INACTIVE)
    {
      return i;
    }
  }
  return n;
}

pid_t get_fg_pid(Job* jobs)
{
  size_t i;
  for (i = 0; i < MAX_JOBS; i++)
  {
    if (jobs[i].state == FOREGROUND)
    {
      return jobs[i].pid;
    }
  }
  return 0;
}

Job* get_fg_job(Job* jobs)
{
  size_t i;
  for (i = 0; i < MAX_JOBS; i++)
  {
    if (jobs[i].state == FOREGROUND)
    {
      return &jobs[i];
    }
  }
  return NULL;
}

Job* get_highest_order_job(Job* jobs)
{
  int max_order_index = 0;
  size_t i;
  for (i = 0; i < MAX_JOBS; i++)
  {
    if (jobs[i].state == INACTIVE || jobs[i].state == FOREGROUND)
    {
      jobs[i].order = -1;
    }
    else
    {
      if (jobs[i].order > jobs[max_order_index].order)
      {
        max_order_index = i;
      }
    }
  }
  if (jobs[max_order_index].order < 0)
  {
    return NULL;
  }
  return &jobs[max_order_index];
}

Job* get_highest_stopped_job(Job* jobs)
{
  int max_order_index = -1;
  size_t i;
  for (i = 0; i < MAX_JOBS; i++)
  {
    if (jobs[i].state == INACTIVE || jobs[i].state == FOREGROUND)
    {
      jobs[i].order = -1;
    }
    else if (jobs[i].state == STOPPED)
    {
      if (max_order_index < 0)
      {
        max_order_index = i;
      }
      else if (jobs[i].order > jobs[max_order_index].order)
      {
        max_order_index = i;
      }
    }
  }
  if (max_order_index < 0 || jobs[max_order_index].order < 0)
  {
    return NULL;
  }
  return &jobs[max_order_index];
}
