#include "kernel/types.h"
#include "user/user.h"

static void
usage(void)
{
  fprintf(2, "usage: trace <mask> <cmd> [args...]\n");
  exit(1);
}

int
main(int argc, char *argv[])
{
  int mask = 0;

  if (argc < 3 || argv[1][0] == '\0')
    usage();
  for (char *p = argv[1]; *p; p++) {
    if (*p < '0' || *p > '9' || mask > (2147483647 - (*p - '0')) / 10)
      usage();
    mask = mask * 10 + (*p - '0');
  }
  if (trace(mask) < 0) {
    fprintf(2, "trace: trace failed\n");
    exit(1);
  }
  exec(argv[2], &argv[2]);
  fprintf(2, "trace: exec %s failed\n", argv[2]);
  exit(1);
}
