#include "kernel/types.h"
#include "user/user.h"

static void
usage(void)
{
  fprintf(2, "usage: sleep <ticks>\n");
  exit(1);
}

int
main(int argc, char *argv[])
{
  int ticks = 0;

  if (argc != 2 || argv[1][0] == '\0')
    usage();

  for (char *p = argv[1]; *p; p++) {
    if (*p < '0' || *p > '9')
      usage();
    if (ticks > (2147483647 - (*p - '0')) / 10)
      usage();
    ticks = ticks * 10 + (*p - '0');
  }

  pause(ticks);
  exit(0);
}
