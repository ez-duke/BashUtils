#ifndef CAT_H
#define CAT_H

#define _POSIX_C_SOURCE 200809L

#define bflag flags[0]
#define eflag flags[1]
#define nflag flags[2]
#define sflag flags[3]
#define tflag flags[4]
#define vflag flags[5]
#define Eflag flags[6]
#define Tflag flags[7]

#include <ctype.h>
#include <locale.h>

#include "../common/common.h"

void flagged_cat(FILE *fp, const int flags[]);
int get_flags(int argc, char *argv[], int flags[]);
int is_ascii(int ch);
void process_flags(char ch, const int flags[], int *skip, int *line,
                   int *extra);
void process_flags_next(char ch, const int flags[]);
void raw_cat(FILE *fp);
void scan_files(int argc, char *argv[], int flagged, int flags[]);
int to_ascii(int ch);

#endif
