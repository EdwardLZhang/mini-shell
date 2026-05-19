#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "execute.h"
#include "shell.h"
#include "job_handler.h"
#include "parse.h"
#include "builtins.h"


char* shell_builtin_cmds[NUM_SHELL_BUILTINS] = {"cd", "exit", "help", "fg", "jobs", "bg", "history"};
int (*shell_builtins[NUM_SHELL_BUILTINS])(Job* jobs, char** lexemes, size_t start, size_t end, size_t n, LList* ll) = {
  builtin_cd, builtin_exit, builtin_help, builtin_fg, builtin_jobs, builtin_bg, builtin_history
};

/**
 * Checks if a char is in a string, but backwards
 * 
 * @param ch the character
 * @param str the string
 * @param index return address of the index, if found
 * @return 1 if found, 0 if not
 */
int char_element_of_index_bw(char ch, char* str, size_t* index)
{
  size_t len = strlen(str);
  size_t i;
  for (i = 0; i < len; i++)
  {
    if (ch == str[len - 1 - i])
    {
      *index = len - 1 - i;
      return 1;
    }
  }
  return 0;
}

// Code modified from
// https://faq.cprogramming.com/cgi-bin/smartfaq.cgi?answer=1046380353&id=1044780608
int check_path(char* ipath, char* command)
{
  size_t last_slash;
  int slash_exists = char_element_of_index_bw('/', command, &last_slash);
  // printf("slash checking done\n");
  char* path;
  if (slash_exists)
  {
    path = malloc((strlen(ipath) + strlen(command) + 2) * sizeof(char));
    strcpy(path, ipath);
    size_t lindex = strlen(ipath);
    size_t i;
    for (i = 0; i <= last_slash; i++)
    {
      path[lindex + i] = command[i];
    }
    command = &command[last_slash + 1];
  }
  else
  {
    path = ipath;
  }

  // printf("%s%s\n", path, command);

  DIR *d;
  struct dirent *dir;
  d = opendir(path);
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      // printf("%s\n", dir->d_name);
      if (strcmp(command, dir->d_name) == 0)
      {
        closedir(d);
        return 1;
      }
    }
    closedir(d);
  }
  if (slash_exists)
  {
    free(path);
  }
  return 0;
}

int execute_no_semi(Job* jobs, char** lexemes, size_t start, size_t end, size_t total_lexemes, int bg_flag, LList* ll)
{
  // now we gotta split on either || and &&

  if (start > end)
  {
    return 1;
  }

  if (start == end)
  {
    return 0;
  }
  size_t i;

  size_t cmd_start = start;
  int result;

  for (i = start; i < end; i++)
  {
    if (strcmp(lexemes[i], "||") == 0)
    {
      result = handle_pipe2(jobs, lexemes, cmd_start, i, total_lexemes, bg_flag, ll);
      cmd_start = i + 1;
      if (result == 0)
      {
        return 0;
      }
    }
    else if (strcmp(lexemes[i], "&&") == 0)
    {
      result = handle_pipe2(jobs, lexemes, cmd_start, i, total_lexemes, bg_flag, ll);
      cmd_start = i + 1;
      if (result != 0)
      {
        return result;
      }
    }
  }
  result = handle_pipe2(jobs, lexemes, cmd_start, end, total_lexemes, bg_flag, ll);
  return result;
}

