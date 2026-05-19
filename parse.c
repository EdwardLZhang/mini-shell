#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

// What non-alphanumeric commands are interesting?
// | || && < > << >> ;

#define NUM_I_CHARS 5
#define NUM_D_CHARS 4

char interesting_chars[NUM_I_CHARS] = {'|', '&', '<', '>', ';'};
char double_chars[NUM_D_CHARS] = {'|', '&', '<', '>'};


int char_element_of(char key, char* arr, size_t len)
{
  size_t i;
  for (i = 0; i < len; i++)
  {
    if (key == arr[i])
    {
      return 1;
    }
  }
  return 0;
}

int str_element_of(char* key, char** arr, size_t len, size_t* index)
{
  if (!key || !arr || len == 0)
  {
    return 0;
  }
  size_t i;
  for (i = 0; i < len; i++)
  {
    if (strcmp(key, arr[i]) == 0)
    {
      *index = i;
      return 1;
    }
  }
  return 0;
}

/**
 * Determines if a state change occurs. A state change is when it's time
 * to start tracking a new lexeme.
 * 
 * @param left the left char
 * @param right the right char
 * @param special_length pointer to special character lexeme length
 * @return 1 if state change occurs, 0 otherwise
 */
int state_change(char left, char right, size_t* special_length, int* quote_flag)
{
    if (right == '\0')
    {
        return 1;
    }
    
    if (*quote_flag == 1)
    {
        if (right == '\'')
        {
            *quote_flag = 0;
            return 0;
        }
        else
        {
            return 0;
        }
    }
    if (*quote_flag == 2)
    {
        if (right == '\"')
        {
            *quote_flag = 0;
            return 0;
        }
        else
        {
            return 0;
        }
    }
    
    if (isspace(right))
    {
        return 1;
    }
    if (right == '\'')
    {
        *quote_flag = 1;
        return 1;
    }
    if (right == '\"')
    {
        *quote_flag = 2;
        return 1;
    }
    if (isspace(left))
    {
        return 1;
    }
    if (char_element_of(left, interesting_chars, NUM_I_CHARS))
    {
        if (*special_length >= 1)
        {
            return 1;
        }
        if (char_element_of(left, double_chars, NUM_D_CHARS))
        {
            if (left == right)
            {
                (*special_length)++;
                return 0;
            }
            else
            {
                return 1;
            }
        }
        else
        {
            return 1;
        }
    }
    if (char_element_of(right, interesting_chars, NUM_I_CHARS))
    {
        return 1;
    }
    return 0;
}

/**
 * Copies a substring to a heap-allocated string
 * 
 * @param str the string
 * @param start the index of the first character of the substring
 * @param end the index of the stopping point of the substring; not to be included
 * @return a pointer to the substring, or NULL on failure
 */
char* copy_substring(char* str, size_t start, size_t end)
{
    if (!str)
    {
        return NULL;
    }
    if (end < start)
    {
        char* ret = malloc(sizeof(char));
        ret[0] = '\0';
        return ret;
    }
    size_t len = end - start;
    char* substr = malloc((len + 1) * sizeof(char));
    if (!substr)
    {
        return NULL;
    }
    for (size_t i = start; i < end; i++)
    {
        substr[i - start] = str[i];
    }
    substr[len] = '\0';
    return substr;
}

char** parse(char* str, size_t* n)
{
    size_t i;
    size_t special_length = 0;
    int quote_flag;
    if (str[0] == '\'')
    {
        quote_flag = 1;
    }
    else if (str[0] == '\"')
    {
        quote_flag = 2;
    }
    else
    {
        quote_flag = 0;
    }
    size_t len = strlen(str);
    size_t lexeme_start = 0;
    size_t n_lexemes = 0;
    for (i = 0; i < len; i++)
    {
        if (state_change(str[i], str[i + 1], &special_length, &quote_flag))
        {
            if (isspace(str[i]))
            {
                special_length = 0;
                continue;
            }
            special_length = 0;
            n_lexemes++;
        }
    }
    char** lexemes = malloc(n_lexemes * sizeof(char*));
    if (!lexemes)
    {
        return NULL;
    }
    size_t lexemes_copied = 0;
    if (str[0] == '\'')
    {
        quote_flag = 1;
    }
    else if (str[0] == '\"')
    {
        quote_flag = 2;
    }
    else
    {
        quote_flag = 0;
    }
    special_length = 0;
    for (i = 0; i < len; i++)
    {
        if (state_change(str[i], str[i + 1], &special_length, &quote_flag))
        {
            if (isspace(str[i]))
            {
                special_length = 0;
                lexeme_start = i + 1;
            }
            else
            {
                special_length = 0;
                size_t end = i + 1;
                if (str[lexeme_start] == '\'' || str[lexeme_start] == '\"')
                {
                    lexeme_start++;
                    end--;
                }
                char* lexeme = copy_substring(str, lexeme_start, end);
                lexeme_start = i + 1;
                if (!lexeme)
                {
                    for (size_t j = 0; j < lexemes_copied; j++)
                    {
                        free(lexemes[j]);
                    }
                    free(lexemes);
                }
                lexemes[lexemes_copied++] = lexeme;
            }
        }
    }
    *n = n_lexemes;
    return lexemes;
}

/**
 * Frees a char* array and all its elements
 * 
 * @param lexemes the char* array
 * @param n the number of elements
 */
void free_lexemes(char** lexemes, size_t n)
{
  for (size_t i = 0; i < n; i++)
  {
    free(lexemes[i]);
  }
  free(lexemes);
}

/**
 * Prints lexemes duh
 * 
 * @param lexemes char* array of lexemes
 * @param n number of lexemes
 */
void print_lexemes(char** lexemes, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        printf("%s\n", lexemes[i]);
    }
}
