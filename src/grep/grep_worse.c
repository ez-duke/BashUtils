#include "grep_worse.h"

int main(int argc, char *argv[]) {
  Config Options = {};
  Options.argc = argc;
  char *pattern = NULL;

  int wrong = get_flags(argv, &pattern, &Options);

  if ((Options.cflag || Options.lflag)) Options.nflag = 0;
  Options.argc -= optind;
  if (Options.argc + Options.eflag + Options.fflag < 2) wrong = 1;

  if (wrong == 1) {
    printf("usage: ./s21_grep [-eivclnhsfo] \'template\' [file ...]\n");
  } else {
    argv += optind;
    if (!pattern) {
      if (add_pattern(&pattern, argv[0])) {
        wrong = 1;
      } else {
        Options.argc -= 1;
        argv++;
      }
    }
    if (!wrong) comp_grep(argv, &Options, pattern);
  }

  if (pattern) free(pattern);
  return 0;
}

int get_flags(char *argv[], char **pattern, Config *Options) {
  int ch, option_index, wrong = 0;
  const struct option long_options[] = {{NULL, 0, NULL, 0}};

  while (!wrong && (ch = getopt_long(Options->argc, argv, "e:ivclnhsf:o",
                                     long_options, &option_index)) != -1) {
    switch (ch) {
      case '?':
        wrong = 1;
        break;
      case 'e':
        if (add_pattern(pattern, optarg))
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
        if (add_pattern_from_file(pattern, optarg, Options))
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

int add_pattern(char **pattern, const char *str) {
  char *ptr = NULL;
  // 1 for \0 and 1 for |
  int size = strlen(str) + 2, error = 0;

  if (*pattern == NULL) {
    *pattern = calloc(size, 1);
    if (!*pattern) error = 1;
  } else {
    ptr = realloc(*pattern, strlen(*pattern) + size);

    if (ptr) {
      *pattern = ptr;
    } else {
      ptr = *pattern;
      *pattern = NULL;
      free(ptr);
      error = 1;
    }
  }

  if (!error) {
    if (strlen(*pattern)) strcat(*pattern, "|");
    strcat(*pattern, str);
  }
  return error;
}

int add_pattern_from_file(char **pattern, const char *str, Config *Options) {
  FILE *fp = fopen(str, "r");
  char *line = NULL;
  size_t len = 0;
  int error = 0;

  if (fp) {
    while (getline(&line, &len, fp) > 0) {
      line[strcspn(line, "\n")] = 0;
      if (strlen(line) == 0) {
        if (add_pattern(pattern, ".*"))
          error = 1;
        else
          Options->print_all = 1;
      } else {
        if (add_pattern(pattern, line)) error = 1;
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

void comp_grep(char *argv[], Config *Options, char *pattern) {
  FILE *fp = NULL;
  regex_t regex;
  int reti =
      regcomp(&regex, pattern, Options->iflag * REG_ICASE + REG_EXTENDED);

  if (reti) {
    printf("Could not compile regex\n");
  } else {
    for (int i = 0; i < Options->argc; i++) {
      Options->match_count = 0;
      Options->file_match = 0;
      Options->line_count = 0;

      if ((fp = fopen(argv[i], "r")) != NULL) {
        exec_grep(&regex, argv[i], Options, fp, pattern);
        rewind(fp);
        fclose(fp);

        conditional_print(argv[i], *Options);
      }
      if (!fp && !Options->sflag) warn("%s", argv[i]);
    }

    regfree(&regex);
  }
}

void exec_grep(regex_t *regex, const char *filename, Config *Options, FILE *fp,
               const char *pattern) {
  size_t len = 0;
  char *line = NULL;

  while (getline(&line, &len, fp) > 0) {
    line[strcspn(line, "\n")] = 0;
    Options->line_count += 1;

    int reti = regexec(regex, line, 0, NULL, 0);
    if (Options->vflag) reti = !reti;

    if (!reti) {
      Options->match_count += 1;
      Options->file_match = 1;

      if (!Options->cflag && !Options->lflag)
        regular_print(regex, filename, *Options, line, pattern);

      if (Options->lflag) fseek(fp, 0, SEEK_END);
    }
  }

  free(line);
}

void regular_print(regex_t *regex, const char *filename, Config Options,
                   char *line, const char *pattern) {
  if (!Options.hflag && Options.argc > 1) printf("%s:", filename);
  if (Options.nflag) printf("%d:", Options.line_count);
  if (Options.oflag && !Options.print_all && !Options.vflag)
    print_parts(regex, Options, line, pattern);
  else
    printf("%s\n", line);
}

void print_parts(regex_t *regex, Config Options, char *line,
                 const char *pattern) {
  regmatch_t whole_match;
  size_t length = strlen(line);
  int i = 0;
  int *matches = calloc(length, sizeof(int));
  char *match = NULL, *str = calloc(length * 2, 1);

  if (matches && str) {
    size_t offset = 0;
    while (offset <= length &&
           regexec(regex, line + offset, 1, &whole_match, 0) == 0) {
      match = calloc((size_t)whole_match.rm_eo - whole_match.rm_so + 1, 1);

      if (match) {
        sprintf(match, "%.*s", (int)(whole_match.rm_eo - whole_match.rm_so),
                line + whole_match.rm_so + offset);
        if (strlen(str)) strcat(str, "|");
        strcat(str, match);
        add_match(Options, match, pattern, matches, &i, str);
        free(match);
      } else {
        // break
        offset = length + 1;
      }

      offset += whole_match.rm_eo;
      if (whole_match.rm_so == whole_match.rm_eo) offset++;
      i++;
    }
    scan_matches(str, matches, i);

    free(str);
    free(matches);
  } else {
    if (str)
      free(str);
    else
      free(matches);
  }
}

void add_match(Config Options, const char *match, const char *pattern,
               int *matches, int *i, char *buf) {
  char *str = calloc(strlen(pattern) + 1, 1);
  char *ptr = str, *match_copy = calloc(strlen(match) + 1, 1);

  if (match_copy) {
    strcpy(match_copy, match);
    if (Options.iflag) match_copy = s21_to_lower(match_copy);
  }
  if (str) strcpy(str, pattern);

  if (str && match_copy) {
    if (Options.iflag) str = s21_to_lower(str);

    char *ptr_null = str;
    int index = 0, enough = 0;

    while (!enough && (str = strtok(ptr_null, "|"))) {
      index++;
      if (Options.iflag) str = s21_to_lower(str);

      if (!strcmp(str, match_copy)) {
        enough = 1;
        matches[*i] = index;
      }
      ptr_null = NULL;
    }

    while ((str = strtok(NULL, "|"))) {
      if (Options.iflag) str = s21_to_lower(str);
      if (!strcmp(str, match_copy)) {
        *i += 1;
        matches[*i] = index;
        strcat(buf, "|");
        strcat(buf, match);
      }
    }

    free(ptr);
    free(match_copy);
  } else {
    if (ptr)
      free(ptr);
    else
      free(match_copy);
  }
}

void *s21_to_lower(char *str) {
  for (size_t i = 0; str[i]; i++) {
    if (str[i] >= 'A' && str[i] <= 'Z') str[i] += ' ';
  }
  return str;
}

void scan_matches(char *str, int *matches, int i) {
  int high = matches[i - 1];

  for (int j = i - 2; j >= 0; j--) {
    if (matches[j] > high)
      matches[j] = 0;
    else
      high = matches[j];
  }

  char *match = NULL;
  for (int j = 0; j < i; j++) {
    match = strtok(str, "|");
    if (matches[j]) printf("%s\n", match);
    if (str) str = NULL;
  }
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