int executefg(Job* jobs, char** lexemes, size_t start, size_t end, size_t total_lexemes, int bg_flag, LList* ll)
{
  size_t open_job = first_vacant_job(jobs, MAX_JOBS);
  if (open_job == MAX_JOBS)
  {
    fprintf(stderr, "Maximum number of jobs reached\n");
    return 1;
  }

  if (start > end)
  {
    return 1;
  }

  if (start == end)
  {
    return 0;
  }

  size_t bi_index;
  int is_builtin = str_element_of(lexemes[start], shell_builtin_cmds, NUM_SHELL_BUILTINS, &bi_index);
  if (is_builtin)
  {
    return shell_builtins[bi_index](jobs, lexemes, start, end, total_lexemes, ll);
  }

  // char* head = "";
  if (check_path(CURR, lexemes[start]))
  {
    // head = CURR;
  }
  else if (check_path(BIN, lexemes[start]))
  {
    // head = BIN;
  }
  else
  {
    fprintf(stderr, "Command not found--Did you mean something else?\n");
    return 1;
  }

  size_t n_lexemes = end - start;

  char** temp_lexemes = malloc((n_lexemes + 1) * sizeof(char*));

  size_t i;

  // char* command_head = malloc((strlen(lexemes[start]) + strlen(head) + 1) * sizeof(char));
  // command_head[0] = '\0';
  // strcpy(command_head, head);
  // strcat(command_head, lexemes[start]);

  // printf("%s\n", command_head);

  // temp_lexemes[0] = command_head;
  for (i = 0; i < n_lexemes; i++)
  {
    temp_lexemes[i] = lexemes[start + i];
  }
  temp_lexemes[n_lexemes] = NULL;

  int child_status;

  pid_t pid = fork();

  if (pid == 0)
  {
    // if (strcmp(head, BIN) == 0)
    // {
    //   setpgid(0, 0);
    // }
    setpgid(0, 0);
    // printf("child pid %d\n", (int) getpid());
    execvp(temp_lexemes[0], temp_lexemes);
    if (errno == ENOENT)
    {
      fprintf(stderr, "Command not found--Did you mean something else?\n");
    }
    // free(temp_lexemes[0]);
    // free(temp_lexemes);
    exit(1);
  }
  else
  {
    
    // tcsetpgrp(STDIN_FILENO, pid);
    // setpgid(pid, 0);
    
    // printf("pid %d\n", (int) pid);
    // printf("%d %d\n", jobs[open_job].state, (int) jobs[open_job].pid);
    // printf("%d active process\n", (int) get_fg_pid(jobs));
    // waitpid(getpgid(pid), &child_status, 0);
    if (!bg_flag)
    {
      jobs[open_job].state = FOREGROUND;
      // jobs[open_job].pid = getpgid(pid);
      jobs[open_job].pid = pid;
      waitpid(pid, &child_status, WUNTRACED);
      if (WIFSTOPPED(child_status))
      {
        // printf("\n%zu %d Stopped\n", open_job + 1, jobs[open_job].pid);
        Job* max_order_job = get_highest_order_job(jobs);
        if (!max_order_job)
        {
          jobs[open_job].order = 1;
        }
        else
        {
          jobs[open_job].order = max_order_job->order + 1;
        }
        // printf("STOPPED\n");
        
        jobs[open_job].state = STOPPED;
        // for (size_t z = 0; z < MAX_JOBS; z++)
        // {
        //   printf("%zu %d %d\n", z, jobs[z].pid, jobs[z].state);
        // }
      }
      else
      {
        jobs[open_job].state = INACTIVE;
      }
    }
    else
    {
      if (is_builtin)
      {
        waitpid(pid, &child_status, 0);
      }
      else
      {
        jobs[open_job].state = BACKGROUND;
        // jobs[open_job].pid = getpgid(pid);
        jobs[open_job].pid = pid;
        Job* max_order_job = get_highest_order_job(jobs);
        if (!max_order_job) {
          jobs[open_job].order = 1;
        } else {
          jobs[open_job].order = max_order_job->order + 1;
        }
        // printf("%zu %d Running\n", open_job + 1, pid);
        waitpid(pid, &child_status, WNOHANG);
        // printf("%d result\n", result);
        // if (result == pid)
        // {
        //   printf("INACTIVE\n");
        //   jobs[open_job].state = INACTIVE;
        // }
      }
    }
    // tcsetpgrp(STDIN_FILENO, getpid());
    // jobs[open_job].state = INACTIVE;
    // free(temp_lexemes[0]);
    free(temp_lexemes);
    return child_status;
  }
}

