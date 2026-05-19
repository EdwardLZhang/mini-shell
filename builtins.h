#ifndef __BUILTINS_H__
#define __BUILTINS_H__

#include "job_handler.h"
#include "shell.h"

/**
 * Exits the entire program. Frees lexemes in the process, and kills
 * all subprocesses.
 * 
 * @param jobs all the jobs running at the moment
 * @param start the first lexeme index (not really useful)
 * @param end the last lexeme index, exclusive (also not useful)
 * @param n number of lexemes total (useful for exit)
 * @return 0 on success, 1 on failure
 */
int builtin_exit(Job* jobs, char** lexemes, size_t start, size_t end, size_t n, LList* ll);

int builtin_cd(Job* jobs, char** lexemes, size_t start, size_t end, size_t n, LList* ll);

int builtin_help(Job* jobs, char** lexemes, size_t start, size_t end, size_t n, LList* ll);

int builtin_fg(Job* jobs, char** lexemes, size_t start, size_t end, size_t n, LList* ll);

int builtin_bg(Job* jobs, char** lexemes, size_t start, size_t end, size_t n, LList* ll);

int builtin_jobs(Job* jobs, char** lexemes, size_t start, size_t end, size_t n, LList* ll);

int builtin_history(Job* jobs, char** lexemes, size_t start, size_t end, size_t n, LList* ll);

#endif
