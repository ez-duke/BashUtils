#include "grep.h"

int main(int argc, char *argv[]) {
  Config Options = {};
  Options.argc = argc;
  FILE *pattern = NULL;

  int wrong = get_flags(argv, &pattern, &Options);

  if ((Options.cflag || Options.lflag)) Options.nflag = 0;
  Options.argc -= optind;
  if (Options.argc + Options.eflag + Options.fflag < 2) wrong = 1;

  if (wrong == 1) {
    printf("usage: ./s21_grep [-eivclnhsfo] \'template\' [file ...]\n");
  } else {
    argv += optind;
    if (!pattern) {
      if (add_pattern(argv[0], &pattern, Options)) {
        wrong = 1;
      } else {
        Options.argc -= 1;
        argv++;
      }
    }
    if (!wrong) comp_grep(argv, pattern, &Options);
  }

  if (pattern) {
    rewind(pattern);
    fclose(pattern);
    remove("patterns.txt");
  }
  return 0;
}

int get_flags(char *argv[], FILE **pattern, Config *Options) {
  int ch, option_index, wrong = 0;
  const struct option long_options[] = {{NULL, 0, NULL, 0}};

  while (!wrong && (ch = getopt_long(Options->argc, argv, "e:ivclnhsf:o",
                                     long_options, &option_index)) != -1) {
    switch (ch) {
      case '?':
        wrong = 1;
        break;
      case 'e':
        if (add_pattern(optarg, pattern, *Options))
          wrong = 1;
        else
          Options->eflag = 1;
        break;
      case 'i':
        Options->iflag = 1;
        break;
      case 'v':
        Options->vflag = 1;
        break;
      case 'c':
        Options->cflag = 1;
        break;
      case 'l':
        Options->lflag = 1;
        break;
      case 'n':
        Options->nflag = 1;
        break;
      case 'h':
        Options->hflag = 1;
        break;
      case 's':
        Options->sflag = 1;
        break;
      case 'f':
        if (add_pattern_from_file(optarg, pattern, Options))
          wrong = 2;
        else
          Options->fflag = 1;
        break;
      case 'o':
        Options->oflag = 1;
    }
  }
  return wrong;
}

int add_pattern(const char *str, FILE **pattern, Config Options) {
  int error = 0;

  if (*pattern == NULL) {
    *pattern = fopen("patterns.txt", "w+");
    if (!*pattern) error = 1;
  }

  if (error) {
    if (!Options.sflag) warn("%s", "patterns.txt");
  } else {
    fprintf(*pattern, "%s\n", str);
  }
  return error;
}

int add_pattern_from_file(const char *str, FILE **pattern, Config *Options) {
  FILE *fp = fopen(str, "r");
  char *line = NULL;
  size_t len = 0;
  int error = 0;

  if (fp) {
    while (!error && getline(&line, &len, fp) > 0) {
      line[strcspn(line, "\n")] = 0;
      if (strlen(line) == 0) {
        if (add_pattern(".*", pattern, *Options))
          error = 1;
        else
          Options->print_all = 1;
      } else {
        if (add_pattern(line, pattern, *Options)) error = 1;
      }
    }

    fclose(fp);
    free(line);
  } else {
    if (!Options->sflag) warn("%s", str);
    error = 1;
  }
  return error;
}

void comp_grep(char *argv[], FILE *pattern, Config *Options) {
  FILE *fp = NULL;
  int reti = 0;

  for (int i = 0; !reti && i < Options->argc; i++) {
    Options->match_count = 0;
    Options->file_match = 0;
    Options->line_count = 0;

    if ((fp = fopen(argv[i], "r")) != NULL) {
      reti = exec_grep(argv[i], pattern, fp, Options);
      rewind(fp);
      fclose(fp);

      if (!reti) conditional_print(argv[i], *Options);
    }
    if (!fp && !Options->sflag) warn("%s", argv[i]);
  }
}

int exec_grep(const char *filename, FILE *pattern, FILE *fp, Config *Options) {
  regex_t regex;
  size_t len = 0, pat_len = 0;
  char *line = NULL, *pat_line = NULL;
  int reg = 0, count_bup = 0;
  rewind(pattern);
  while (getline(&pat_line, &pat_len, pattern) > 0) count_bup++;

  while (!reg && getline(&line, &len, fp) > 0) {
    int match_found = 0, count = count_bup, offset = 0;
    rewind(pattern);
    line[strcspn(line, "\n")] = 0;
    Options->line_count += 1;

    while (!reg && getline(&pat_line, &pat_len, pattern) > 0) {
      pat_line[strcspn(pat_line, "\n")] = 0;
      reg = regcomp(&regex, pat_line, Options->iflag * REG_ICASE);
      if (reg) {
        printf("Could not compile regex!\n");
      } else {
        process_grep(&regex, filename, line, fp, Options, &count, &match_found,
                     &offset);
      }
    }
  }

  if (!reg) regfree(&regex);
  free(line);
  free(pat_line);
  return reg;
}

void process_grep(regex_t *regex, const char *filename, char *line, FILE *fp,
                  Config *Options, int *count, int *match_found, int *offset) {
  int reti = regexec(regex, line, 0, NULL, 0);
  if (Options->vflag) {
    reti = !reti;
    if (!reti) *count -= 1;
  }

  if ((!reti && !*match_found && !Options->vflag) ||
      (Options->vflag && *count == 0)) {
    Options->match_count += 1;
    Options->file_match = 1;
    *match_found = 1;

    if (!Options->cflag && !Options->lflag)
      regular_print(line, filename, *Options);

    if (Options->lflag) fseek(fp, 0, SEEK_END);
  }
  if (Options->oflag && !Options->lflag && !Options->cflag &&
      !Options->print_all) {
    if (Options->vflag && *count == 0)
      printf("%s\n", line);
    else
      *offset += print_parts(regex, line + *offset, *Options);
  }
}

void regular_print(const char *line, const char *filename, Config Options) {
  if (!Options.hflag && Options.argc > 1) printf("%s:", filename);
  if (Options.nflag) printf("%d:", Options.line_count);
  if (!Options.oflag) printf("%s\n", line);
}

int print_parts(regex_t *regex, char *line, Config Options) {
  regmatch_t whole_match;
  size_t offset = 0, count = 0;
  size_t length = strlen(line);

  while (offset <= length &&
         regexec(regex, line + offset, 1, &whole_match, 0) == 0) {
    if (!Options.vflag)
      printf("%.*s\n", (int)(whole_match.rm_eo - whole_match.rm_so),
             line + whole_match.rm_so + offset);

    offset += whole_match.rm_eo;
    if (whole_match.rm_so == whole_match.rm_eo) offset += 1;
    count++;
  }
  if (offset) offset -= (int)(whole_match.rm_eo - whole_match.rm_so);
  return offset;
}

void conditional_print(const char *filename, Config Options) {
  if (Options.cflag) {
    if (Options.lflag) {
      if (Options.argc > 1 && !Options.hflag)
        printf("%s:%d\n", filename, Options.file_match);
      else
        printf("%d\n", Options.file_match);
    } else {
      if (Options.argc > 1 && !Options.hflag) printf("%s:", filename);
      printf("%d\n", Options.match_count);
    }
  }
  if (Options.lflag && Options.match_count) printf("%s\n", filename);
}