int handle_pipe2(Job* jobs, char** lexemes, size_t start, size_t end, size_t total_lexemes, int bg_flag, LList* ll)
{
  size_t open_job = first_vacant_job(jobs, MAX_JOBS);
  if (open_job == MAX_JOBS)
  {
    fprintf(stderr, "Maximum number of jobs reached\n");
    return 1;
  }

  if (start > end)
  {
    return 1;
  }

  if (start == end)
  {
    return 0;
  }

  // count the number of pipes
  size_t i;
  size_t n_pipes = 0;
  for (i = start; i < end; i++)
  {
    if (strcmp(lexemes[i], "|") == 0)
    {
      n_pipes++;
    }
  }

  if (n_pipes == 0)
  {
    return executefg(jobs, lexemes, start, end, total_lexemes, bg_flag, ll);
  }

  // if (n_pipes == 1)
  // {
  //   return handle_pipe(lexemes, start, end);
  // }

  int** fds = malloc(n_pipes * sizeof(int*));

  size_t cmd_index = 0;
  for (i = 0; i < n_pipes; i++)
  {
    int* fd = malloc(2 * sizeof(int));
    pipe(fd);
    fds[i] = fd;
  }

  pid_t* pids = malloc((n_pipes + 1) * sizeof(pid_t));
  size_t cmd_start = start;
  char** cmd_lexemes;
  pid_t pid;
  pid_t gpid = 0;
  for (i = start; i < end; i++)
  {
    //read until the pipe
    if (strcmp(lexemes[i], "|") == 0)
    {
      size_t bi_index;
      int is_builtin = str_element_of(lexemes[cmd_start], shell_builtin_cmds, NUM_SHELL_BUILTINS, &bi_index);
      if (is_builtin)
      {
        if (cmd_index == 0)
        {
          pid = fork();
          if (pid == 0) 
          {
            setpgid(0, 0);
            dup2(fds[cmd_index][1], STDOUT_FILENO);
            close(fds[cmd_index][0]);
            close(fds[cmd_index][1]);
            exit(shell_builtins[bi_index](jobs, lexemes, cmd_start, i, total_lexemes, ll));
          } 
          else 
          {
            gpid = pid;
            pids[cmd_index] = pid;
            close(fds[cmd_index][1]);
          }
        }
        else
        {
          pid = fork();
          if (pid == 0) 
          {
            setpgid(0, gpid);
            dup2(fds[cmd_index - 1][0], STDIN_FILENO);
            dup2(fds[cmd_index][1], STDOUT_FILENO);
            close(fds[cmd_index][0]);
            close(fds[cmd_index][1]);
            exit(shell_builtins[bi_index](jobs, lexemes, cmd_start, i, total_lexemes, ll));
          }
          else 
          {
            // waitpid(pid, &child_status, 0);
            pids[cmd_index] = pid;
            close(fds[cmd_index][1]);
          }
        }
      }
      else
      {
        cmd_lexemes = get_cmd_lexemes(lexemes, cmd_start, i);
        if (!cmd_lexemes)
        {
          // cmd_index++;
          // cmd_start = i + 1;
          // continue;
          return 1;
        }
        else if (cmd_index == 0)
        {
          pid = fork();
          if (pid == 0) 
          {
            setpgid(0, 0);
            dup2(fds[cmd_index][1], STDOUT_FILENO);
            close(fds[cmd_index][0]);
            close(fds[cmd_index][1]);
            execvp(cmd_lexemes[0], cmd_lexemes); 
            if (errno == ENOENT)
            {
              fprintf(stderr, "Command not found--Did you mean something else?\n");
            }
            free(cmd_lexemes);
            exit(1);
          } 
          else 
          {
            gpid = pid;
            pids[cmd_index] = pid;
            close(fds[cmd_index][1]);
          }
        }
        else
        {
          pid = fork();
          if (pid == 0) 
          {
            setpgid(0, gpid);
            dup2(fds[cmd_index - 1][0], STDIN_FILENO);
            dup2(fds[cmd_index][1], STDOUT_FILENO);
            close(fds[cmd_index][0]);
            close(fds[cmd_index][1]);
            execvp(cmd_lexemes[0], cmd_lexemes);
            if (errno == ENOENT)
            {
              fprintf(stderr, "Command not found--Did you mean something else?\n");
            }
            free(cmd_lexemes);
            exit(1);       
          }
          else 
          {
            // waitpid(pid, &child_status, 0);
            pids[cmd_index] = pid;
            close(fds[cmd_index][1]);
          }
        }
        // free(cmd_lexemes[0]);
        free(cmd_lexemes);
      }
      cmd_index++;
      cmd_start = i + 1;
    }
  }
  
  size_t bi_index;
  // printf("%s\n", lexemes[cmd_start]);
  if (lexemes[cmd_start])
  {
    
    int is_builtin = str_element_of(lexemes[cmd_start], shell_builtin_cmds, NUM_SHELL_BUILTINS, &bi_index);
    if (is_builtin)
    {
      pid = fork();
      if (pid == 0) 
      {
        setpgid(0, gpid);
        dup2(fds[cmd_index - 1][0], STDIN_FILENO);
        exit(shell_builtins[bi_index](jobs, lexemes, cmd_start, i, total_lexemes, ll));
      }
      else
      {
        pids[cmd_index] = pid;
        int child_status;
        jobs[open_job].state = FOREGROUND;
        jobs[open_job].pid = gpid;
        for (size_t j = 0; j <= n_pipes; j++)
        {
          waitpid(pids[j], &child_status, WUNTRACED);
        }
        if (WIFSTOPPED(child_status))
        {
          jobs[open_job].state = STOPPED;
        }
        else
        {
          jobs[open_job].state = INACTIVE;
        }
        // waitpid(pid, &child_status, 0);
      }
    }
    else
    {
      cmd_lexemes = get_cmd_lexemes(lexemes, cmd_start, i);
      if (!cmd_lexemes)
      {
        return 1;
      }
      pid = fork();
      if (pid == 0) 
      {
        pids[cmd_index] = pid;
        setpgid(0, gpid);
        dup2(fds[cmd_index - 1][0], STDIN_FILENO);
        execvp(cmd_lexemes[0], cmd_lexemes);
        if (errno == ENOENT)
        {
          fprintf(stderr, "Command not found--Did you mean something else?\n");
        }
        free(cmd_lexemes);
        exit(1);
      }
      else
      {
        // waitpid(pid, &child_status, 0); // this is probably unnecessary
      }
      

      // free(cmd_lexemes[0]);
      free(cmd_lexemes);
    }
  }
  
  int child_status = 0;
  for (size_t j = 0; j <= n_pipes; j++)
  {
    waitpid(pids[j], &child_status, 0);
  }
  waitpid(-gpid, &child_status, 0);

  for (i = 0; i < n_pipes; i++)
  {
    free(fds[i]);
  }
  free(pids);
  free(fds);
  return 0;
}





