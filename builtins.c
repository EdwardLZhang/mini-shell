#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
#include <signal.h>

#include "builtins.h"
#include "parse.h"
#include "job_handler.h"
#include "shell.h"

// this is identical to the one in execute.c, but i don't really want to
// expose that. besides it throws compiler errors otherwise
char* shell_builtin_cmds2[NUM_SHELL_BUILTINS] = {"cd", "exit", "help", "fg", "jobs", "bg", "history"};

int builtin_exit(Job* jobs, char** lexemes, size_t start, size_t end, size_t n, LList* ll)
{
  free_lexemes(lexemes, n);
  cleanup_jobs(jobs, MAX_JOBS);
  if (ll)
  {
    Node* curr = ll->head;
    Node* temp;
    while(1)
    {
      if (curr)
      {
        free(curr->text);
        temp = curr;
        curr = curr->next;
        free(temp);
      }
      else
      {
        break;
      }
    }
    free(ll);
  }
  printf("exit\n");
  exit(0);
  return 0;
}

int builtin_cd(Job* jobs, char** lexemes, size_t start, size_t end, size_t n, LList* ll)
{
  char* path;
  if (start >= end - 1)
  {
    path = "";
  }
  else
  {
    path = lexemes[start + 1];
  }
  int result = chdir(path);
  if (result)
  {
    fprintf(stderr, "mini-shell: cd: %s: No such file or directory\n", path);
  }
  return result;
}

int builtin_help(Job* jobs, char** lexemes, size_t start, size_t end, size_t n, LList* ll)
{
  size_t i;
  for (i = 0; i < NUM_SHELL_BUILTINS; i++)
  {
    printf("%s\n", shell_builtin_cmds2[i]);
  }
  return 0;
}

int builtin_fg(Job* jobs, char** lexemes, size_t start, size_t end, size_t n, LList* ll)
{
  // command should be fg [n] ...
  if (start >= end)
  {
    return 1;
  }
  pid_t gpid;
  // size_t i;
  int jnum;
  if (start == end - 1)
  {
    //foreground the first process
    // for (i = 0; i < MAX_JOBS; i++)
    // {
    //   if (jobs[i].state == BACKGROUND)
    //   {
    //     jnum = (int) i + 1;
    //     break;
    //   }
    // }
    // if (i == MAX_JOBS)
    // {
    //   fprintf(stderr, "minishell: fg: current: no such job\n");
    //   return 1;
    // }
    // gpid = jobs[i].pid;
    Job* job = get_highest_order_job(jobs);
    if (!job)
    {
      fprintf(stderr, "minishell: fg: current: no such job\n");
      return 1;
    }
    job->state = FOREGROUND;
    // printf("%d %d\n", job->pid, job->state);
    killpg(job->pid, SIGCONT);
    int child_status;
    waitpid(-job->pid, &child_status, WUNTRACED);
    if (WIFSTOPPED(child_status))
    {
      // printf("\n%d %d Stopped\n", jnum, jobs[jnum - 1].pid);
      job->state = STOPPED;
    }
    else
    {
      // printf("\n%d %d Done\n", jnum, jobs[jnum - 1].pid);
      job->state = INACTIVE;
    }
    return child_status;
  }
  else
  {
    // printf("%s\n", lexemes[start + 1]);
    jnum = atoi(lexemes[start + 1]);
    if (jnum < 1 || jnum > MAX_JOBS)
    {
      fprintf(stderr, "minishell: fg: %d: no such job\n", jnum);
      return 1;
    }
    if (!(jobs[jnum - 1].state == BACKGROUND || jobs[jnum - 1].state == STOPPED))
    {
      fprintf(stderr, "minishell: fg: %d: no such job\n", jnum);
      return 1;
    }
    gpid = jobs[jnum - 1].pid;
  }
  jobs[jnum - 1].state = FOREGROUND;
  // tcsetpgrp(STDIN_FILENO, gpid);
  killpg(gpid, SIGCONT);
  int child_status;
  // killpg(gpid, SIGTSTP);
  waitpid(-gpid, &child_status, WUNTRACED);
  if (WIFSTOPPED(child_status))
  {
    // printf("\n%d %d Stopped\n", jnum, jobs[jnum - 1].pid);
    jobs[jnum - 1].state = STOPPED;
  }
  else
  {
    // printf("\n%d %d Done\n", jnum, jobs[jnum - 1].pid);
    jobs[jnum - 1].state = INACTIVE;
  }
  return child_status;
}

int builtin_bg(Job* jobs, char** lexemes, size_t start, size_t end, size_t n, LList* ll)
{
  // command should be fg [n] ...
  if (start >= end)
  {
    return 1;
  }
  pid_t gpid;
  // size_t i;
  int jnum;
  if (start == end - 1)
  {
    //resume the first process
    // for (i = 0; i < MAX_JOBS; i++)
    // {
    //   if (jobs[i].state == STOPPED)
    //   {
    //     jnum = (int) i + 1;
    //     break;
    //   }
    // }
    // if (i == MAX_JOBS)
    // {
    //   fprintf(stderr, "minishell: bg: current: no such job\n");
    //   return 1;
    // }
    // gpid = jobs[i].pid;
    Job* job = get_highest_stopped_job(jobs);
    if (!job)
    {
      fprintf(stderr, "minishell: bg: current: no such job\n");
      return 1;
    }
    job->state = BACKGROUND;
    return killpg(job->pid, SIGCONT);
  }
  else
  {
    jnum = atoi(lexemes[start + 1]);
    if (jnum < 1 || jnum > MAX_JOBS)
    {
      fprintf(stderr, "minishell: bg: %d: no such job\n", jnum);
      return 1;
    }
    if (jobs[jnum - 1].state == BACKGROUND)
    {
      fprintf(stderr, "minishell: bg: job %d already running in background\n", jnum);
      return 1;
    }
    if (jobs[jnum - 1].state != STOPPED)
    {
      fprintf(stderr, "minishell: bg: %d: no such job\n", jnum);
      return 1;
    }
    // if (jobs[jnum - 1].state != BACKGROUND)
    // {
    //   fprintf(stderr, "minishell: bg: %d: no such job\n", jnum);
    // }
    gpid = jobs[jnum - 1].pid;
  }
  jobs[jnum - 1].state = BACKGROUND;
  return killpg(gpid, SIGCONT);
}

int builtin_jobs(Job* jobs, char** lexemes, size_t start, size_t end, size_t n, LList* ll)
{
  size_t i;
  int result;
  int child_status;
  for (i = 0; i < MAX_JOBS; i++)
  {
    if (jobs[i].state != INACTIVE)
    {
      result = waitpid(-jobs[i].pid, &child_status, WNOHANG);
      if (result)
      {
        jobs[i].state = INACTIVE;
      }
    }
  }
  for (i = 0; i < MAX_JOBS; i++)
  {
    if (jobs[i].state != INACTIVE)
    {
      char* state;
      if (jobs[i].state == FOREGROUND)
      {
        state = "Foreground";
      }
      else if (jobs[i].state == BACKGROUND)
      {
        state = "Running";
      }
      else
      {
        state = "Stopped";
      }
      printf("%zu %d %s\n", i + 1, (int) jobs[i].pid, state);
    }
  }
  return 0;
}

int builtin_history(Job* jobs, char** lexemes, size_t start, size_t end, size_t n, LList* ll)
{
  if (!ll)
  {
    return 1;
  }
  Node* curr = ll->head;
  while(1)
  {
    if (curr)
    {
      printf("%s", curr->text);
      curr = curr->next;
    }
    else
    {
      break;
    }
  }
  return 0;
}
