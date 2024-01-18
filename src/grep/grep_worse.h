#ifndef GREP_H
#define GREP_H

#define _POSIX_C_SOURCE 200809L

#define iflag flags[0]
#define vflag flags[1]
#define cflag flags[2]
#define lflag flags[3]
#define nflag flags[4]
#define hflag flags[5]
#define sflag flags[6]
#define oflag flags[7]
#define fflag flags[8]
#define eflag flags[9]

#include <regex.h>
#include <stdlib.h>

#include "../common/common.h"

typedef struct Stuff {
  // a.k.a. files_count
  int argc;
  int flags[10];
  int file_match;
  int line_count;
  int match_count;
  int print_all;
} Config;

void add_match(Config Options, const char *match, const char *pattern,
               int *matches, int *i, char *buf);
int add_pattern(char **pattern, const char *str);
int add_pattern_from_file(char **pattern, const char *str, Config *Options);
void comp_grep(char *argv[], Config *Options, char *pattern);
void conditional_print(const char *filename, Config Options);
void exec_grep(regex_t *regex, const char *filename, Config *Options, FILE *fp,
               const char *pattern);
int get_flags(char *argv[], char **pattern, Config *Options);
void print_parts(regex_t *regex, Config Options, char *line,
                 const char *pattern);
void regular_print(regex_t *regex, const char *filename, Config Options,
                   char *line, const char *pattern);
void scan_files(char *argv[], char **pattern, Config *Options);
void scan_matches(char *str, int *matches, int i);
void *s21_to_lower(char *str);

#endif