char** get_cmd_lexemes(char** lexemes, size_t start, size_t end)
{
  if (start > end)
  {
    return NULL;
  }

  if (start == end)
  {
    char** t_lexemes = malloc(sizeof(char*));
    // t_lexemes[0] = malloc(sizeof(char*));
    // t_lexemes[0][0] = '\0';
    t_lexemes[0] = NULL;
    return t_lexemes;
  }

  // char* head = "";
  if (check_path(CURR, lexemes[start]))
  {
    // head = CURR;
  }
  else if (check_path(BIN, lexemes[start]))
  {
    // head = BIN;
  }
  else
  {
    fprintf(stderr, "Command not found--Did you mean something else?\n");
    return NULL;
  }

  size_t n_lexemes = end - start;

  char** temp_lexemes = malloc((n_lexemes + 1) * sizeof(char*));

  size_t i;

  // char* command_head = malloc((strlen(lexemes[start]) + strlen(head) + 1) * sizeof(char));
  // command_head[0] = '\0';
  // strcpy(command_head, head);
  // strcat(command_head, lexemes[start]);

  // printf("%s\n", command_head);

  // temp_lexemes[0] = command_head;
  for (i = 0; i < n_lexemes; i++)
  {
    temp_lexemes[i] = lexemes[start + i];
  }
  temp_lexemes[n_lexemes] = NULL;
  return temp_lexemes;
}
