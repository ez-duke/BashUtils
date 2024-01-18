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
  char *filename;
  char *line;
  FILE *fp;
  FILE *pattern;
  // a.k.a. files_count
  int argc;
  int flags[10];
  int file_match;
  int line_count;
  int match_count;
} Config;

typedef struct Conf {
  int count;
  int match_found;
  int offset;
} Opts;

int add_pattern(const char *str, Config *Options);
int add_pattern_from_file(const char *str, Config *Options);
void comp_grep(char *argv[], Config *Options);
void conditional_print(Config Options);
int exec_grep(Config *Options);
int get_flags(char *argv[], Config *Options);
int print_parts(regex_t *regex, char *line, Config Options);
void process_grep(regex_t *regex, Config *Options, Opts *Ints);
void regular_print(Config Options);

#endif
