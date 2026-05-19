#include <stdlib.h>



#ifndef __EXECUTE_H__
#define __EXECUTE_H__

#include "job_handler.h"
#include "shell.h"

#define NUM_SHELL_BUILTINS 7

/**
 * Executes a sequence of lexemes as a command. Assumes no |, &&, ||, ;.
 * The job is run in the foreground.
 * 
 * Kind of deprecated, but it's simple so I still use it.
 * Real job handling is done through execute_foreground
 * and execute_background.
 * 
 * @param jobs all the jobs
 * @param lexemes list of all lexemes
 * @param start the index of the first lexeme to be run
 * @param end the index of the end of the sequence, exclusive
 * @param total_lexemes the total number of lexemes
 * @param bg_flag
 * @return 0 on success, 1 on failure
 */
int executefg(Job* jobs, char** lexemes, size_t start, size_t end, size_t total_lexemes, int bg_flag, LList* ll);

/**
 * Executes a command that is to be piped as output
 * 
 * @param fd the file descriptors of the pipe
 * @param lexemes all lexemes from line of input
 * @param start index of first lexeme in command
 * @param end index of last lexeme in command, exclusive
 * @return 0 on success, 1 on failure
 */
// int execute_piped(int fd[2], char** lexemes, size_t start, size_t end);

/**
 * Executes a command that does not have ;.
 * 
 * @param jobs the jobs
 * @param lexemes all lexemes from line of input
 * @param start index of first lexeme in command
 * @param end index of last lexeme in command, exclusive
 * @param total_lexemes
 * @param bg_flag 0 if fg, 1 if bg
 * @return 0 on success, 1 on failure
 */
int execute_no_semi(Job* jobs, char** lexemes, size_t start, size_t end, size_t total_lexemes, int bg_flag, LList* ll);

/**
 * Checks to see if command is in the path folder as an executable
 * 
 * @param ipath the path to the directory to check
 * @param command the command to check
 * @return 1 if found, 0 if not
 */
int check_path(char* ipath, char* command);

/**
 * Executes a command split by an &&
 * 
 * @param lexemes all the lexemes
 * @param start start of the sequence of lexemes to execute
 * @param splitter the index of the &&
 * @param end the end of the sequence exclusive
 * @return 0 on success, 1 on failure
 */
// int execute_and(char** lexemes, size_t start, size_t splitter, size_t end);

/**
 * Executes a command split by an ||
 * 
 * @param lexemes the lexemes
 * @param start you know what this is 
 * @param splitter why do i even explain it
 * @param end exclusive
 * @return 0 on success, 1 on failure
 */
// int execute_or(char** lexemes, size_t start, size_t splitter, size_t end);

/**
 * Handles a series of lexemes with no ;, ||, &&, but may contain |.
 * 
 * @param lexemes the lexemes
 * @param start includisve
 * @param end exclusive
 * @return 0 on success, 1 on failure
 */
// int handle_pipe(char** lexemes, size_t start, size_t end);

/**
 * Executes the last command of a chain of pipes, and pipes to stdout.
 * 
 * @param fd file descriptor for the pipe
 * @param lexemes
 * @param start inclusive
 * @param end exclusive
 * @return 0 on success, 1 on failure
 */
// int execute_pipe_final(int fd[2], char** lexemes, size_t start, size_t end);

/**
 * Handles pipes in a loop with a list of file descriptor pairs.
 * 
 * @param jobs the jobs
 * @param lexemes the lexemes
 * @param start inclusive
 * @param end inclusive
 * @param total_lexemes
 * @param bg_flag
 * @param head head of the history linked list
 * @return 0 on success, 1 on failure
 */
int handle_pipe2(Job* jobs, char** lexemes, size_t start, size_t end, size_t total_lexemes, int bg_flag, LList* ll);

/**
 * Gets a heap-allocated NULL-terminated string array of the lexemes of a command
 * The first element is always heap-allocated separate from the original
 * list of lexemes
 * 
 * Thus the caller is responsible for freeing the first element
 * as well as the whole list
 * 
 * @param lexemes the lexemes
 * @param start inclusive
 * @param end exclusive
 * @return the command lexemes, or NULL if failed
 */
char** get_cmd_lexemes(char** lexemes, size_t start, size_t end);

/**
 * Executes a program in the foreground, and tracks it with jobs
 * 
 * @param jobs list of jobs
 * @param lexemes the lexemes to execute
 * @param n the number of lexemes
 */
// int execute_foreground(Job* jobs, char** lexemes, size_t n);

/**
 * Executes a program in the background, and tracks it with jobs
 * 
 * @param jobs list of jobs
 * @param lexemes the lexemes to execute
 * @param n the number of lexemes, excluding the ampersand
 */
// int execute_background(Job* jobs, char** lexemes, size_t n);

#endif
