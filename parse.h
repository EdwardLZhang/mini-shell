#include <stdlib.h>

#ifndef __PARSE_H__
#define __PARSE_H__

char** parse(char* str, size_t* n);
void free_lexemes(char** lexemes, size_t n);
void print_lexemes(char** lexemes, size_t n);

/**
 * Determines if a char is an element of a list of chars
 * 
 * @param key the char to check
 * @param arr the char array to check against
 * @param len the length of the char array
 * @return 1 if in, 0 if not
 */
int char_element_of(char key, char* arr, size_t len);

/**
 * Determines if a string is an element of a list of strings
 * 
 * @param key the string to check
 * @param arr the string array to check against
 * @param len the length of the string array
 * @param index return address for the index of the string, if found
 * @return 1 if found, 0 if not
 */
int str_element_of(char* key, char** arr, size_t len, size_t* index);

#endif
