#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

static void
usage(void)
{
  fprintf(2, "usage: xargs [-n N] cmd [args...]\n");
  exit(1);
}

static int
positive(const char *s)
{
  int n = 0;
  if (*s == '\0')
    return -1;
  for (; *s; s++) {
    if (*s < '0' || *s > '9' || n > (2147483647 - (*s - '0')) / 10)
      return -1;
    n = n * 10 + (*s - '0');
  }
  return n > 0 ? n : -1;
}

static int
run(char **args)
{
  int status;
  int pid = fork();
  if (pid < 0) {
    fprintf(2, "xargs: fork failed\n");
    exit(1);
  }
  if (pid == 0) {
    exec(args[0], args);
    fprintf(2, "xargs: exec %s failed\n", args[0]);
    exit(1);
  }
  if (wait(&status) < 0) {
    fprintf(2, "xargs: wait failed\n");
    exit(1);
  }
  return status != 0;
}

int
main(int argc, char *argv[])
{
  char input[1024], *args[MAXARG];
  int command = 1, limit = 0, base, count, len = 0;
  int failed = 0, line_has_word = 0, in_word = 0, word_start = 0;
  char c;

  if (argc > 1 && strcmp(argv[1], "-n") == 0) {
    if (argc < 4 || (limit = positive(argv[2])) < 1)
      usage();
    command = 3;
  }
  if (command >= argc || argc - command >= MAXARG)
    usage();

  base = argc - command;
  for (int i = 0; i < base; i++)
    args[i] = argv[command + i];
  count = base;

  for (;;) {
    int nread = read(0, &c, 1);
    if (nread < 0) {
      fprintf(2, "xargs: read failed\n");
      exit(1);
    }
    int separator = nread == 0 || c == ' ' || c == '\t' || c == '\n';

    if (!separator) {
      if (len >= (int)sizeof(input) - 1) {
        fprintf(2, "xargs: input too long\n");
        exit(1);
      }
      if (!in_word) {
        word_start = len;
        in_word = 1;
      }
      input[len++] = c;
      continue;
    }

    if (in_word) {
      if (count >= MAXARG - 1) {
        fprintf(2, "xargs: too many arguments\n");
        exit(1);
      }
      input[len++] = '\0';
      args[count++] = &input[word_start];
      line_has_word = 1;
      in_word = 0;
    }

    if ((limit && count - base == limit) ||
        (!limit && c == '\n' && line_has_word) ||
        (nread == 0 && count > base)) {
      args[count] = 0;
      failed |= run(args);
      count = base;
      len = 0;
      line_has_word = 0;
      in_word = 0;
    }

    if (nread == 0)
      break;
  }

  exit(failed ? 1 : 0);
}
