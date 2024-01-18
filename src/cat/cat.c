#include "cat.h"

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "");
  //  flags are [benstvETA], A is all of them;
  int flags[8] = {};
  int wrong = get_flags(argc, argv, flags);

  if (!wrong) {
    argc -= optind;
    argv += optind;
    int count = 0;

    for (int i = 0; i < 8; i++) count += flags[i];

    scan_files(argc, argv, count > 0, flags);
  } else {
    printf(
        "usage: ./s21_cat [-benstvETA] --number-nonblank --number "
        "--squeeze-blank [file ...]\n");
  }
  return 0;
}

int get_flags(int argc, char *argv[], int flags[]) {
  int ch, option_index, wrong = 0;
  const struct option long_options[] = {
      {"number-nonblank", no_argument, NULL, 0},
      {"number", no_argument, NULL, 1},
      {"squeeze-blank", no_argument, NULL, 2},
      {NULL, 0, NULL, 0}};

  while (!wrong && (ch = getopt_long(argc, argv, "benstvETA", long_options,
                                     &option_index)) != -1) {
    switch (ch) {
      case '?':
        wrong = 1;
        break;
      case 'A':
        // -A implies all
        bflag = eflag = nflag = sflag = tflag = vflag = 1;
        break;
      case 0:
      case 'b':
        // -b implies -n
        bflag = nflag = 1;
        break;
      case 'e':
        // -e implies -v
        eflag = vflag = 1;
        break;
      case 1:
      case 'n':
        nflag = 1;
        break;
      case 2:
      case 's':
        sflag = 1;
        break;
      case 't':
        // -t implies -v
        tflag = vflag = 1;
        break;
      case 'v':
        vflag = 1;
        break;
      case 'E':
        Eflag = 1;
        break;
      case 'T':
        Tflag = 1;
    }
  }
  return wrong;
}

void scan_files(int argc, char *argv[], int flagged, int flags[]) {
  if (argc == 0) {
    if (flagged)
      flagged_cat(stdin, flags);
    else
      raw_cat(stdin);
  }

  for (int i = 0; i < argc; i++) {
    FILE *fp = fopen(argv[i], "r");

    if (fp) {
      if (flagged)
        flagged_cat(fp, flags);
      else
        raw_cat(fp);
      if (fp != stdin) fclose(fp);
    } else {
      warn("%s", argv[i]);
    }
  }
}

void flagged_cat(FILE *fp, const int flags[]) {
  int ch, skip = 0, line = 0, extra = 0;

  for (int prev = '\n'; (ch = fgetc(fp)) != EOF; prev = ch, skip = 0) {
    if (prev == '\n') {
      process_flags(ch, flags, &skip, &line, &extra);
    }
    if (!skip) {
      process_flags_next(ch, flags);
    }
  }
}

void process_flags(char ch, const int flags[], int *skip, int *line,
                   int *extra) {
  if (sflag) {
    if (ch == '\n') {
      if (*extra)
        *skip = 1;
      else
        *extra = 1;
    } else {
      *extra = 0;
    }
  }
  if (!*skip && nflag && (!bflag || ch != '\n')) printf("%6d\t", ++(*line));
}

void process_flags_next(char ch, const int flags[]) {
  int skip = 0;

  if (ch == '\n') {
    if (eflag || Eflag) putchar('$');
  } else if (ch == '\t') {
    if (tflag || Tflag) {
      printf("^I");
      skip = 1;
    }
  } else if (vflag) {
    if (iscntrl(ch)) {
      if (ch == '\177')
        printf("^?");
      else
        printf("^%c", ch + '@');
      skip = 1;
    }

    if (!is_ascii(ch) && !isprint(ch)) {
      printf("M-");
      ch = to_ascii(ch);
    }
  }

  if (!skip) putchar(ch);
}

int is_ascii(int ch) { return ch >= 0 && ch <= 127; }

int to_ascii(int ch) { return ch & 0b1111111; }

void raw_cat(FILE *fp) {
  int ch;
  while ((ch = fgetc(fp)) != EOF) putchar(ch);
}
