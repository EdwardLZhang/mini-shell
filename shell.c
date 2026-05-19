// Implement your shell in this source file.
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <signal.h>

#include "parse.h"
#include "execute.h"
#include "shell.h"
#include "job_handler.h"

Job all_jobs[MAX_JOBS];
int TERMINATED = 0;
LList* cmd_llist;

void signal_ctrl_z(int signum)
{
  
  // pid_t fg_pid = get_fg_pid(all_jobs);
  // printf("signal stop pid %d\n", (int) fg_pid);
  // if (fg_pid == 0)
  Job* fg_job = get_fg_job(all_jobs);
  if (!fg_job)
  {
    return;
  }
  
  // killpg(fg_pid, SIGTSTP);
  Job* max_order_job = get_highest_order_job(all_jobs);
  kill(fg_job->pid, SIGTSTP);
  killpg(fg_job->pid, SIGTSTP);
  if (!max_order_job)
  {
    fg_job->order = 1;
  }
  else
  {
    fg_job->order = max_order_job->order + 1;
  }
  fg_job->state = STOPPED;
}

void signal_ctrl_c(int signum)
{
  // TERMINATED = 1;
  // pid_t fg_pid = get_fg_pid(all_jobs);
  // if (fg_pid != 0)
  // {
  //   kill(fg_pid, SIGINT);
  //   killpg(get_fg_pid(all_jobs), SIGINT);
  // }
  printf("\nmini-shell terminated\n");
  cleanup_jobs(all_jobs, MAX_JOBS);
  if (cmd_llist)
  {
    Node* curr = cmd_llist->head;
    Node* temp;
    while (1)
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
    free(cmd_llist);
  }
  exit(0);
} 

int main(int argc, char** argv)
{
  signal(SIGTSTP, signal_ctrl_z);
  signal(SIGINT, signal_ctrl_c);

  initialize_jobs(all_jobs, MAX_JOBS);

  // Please leave in this line as the first statement in your program.
  alarm(120); // This will terminate your shell after 120 seconds,
              // and is useful in the case that you accidently create a 'fork bomb'

  char line[MAX_BUFFER_SIZE];

  char* text_line;
  // Job jobs[MAX_JOBS];
  // size_t n_jobs;

  cmd_llist = malloc(sizeof(LList));
  cmd_llist->head = NULL;
  cmd_llist->tail = NULL;

  while (1)
  {
    if (TERMINATED)
    {
      printf("mini-shell terminated\n");
      cleanup_jobs(all_jobs, MAX_JOBS);
      if (cmd_llist)
      {
        Node* curr = cmd_llist->head;
        Node* temp;
        while (1)
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
        free(cmd_llist);
      }
      return 0;
    }
    printf("mini-shell>");
    fgets(line, MAX_BUFFER_SIZE, stdin);
    
    
    size_t n;
    char** lexemes = parse(line, &n);

    if (n > 0 && strcmp("exit", lexemes[0]) == 0)
    {
      free_lexemes(lexemes, n);
      cleanup_jobs(all_jobs, MAX_JOBS);
      if (cmd_llist)
      {
        Node* curr = cmd_llist->head;
        Node* temp;
        while (1)
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
        free(cmd_llist);
      }
      return 0;
    }

    // else if (n > 0 && strcmp("jobs", lexemes[0]) == 0)
    // {
    //   free_lexemes(lexemes, n);
      
    // }

    size_t i;
    //first, check to see if any of ||, &&, ;, | are present;
    //otherwise must invoke the job handler

    //called the unemployed flag because if it's true, then 
    //i don't have to handle jobs. get it? ha ha
    // int unemployed_flag = 1;
    // for (i = 0; i < n; i++)
    // {
    //   if (strcmp(lexemes[i], ";") == 0
    //     || strcmp(lexemes[i], "||") == 0
    //     || strcmp(lexemes[i], "&&") == 0
    //     || strcmp(lexemes[i], "|") == 0)
    //   {
    //     unemployed_flag = 1;
    //     break;
    //   }
    // }

    // if (unemployed_flag)
    // {
    size_t no_amp_start = 0;
    size_t j;
    for (j = 0; j < n; j++)
    {
      if (strcmp(lexemes[j], "&") == 0)
      {
        // printf("& at index %zu\n", j);
        size_t no_semi_start = no_amp_start;
        for (i = no_amp_start; i < j; i++)
        {
          if (strcmp(lexemes[i], ";") == 0)
          {
            execute_no_semi(all_jobs, lexemes, no_semi_start, i, n, 1, cmd_llist);
            no_semi_start = i + 1;
          }
        }
        execute_no_semi(all_jobs, lexemes, no_semi_start, j, n, 1, cmd_llist);
        no_amp_start = j + 1;
      }
    }

    size_t no_semi_start = no_amp_start;
    for (i = no_amp_start; i < n; i++)
    {
      if (strcmp(lexemes[i], ";") == 0)
      {
        execute_no_semi(all_jobs, lexemes, no_semi_start, i, n, 0, cmd_llist);
        no_semi_start = i + 1;
      }
    }
    execute_no_semi(all_jobs, lexemes, no_semi_start, n, n, 0, cmd_llist);
    // }

    // else
    // {
    //   handle_jobs(all_jobs, lexemes, n);
    // }

    // printf("%zu\n", n);
    // print_lexemes(lexemes, n);
    // executefg(lexemes, 0, n);
    // printf("%d\n", status);

    // check_path(lexemes[0], NULL);

    free_lexemes(lexemes, n);

    text_line = malloc(MAX_BUFFER_SIZE * sizeof(char));
    strcpy(text_line, line);

    Node* entry = malloc(sizeof(Node));
    entry->text = text_line;
    entry->next = NULL;

    if (!cmd_llist->head)
    {
      cmd_llist->head = entry;
      cmd_llist->tail = entry;
    }
    else
    {
      cmd_llist->tail->next = entry;
      cmd_llist->tail = entry;
    }
  }

  return 0;
}


